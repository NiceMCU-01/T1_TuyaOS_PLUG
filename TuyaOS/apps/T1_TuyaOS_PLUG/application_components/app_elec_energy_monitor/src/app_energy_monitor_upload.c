/**
 * @file app_energy_monitor_upload.c
 * @author www.tuya.com
 * @brief app_energy_monitor_upload module is used to 
 * @version 0.1
 * @date 2023-09-15
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "app_energy_monitor_upload.h"
#include "tfm_timing_storage.h"

#include "tal_sw_timer.h"
#include "tal_memory.h"
#include "tal_mutex.h"
#include "tal_system.h"

#include "tal_log.h"

#include "gw_intf.h"

/***********************************************************
************************macro define************************
***********************************************************/
/*电量存储*/
#define TEMP_ENERGY_KEY     "temp_energy" // 不带时标的临时电量
#define DAY_ENERGY_KEY      "day_energy"  // 每天的电量存储

#define TFM_FAST_SAMPLE_TIME_MS     (5*1000U)
#define TFM_SAMPLE_TIME_MS          (30*1000U)

// 离线保存天数
#define TFM_OFFLINE_DAY_MAX         (10)

/* pvi 上报控制宏 */
#define UPLOAD_THRESHOLD_VOLTAGE    (2)  // 2%
#define UPLOAD_THRESHOLD_POWER      (20) // 20%

// PVI 强制上报时间
#define FORCED_REPORT_TIME_MS        (1*60*60*1000U) // 1h

/* 累计电量上报控制宏 */
#define LARGE_ELECTRIC_QUANTITY     (100) // w*H
#define LARGE_ELEC_REPORT_TIME_MS   (10*60*1000U) // 10 min
#define SMALL_ELECTRIC_QUANTITY     (1) // w*H
#define SMALL_ELEC_REPORT_TIME_MS   (30*60*1000U) // 30 min

#define GET_ERROR_PERCENT(last_value, current_value) \
    (last_value > current_value) ? ((100*(last_value-current_value))/last_value) : ((100*(current_value-last_value))/last_value)

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    UINT_T timestamp;
    UINT_T electric_quantity;
} TFM_ENERGY_UPLOAD_DATA_T;

typedef struct {
    UINT_T num;
    TFM_ENERGY_UPLOAD_DATA_T data[TFM_OFFLINE_DAY_MAX];
} LOCAL_ELECTRIC_QUANTITY_T;

typedef struct {
    ENERGY_MONITOR_HANDLE_T tdl_hdl;
    TFM_ENERGY_CB_T upload_cb;
    TIMER_ID timer_id;
    TIME_MS sample_ms;
    MUTEX_HANDLE mutex_hdl;

    // last upload data
    UINT32_T pvi_interval_ms; // 距离上一次上报pvi数据过去了多久
    UINT32_T last_vol;
    UINT32_T last_cur;
    UINT32_T last_power;

    // 累计电能
    UINT32_T energy_interval_ms; // 距离上一次上报电量数据过去了多久
    // 临时不带标电量值
    UINT32_T cur_add_energy;

    // 带标累计电量值
    LOCAL_ELECTRIC_QUANTITY_T day_energy;
} TFM_ENERGY_INFO_T;

/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/


/***********************************************************
***********************function define**********************
***********************************************************/
STATIC VOID_T __tfm_local_data_print(VOID_T)
{
    TFM_ENERGY_INFO_T tmp_uf_data = {0};
    UINT32_T i = 0;
    tfm_kv_uf_storage_read_data(TEMP_ENERGY_KEY, (UCHAR_T *)&tmp_uf_data.cur_add_energy, SIZEOF(UINT32_T));
    tfm_kv_uf_storage_read_data(DAY_ENERGY_KEY, (UCHAR_T *)&tmp_uf_data.day_energy, SIZEOF(LOCAL_ELECTRIC_QUANTITY_T));
    TAL_PR_DEBUG("---> temp add temp: %d", tmp_uf_data.cur_add_energy);
    TAL_PR_DEBUG("---> day number: %d", tmp_uf_data.day_energy.num);
    for (i=0; i<tmp_uf_data.day_energy.num; i++) {
        TAL_PR_DEBUG("---> day energy %d: %d, %d", i, tmp_uf_data.day_energy.data[i].electric_quantity, tmp_uf_data.day_energy.data[i].timestamp);
    }

    return;
}

STATIC OPERATE_RET __tfm_electric_quantity_local_write(TFM_ENERGY_INFO_T *tfm_hdl)
{
    OPERATE_RET rt = OPRT_OK;
    POSIX_TM_S cur_time = {0}, temp_time = {0};
    TIME_T timestamp = 0;
    UINT8_T data_index = 0;

    // 判断本地时间是否经过同步
    rt = tal_time_check_time_sync();
    if (OPRT_OK != rt) {
        TAL_PR_DEBUG("time not sync, save temp energy");
        TAL_PR_DEBUG("energy temp: %d", tfm_hdl->cur_add_energy);
        TUYA_CALL_ERR_RETURN(tfm_kv_uf_storage_write_data(TEMP_ENERGY_KEY, (UCHAR_T *)&tfm_hdl->cur_add_energy, sizeof(UINT32_T)));
        return OPRT_OK;
    }

    // 查找数据存储的索引值
    TUYA_CALL_ERR_RETURN(tal_time_get_local_time_custom(0, &cur_time));
    if (tfm_hdl->day_energy.num == 0) {
        data_index = 0;
        tfm_hdl->day_energy.num++;
    } else {
        data_index = tfm_hdl->day_energy.num-1;
        if (tfm_hdl->day_energy.num < TFM_OFFLINE_DAY_MAX) {
            timestamp = tfm_hdl->day_energy.data[data_index].timestamp;
            TUYA_CALL_ERR_RETURN(tal_time_get_local_time_custom(timestamp, &temp_time));
            if (temp_time.tm_year != cur_time.tm_year || \
                temp_time.tm_mon != cur_time.tm_mon || \
                temp_time.tm_mday != cur_time.tm_mday) {
                    data_index = tfm_hdl->day_energy.num;
                    tfm_hdl->day_energy.num++;
            }
        }
    }

    tfm_hdl->day_energy.data[data_index].timestamp = tal_time_get_posix();
    tfm_hdl->day_energy.data[data_index].electric_quantity += tfm_hdl->cur_add_energy;
    TUYA_CALL_ERR_RETURN(tfm_kv_uf_storage_write_data(DAY_ENERGY_KEY, (UCHAR_T *)&tfm_hdl->day_energy, sizeof(LOCAL_ELECTRIC_QUANTITY_T)));
    tfm_hdl->cur_add_energy = 0;

    TAL_PR_DEBUG("save num: %d, save index: %d, timestamp: %d, electric: %d", \
            tfm_hdl->day_energy.num, data_index, \
            tfm_hdl->day_energy.data[data_index].timestamp, \
            tfm_hdl->day_energy.data[data_index].electric_quantity);

#if 1
    __tfm_local_data_print();
#endif

    return rt;
}

VOID_T app_electric_quantity_upload(TFM_ENERGY_HANDLE handle)
{
    OPERATE_RET rt = OPRT_OK;
    GW_WORK_STAT_T gw_statue = UNREGISTERED;
    TFM_ENERGY_INFO_T *tfm_hdl = NULL;
    INT_T i = 0;
    UINT32_T stored_add_energy = 0;

    if (NULL == handle) {
        TAL_PR_DEBUG("tfm energy handle is null");
        return;
    }

    tfm_hdl = (TFM_ENERGY_INFO_T *)handle;

    // 未激活，不记录任何数据
    gw_statue = get_gw_active();
    if(gw_statue == UNREGISTERED) {
        TAL_PR_ERR("device not register");
        return;
    }

    if (NULL == tfm_hdl->upload_cb.add_energy_upload_cb) {
        TAL_PR_ERR("add_energy_upload_cb not register");
        return;
    }

    // 增加数据为 0，不上报
    if (0 == tfm_hdl->cur_add_energy && 0 == tfm_hdl->day_energy.num) {
        TAL_PR_DEBUG("cur add, day energy is 0, not upload");
        return;
    }

#if 1
    __tfm_local_data_print();
#endif

    rt = tfm_hdl->upload_cb.add_energy_upload_cb(tfm_hdl->cur_add_energy, 0);
    if (rt == OPRT_OK) {
        TAL_PR_DEBUG("temp energy upload success");
        // 擦除本地临时数据
        TAL_PR_DEBUG("temp energy clear");
        tfm_hdl->cur_add_energy = 0;
          // 读取 Flash 中当前存储的 cur_add_energy 值
        rt = tfm_kv_uf_storage_read_data(TEMP_ENERGY_KEY, (UCHAR_T *)&stored_add_energy, sizeof(UINT32_T));
        if (OPRT_OK == rt && stored_add_energy != 0) {
            TAL_PR_DEBUG("Clearing non-zero temp energy in flash: %u", stored_add_energy);
            TUYA_CALL_ERR_LOG(tfm_kv_uf_storage_write_data(TEMP_ENERGY_KEY, (UCHAR_T *)&tfm_hdl->cur_add_energy, sizeof(UINT32_T)));
        } 
        // 检查本地是否存储了电量
        TAL_PR_DEBUG("tfm_hdl->day_energy.num: %d", tfm_hdl->day_energy.num);
        if (tfm_hdl->day_energy.num>0) {
            for (i=tfm_hdl->day_energy.num-1; i>=0; i--) {
                TAL_PR_DEBUG("day energy: %d, %d, %d", i, tfm_hdl->day_energy.data[i].electric_quantity, tfm_hdl->day_energy.data[i].timestamp);
                rt = tfm_hdl->upload_cb.add_energy_upload_cb(tfm_hdl->day_energy.data[i].electric_quantity, tfm_hdl->day_energy.data[i].timestamp);
                if (OPRT_OK == rt) {
                    tfm_hdl->day_energy.data[i].electric_quantity = 0;
                    tfm_hdl->day_energy.data[i].timestamp = 0;
                    tfm_hdl->day_energy.num--;
                } else {
                    break;
                }
            }
            TUYA_CALL_ERR_LOG(tfm_kv_uf_storage_write_data(DAY_ENERGY_KEY, (UCHAR_T *)&tfm_hdl->day_energy, sizeof(LOCAL_ELECTRIC_QUANTITY_T)));
        }
    } else {
        TAL_PR_DEBUG("add_energy_upload fail");
        TUYA_CALL_ERR_LOG(__tfm_electric_quantity_local_write(tfm_hdl));
    }

    return;
}

STATIC VOID_T __energy_monitor_cycle_sample_cb(TIMER_ID timer_id, VOID_T *arg)
{
    OPERATE_RET rt = OPRT_OK;
    TFM_ENERGY_INFO_T *tfm_hdl = NULL;
    UINT8_T pvi_is_upload = 0, energy_is_upload = 0;
    UINT32_T voltage_percent = 0, power_percent = 0;
    ENERGY_MONITOR_VCP_T realtime_data = {0};
    UINT32_T tmp_e;
    POSIX_TM_S cur_time = {0};

    if (arg == NULL) {
        return;
    }
    tfm_hdl = (TFM_ENERGY_INFO_T *)arg;

    tal_mutex_lock(tfm_hdl->mutex_hdl);

    tfm_hdl->pvi_interval_ms += tfm_hdl->sample_ms;
    tfm_hdl->energy_interval_ms += tfm_hdl->sample_ms;

    rt = tdl_energy_monitor_read_vcp(tfm_hdl->tdl_hdl, &realtime_data);
    if (OPRT_OK != rt) {
        goto __EXIT;
    }

    // 判断 pvi 是否满足上报条件
    if (tfm_hdl->last_vol > 0) {
        voltage_percent = GET_ERROR_PERCENT(tfm_hdl->last_vol, realtime_data.voltage);
        TAL_PR_DEBUG("last: %d, %d vol percent: %d", tfm_hdl->last_vol, realtime_data.voltage, voltage_percent);
    }
    if (tfm_hdl->last_power > 0) {
        power_percent = GET_ERROR_PERCENT(tfm_hdl->last_power, realtime_data.power);
        TAL_PR_DEBUG("last: %d, %d power percent: %d", tfm_hdl->last_power, realtime_data.power, power_percent);
    }
    TAL_PR_DEBUG("last: %d, %d, current", tfm_hdl->last_cur, realtime_data.current);

    if ((tfm_hdl->last_vol == 0 && realtime_data.voltage >0) || \
        (tfm_hdl->last_power == 0 && realtime_data.power >0)) {
        pvi_is_upload = 1;
    } else if (voltage_percent > UPLOAD_THRESHOLD_VOLTAGE || power_percent > UPLOAD_THRESHOLD_POWER) {
        pvi_is_upload = 1;
    } else if (tfm_hdl->pvi_interval_ms >= FORCED_REPORT_TIME_MS) {
        pvi_is_upload = 1;
    } else {
        pvi_is_upload = 0;
        if (tfm_hdl->sample_ms == TFM_FAST_SAMPLE_TIME_MS) {
            tfm_hdl->sample_ms = TFM_SAMPLE_TIME_MS;
            tal_sw_timer_start(tfm_hdl->timer_id, tfm_hdl->sample_ms, TAL_TIMER_CYCLE);
        }
    }

    if (1 == pvi_is_upload && NULL != tfm_hdl->upload_cb.pvi_upload_cb) {
        pvi_is_upload = 0;
        tfm_hdl->pvi_interval_ms = 0;
        TAL_PR_DEBUG("pvi upload");
        rt = tfm_hdl->upload_cb.pvi_upload_cb(realtime_data.voltage, realtime_data.current, realtime_data.power);
        if (OPRT_OK == rt) {
            tfm_hdl->last_vol   = realtime_data.voltage;
            tfm_hdl->last_cur   = realtime_data.current;
            tfm_hdl->last_power = realtime_data.power;
        }

        // 切换为快速采样
        tfm_hdl->sample_ms = TFM_FAST_SAMPLE_TIME_MS;
        tal_sw_timer_start(tfm_hdl->timer_id, tfm_hdl->sample_ms, TAL_TIMER_CYCLE);
    }

    rt = tdl_energy_monitor_read_energy(tfm_hdl->tdl_hdl, &tmp_e);
    if (OPRT_OK != rt) {
        goto __EXIT;
    }

    // 判断累计电量是否满足上报条件
    tfm_hdl->cur_add_energy += tmp_e;
    TAL_PR_DEBUG("cur_add_energy: %d, time: %dms", tfm_hdl->cur_add_energy, tfm_hdl->energy_interval_ms);

    if (tfm_hdl->cur_add_energy >= LARGE_ELECTRIC_QUANTITY && tfm_hdl->energy_interval_ms>=LARGE_ELEC_REPORT_TIME_MS) {
        energy_is_upload = 1;
    } else if (tfm_hdl->cur_add_energy >= SMALL_ELECTRIC_QUANTITY && tfm_hdl->energy_interval_ms>=SMALL_ELEC_REPORT_TIME_MS) {
        energy_is_upload = 1;
    } else {
        if (tfm_hdl->cur_add_energy > 0) {
            // 每天 23.59 如果有电量，上报一下
            rt = tal_time_get_local_time_custom(0, &cur_time);
            if (OPRT_OK == rt && 23 == cur_time.tm_hour && 59 == cur_time.tm_min) {
                energy_is_upload = 1;
            }
        } else {
            energy_is_upload = 0;
        }
    }

    if (1 == energy_is_upload) {
        energy_is_upload = 0;
        tfm_hdl->energy_interval_ms = 0;
        app_electric_quantity_upload(tfm_hdl);
    }

__EXIT:
    tal_mutex_unlock(tfm_hdl->mutex_hdl);

    return;
}

OPERATE_RET app_energy_monitor_upload_init(ENERGY_MONITOR_HANDLE_T tdl_hdl, TFM_ENERGY_CB_T energy_cb, TFM_ENERGY_HANDLE *handle)
{
    OPERATE_RET rt = OPRT_OK;
    TFM_ENERGY_INFO_T *energy_hdl = NULL;

    TUYA_CHECK_NULL_RETURN(tdl_hdl, OPRT_INVALID_PARM);
    TUYA_CHECK_NULL_RETURN(energy_cb.pvi_upload_cb, OPRT_INVALID_PARM);
    TUYA_CHECK_NULL_RETURN(energy_cb.add_energy_upload_cb, OPRT_INVALID_PARM);

#if 1
    __tfm_local_data_print();
#endif

    energy_hdl = (TFM_ENERGY_INFO_T *)tal_malloc(SIZEOF(TFM_ENERGY_INFO_T));
    TUYA_CHECK_NULL_RETURN(energy_hdl, OPRT_MALLOC_FAILED);
    memset(energy_hdl, 0, SIZEOF(TFM_ENERGY_INFO_T));
    *handle = energy_hdl;

    energy_hdl->tdl_hdl = tdl_hdl;
    energy_hdl->sample_ms = TFM_FAST_SAMPLE_TIME_MS;
    memcpy(&energy_hdl->upload_cb, &energy_cb, SIZEOF(TFM_ENERGY_CB_T));

    // read flash
    tfm_kv_uf_storage_read_data(TEMP_ENERGY_KEY, (UCHAR_T *)&energy_hdl->cur_add_energy, SIZEOF(UINT32_T));
    tfm_kv_uf_storage_read_data(DAY_ENERGY_KEY, (UCHAR_T *)&energy_hdl->day_energy, SIZEOF(LOCAL_ELECTRIC_QUANTITY_T));

    TUYA_CALL_ERR_RETURN(tal_mutex_create_init(&energy_hdl->mutex_hdl));

    TUYA_CALL_ERR_RETURN(tal_sw_timer_create(__energy_monitor_cycle_sample_cb, energy_hdl, &energy_hdl->timer_id));
    TUYA_CALL_ERR_RETURN(tal_sw_timer_start(energy_hdl->timer_id, energy_hdl->sample_ms, TAL_TIMER_CYCLE));

    return rt;
}

OPERATE_RET app_energy_monitor_pvi_sample_upload_now(TFM_ENERGY_HANDLE handle)
{
    OPERATE_RET rt = OPRT_OK;
    TFM_ENERGY_INFO_T *tfm_hdl = NULL;
    ENERGY_MONITOR_VCP_T realtime_data = {0};

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);
    tfm_hdl = (TFM_ENERGY_INFO_T *)handle;

    tal_mutex_lock(tfm_hdl->mutex_hdl);

    TUYA_CALL_ERR_GOTO(tdl_energy_monitor_read_vcp(tfm_hdl->tdl_hdl, &realtime_data), __EXIT);
    if (NULL != tfm_hdl->upload_cb.pvi_upload_cb) {
        rt = tfm_hdl->upload_cb.pvi_upload_cb(realtime_data.voltage, realtime_data.current, realtime_data.power);
        if (OPRT_OK == rt) {
            tfm_hdl->last_vol = realtime_data.voltage;
            tfm_hdl->last_cur = realtime_data.current;
            tfm_hdl->last_power = realtime_data.power;
        }
        tfm_hdl->pvi_interval_ms = 0;
    }

__EXIT:
    tal_mutex_unlock(tfm_hdl->mutex_hdl);

    return rt;
}

OPERATE_RET app_energy_monitor_data_erase(TFM_ENERGY_HANDLE handle)
{
    OPERATE_RET rt = OPRT_OK;
    TFM_ENERGY_INFO_T *tfm_hdl = NULL;

    TUYA_CALL_ERR_LOG(tfm_kv_uf_storage_erase_data(TEMP_ENERGY_KEY));
    TUYA_CALL_ERR_LOG(tfm_kv_uf_storage_erase_data(DAY_ENERGY_KEY));

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);
    tfm_hdl = (TFM_ENERGY_INFO_T *)handle;

    tfm_hdl->cur_add_energy = 0;
    memset(&tfm_hdl->day_energy, 0, SIZEOF(LOCAL_ELECTRIC_QUANTITY_T));

    return rt;
}


OPERATE_RET app_energy_monitor_pvi_get(TFM_ENERGY_HANDLE handle,ENERGY_MONITOR_VCP_T *realtime_data)
{
    OPERATE_RET rt = OPRT_OK;
    TFM_ENERGY_INFO_T *tfm_hdl = NULL;

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);
    tfm_hdl = (TFM_ENERGY_INFO_T *)handle;

    tal_mutex_lock(tfm_hdl->mutex_hdl);

    TUYA_CALL_ERR_GOTO(tdl_energy_monitor_read_vcp(tfm_hdl->tdl_hdl, realtime_data), __EXIT);

__EXIT:
    tal_mutex_unlock(tfm_hdl->mutex_hdl);

    return rt;
}
