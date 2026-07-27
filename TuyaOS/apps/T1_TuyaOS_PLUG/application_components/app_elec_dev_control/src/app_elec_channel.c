/**
 * @file app_elec_channel.c
 * @author www.tuya.com
 * @brief app_elec_channel module is used to 
 * @version 0.1
 * @date 2023-03-22
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_app_config.h"

#include "tuya_cloud_types.h"
#include "tuya_ws_db.h"
#include "ty_sys.h"

#include "tal_log.h"
#include "tal_memory.h"

#include "app_elec_channel.h"
#include "tfm_timing_storage.h"
#include "app_elec_led.h"

#include "tdl_relay_manage.h"
#include "tdl_led_manage.h"

#include "tuya_uf_db.h"
#if defined(ENABLE_NVS_STORAGE) && (ENABLE_NVS_STORAGE)
#include "tbs_nvs.h"
#endif
/***********************************************************
************************macro define************************
***********************************************************/
// uf 存储名称
#define POWER_ON_MODE_NAME      "RLY_INIT"
#define POWER_ON_STATUS_NAME    "RLY_STAT"

#define AUTO_SAVE_TIME_MS       (5*1000U)

/* 通道结构体初始化配置宏 */
#define RELAY_NAME(seq)         ELEC_CHANNEL_##seq##_RELAY_NAME
#define CHANNEL_LED_NAME(seq)   ELEC_CHANNEL_##seq##_LED_NAME

#define ELEC_CHANNEL_RELAY_DEF_INIT(seq) \
    .relay_name = RELAY_NAME(seq), \
    .relay_hdl = NULL

#define ELEC_CHANNEL_LED_DEF_INIT(seq) \
    .led_name = CHANNEL_LED_NAME(seq), \
    .led_hdl = NULL

#if (defined(ENABLE_TY_LOAD_OEM_PAR) && (ENABLE_TY_LOAD_OEM_PAR == 1))
#define ELEC_CHANNEL_NUM_USED   ELEC_CHANNEL_NUM
#else
#if defined(T1_PLUG_WAY_NUM)
#define ELEC_CHANNEL_NUM_USED   T1_PLUG_WAY_NUM
#else
#define ELEC_CHANNEL_NUM_USED   DEFAULT_ELEC_CHANNEL_NUM
#endif
#endif

#define RUN_TIME_SWITCH_KEY      "RUNT_SWITCH" // 运行时长开关本地存储key
#define __DEBUG     1
/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    RELAY_HANDLE_T relay_hdl;
    CHAR_T *relay_name;
    PVOID_T led_hdl;
    CHAR_T *led_name;
    BYTE_T dpid;
}APP_ELEC_SINGLE_CHANNEL_T;

typedef struct {
    UINT8_T num;
    UINT_T status;
    APP_CHANNEL_MODE_E mode; // 上电时通道模式
    UINT8_T led_num; // 存储使用了多少 LED，非计量产测会使用
    APP_ELEC_SINGLE_CHANNEL_T *channel;
}APP_ELEC_CHANNEL_T;

#if defined (ELEC_RUNTIME_SWITCH_EN) && (ELEC_RUNTIME_SWITCH_EN == 1)
typedef struct {
    UINT_T timestamp;
    UINT8_T value; // 0/1
} RUN_TIME_SWITCH_UPLOAD_DATA_T;

typedef struct {
    UINT_T num;
    RUN_TIME_SWITCH_UPLOAD_DATA_T data[ELEC_RUNTIME_SWITCH_NUM_MAX];
} LOCAL_RUN_TIME_SWITCH_T;
#endif
/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC APP_ELEC_SINGLE_CHANNEL_T sg_elec_single_chan[ELEC_CHANNEL_NUM_MAX] = {
#if (defined(ENABLE_ELEC_CHANNEL_1) && (ENABLE_ELEC_CHANNEL_1 == 1))
{
    ELEC_CHANNEL_RELAY_DEF_INIT(1),
    #if (defined(ENABLE_ELEC_CHANNEL_1_LED) && (ENABLE_ELEC_CHANNEL_1_LED))
    ELEC_CHANNEL_LED_DEF_INIT(1),
    #else
    .led_hdl = NULL, .led_name = NULL,
    #endif
    .dpid = ELEC_CHANNEL_1_SWITCH_DPID,
},
#endif
#if (defined(ENABLE_ELEC_CHANNEL_2) && (ENABLE_ELEC_CHANNEL_2 == 1))
{
    ELEC_CHANNEL_RELAY_DEF_INIT(2),
    #if (defined(ENABLE_ELEC_CHANNEL_2_LED) && (ENABLE_ELEC_CHANNEL_2_LED))
    ELEC_CHANNEL_LED_DEF_INIT(2),
    #else
    .led_hdl = NULL, .led_name = NULL,
    #endif
    .dpid = ELEC_CHANNEL_2_SWITCH_DPID,
},
#endif
#if (defined(ENABLE_ELEC_CHANNEL_3) && (ENABLE_ELEC_CHANNEL_3 == 1))
{
    ELEC_CHANNEL_RELAY_DEF_INIT(3),
    #if (defined(ENABLE_ELEC_CHANNEL_3_LED) && (ENABLE_ELEC_CHANNEL_3_LED))
    ELEC_CHANNEL_LED_DEF_INIT(3),
    #else
    .led_hdl = NULL, .led_name = NULL,
    #endif
    .dpid = ELEC_CHANNEL_3_SWITCH_DPID,
},
#endif
#if (defined(ENABLE_ELEC_CHANNEL_4) && (ENABLE_ELEC_CHANNEL_4 == 1))
{
    ELEC_CHANNEL_RELAY_DEF_INIT(4),
    #if (defined(ENABLE_ELEC_CHANNEL_4_LED) && (ENABLE_ELEC_CHANNEL_4_LED))
    ELEC_CHANNEL_LED_DEF_INIT(4),
    #else
    .led_hdl = NULL, .led_name = NULL,
    #endif
    .dpid = ELEC_CHANNEL_4_SWITCH_DPID,
},
#endif
#if (defined(ENABLE_ELEC_CHANNEL_5) && (ENABLE_ELEC_CHANNEL_5 == 1))
{
    ELEC_CHANNEL_RELAY_DEF_INIT(5),
    #if (defined(ENABLE_ELEC_CHANNEL_5_LED) && (ENABLE_ELEC_CHANNEL_5_LED))
    ELEC_CHANNEL_LED_DEF_INIT(5),
    #else
    .led_hdl = NULL, .led_name = NULL,
    #endif
    .dpid = ELEC_CHANNEL_5_SWITCH_DPID,
},
#endif
#if (defined(ENABLE_ELEC_CHANNEL_6) && (ENABLE_ELEC_CHANNEL_6 == 1))
{
    ELEC_CHANNEL_RELAY_DEF_INIT(6),
    #if (defined(ENABLE_ELEC_CHANNEL_6_LED) && (ENABLE_ELEC_CHANNEL_6_LED))
    ELEC_CHANNEL_LED_DEF_INIT(6),
    #else
    .led_hdl = NULL, .led_name = NULL,
    #endif
    .dpid = ELEC_CHANNEL_6_SWITCH_DPID,
},
#endif
#if (defined(ENABLE_ELEC_CHANNEL_7) && (ENABLE_ELEC_CHANNEL_7 == 1))
{
    ELEC_CHANNEL_RELAY_DEF_INIT(7),
    #if (defined(ENABLE_ELEC_CHANNEL_7_LED) && (ENABLE_ELEC_CHANNEL_7_LED))
    ELEC_CHANNEL_LED_DEF_INIT(7),
    #else
    .led_hdl = NULL, .led_name = NULL,
    #endif
    .dpid = ELEC_CHANNEL_7_SWITCH_DPID,
},
#endif
#if (defined(ENABLE_ELEC_CHANNEL_8) && (ENABLE_ELEC_CHANNEL_8 == 1))
{
    ELEC_CHANNEL_RELAY_DEF_INIT(8),
    #if (defined(ENABLE_ELEC_CHANNEL_8_LED) && (ENABLE_ELEC_CHANNEL_8_LED))
    ELEC_CHANNEL_LED_DEF_INIT(8),
    #else
    .led_hdl = NULL, .led_name = NULL,
    #endif
    .dpid = ELEC_CHANNEL_8_SWITCH_DPID,
},
#endif
};

STATIC APP_ELEC_CHANNEL_T sg_elec_chan = {
    .num = ELEC_CHANNEL_NUM_USED,
    .status = 0,
    .led_num = 0,
    .mode = ELEC_CHANNEL_POWER_ON_MODE,
    .channel = sg_elec_single_chan,
};

/***********************************************************
***********************function define**********************
***********************************************************/
#if defined (ENABLE_ELEC_COMPATIBLE_DATA_FORMAT) && (ENABLE_ELEC_COMPATIBLE_DATA_FORMAT == 1)
#define CHANNEL_MEM "ch_mem" // 存储断电记忆的key
#define OBJECT_KEY  "ch_state"

#define STORE_CHANGE "init_stat_save" //上电状态
#define DEF_MODE_VAR "def_mode"

/**
 * @brief 兼容老的上电模式key
 *
 * @param[in] : mode 上电模式
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_channel_mode_data_compatible(APP_CHANNEL_MODE_E *mode)
{
    OPERATE_RET rt = OPRT_OK;
    ty_cJSON *root = NULL;
    UCHAR_T *buff = NULL;
    UINT_T buff_len = 0;
    BOOL_T exist = FALSE;
    rt = wd_common_exist(STORE_CHANGE, &exist);
    if (OPRT_OK != rt || exist == FALSE) {
        return OPRT_OK;
    }

    rt = iot_wd_common_read(STORE_CHANGE, &buff, &buff_len);
    if (OPRT_OK != rt) {
        TAL_PR_NOTICE("init cfg is null!!");
        return rt;
    }

    TAL_PR_NOTICE("read def_state: %s", buff);
    root = ty_cJSON_Parse((CHAR_T *)buff);
    Free(buff);
    buff = NULL;
    if (NULL == root) {
        TAL_PR_ERR("ty_cJSON parse err");
        return OPRT_CJSON_PARSE_ERR;
    }

    ty_cJSON *js_json = ty_cJSON_GetObjectItem(root, DEF_MODE_VAR);
    if (NULL == js_json) {
        ty_cJSON_Delete(root);
        root = NULL;
        return OPRT_CJSON_GET_ERR;
    }

    *mode = js_json->valueint;
    rt = tfm_kv_uf_storage_write_data(POWER_ON_MODE_NAME, mode, SIZEOF(ELEC_LIGHT_MODE_E));
    if (OPRT_OK == rt) {
        TUYA_CALL_ERR_LOG(wd_common_delete(STORE_CHANGE));
    }

    ty_cJSON_Delete(root);
    root = NULL;
    return rt;
}

/**
 * @brief 兼容老的上电状态key
 *
 * @param[in] : channel_num 通道数量
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_channel_data_compatible(UINT8_T channel_num)
{
    OPERATE_RET rt = OPRT_OK;
    CHAR_T *buff = NULL;
    UINT_T buff_len = 0;
    BOOL_T exist = FALSE;
    UINT8_T write_buf[ELEC_CHANNEL_NUM_MAX] = {0};

    rt = wd_common_exist(CHANNEL_MEM, &exist);
    if (OPRT_OK != rt || exist == FALSE) {
        return OPRT_OK;
    }

    rt = wd_common_read(CHANNEL_MEM, (BYTE_T **)&buff, &buff_len);
    if (OPRT_OK != rt) {
        TAL_PR_ERR("[%s] wd_common_read err:%d", __FUNCTION__, rt);
        return OPRT_COM_ERROR;
    }

    if (NULL == buff) {
        TAL_PR_ERR("read buff is null");
        return OPRT_COM_ERROR;
    }

    TAL_PR_DEBUG("read channel buff: %s", buff);
    ty_cJSON *root = ty_cJSON_Parse(buff);
    if (NULL == root) {
        return OPRT_CJSON_PARSE_ERR;
    }

    ty_cJSON *object = ty_cJSON_GetObjectItem(root, OBJECT_KEY);
    if (NULL == object) {
        cJSON_Delete(root);
        return OPRT_CJSON_PARSE_ERR;
    }

    UINT8_T i = 0;
    memset(write_buf, 0, ELEC_CHANNEL_NUM_MAX*sizeof(UINT8_T));
    ty_cJSON *array = NULL;
    for (i = 0; i < channel_num; i++) {
        array = ty_cJSON_GetArrayItem(object, i);
        if (NULL == array) {
            continue;
        }
        if (cJSON_True == array->type) {
            TAL_PR_NOTICE("read channel[%d] stat: open", i);
            write_buf[i] = TRUE;
        } else {
            TAL_PR_NOTICE("read channel[%d] stat: close", i);
            write_buf[i] = FALSE;
        }
    }

    rt = tfm_kv_uf_storage_write_data(POWER_ON_STATUS_NAME, write_buf, ELEC_CHANNEL_NUM_MAX);
    if (OPRT_OK == rt) {
        TUYA_CALL_ERR_LOG(wd_common_delete(CHANNEL_MEM));
    }

    cJSON_Delete(root);

    return OPRT_OK;
    return rt;
}

#endif

#if defined (ELEC_RUNTIME_SWITCH_EN) && (ELEC_RUNTIME_SWITCH_EN == 1)
STATIC VOID_T app_run_time_local_data_print(VOID_T)
{
    LOCAL_RUN_TIME_SWITCH_T local_data = {0};
    UINT32_T i = 0;

    tbs_nvs_read(RUN_TIME_SWITCH_KEY, (UCHAR_T *)&local_data, sizeof(LOCAL_RUN_TIME_SWITCH_T));
    TAL_PR_DEBUG("---> runtime_switch number: %d", local_data.num);
    for (i=0; i<local_data.num; i++) {
        TAL_PR_DEBUG("---> runtime_switch %d: %d, %d", i, local_data.data[i].value, local_data.data[i].timestamp);
    }
    return;
}

OPERATE_RET app_run_time_switch_data_erase(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    
    TAL_PR_DEBUG("erase runtime switch data");
    
    rt = tbs_nvs_erase(RUN_TIME_SWITCH_KEY);
    if (OPRT_OK != rt) {
        TAL_PR_ERR("delete runtime switch nvs data failed: %d", rt);
    }    
    return rt; 
}

STATIC OPERATE_RET app_run_time_switch_upload_sync(UINT8_T value, UINT_T timestamp)
{
    OPERATE_RET rt = OPRT_OK;
    TY_OBJ_DP_S obj_data = {0};
    GW_WIFI_NW_STAT_E cur_nw_stat;

    // 判断 MQTT 是否连接，runtime_switch统计必须通过 MQTT 上报，不可以使用蓝牙上报接口
    TUYA_CALL_ERR_RETURN(get_wf_gw_nw_status(&cur_nw_stat));
    if (STAT_CLOUD_CONN != cur_nw_stat) {
        TAL_PR_ERR("mqtt not connect, not upload runtime_switch");
        return OPRT_COM_ERROR;
    }

    // 等待时间同步
    while (OPRT_OK != tal_time_check_time_sync()) {
        tal_system_sleep(500);
    }

    TAL_PR_DEBUG("upload_runtime_switch: %d, %d", value, timestamp);

    obj_data.dpid = ELEC_RUNTIME_SWITCH_1_DPID;
    obj_data.type = PROP_BOOL;
    obj_data.value.dp_bool = value;
    obj_data.time_stamp = timestamp;

    TUYA_CALL_ERR_LOG(dev_report_dp_stat_sync(get_gw_cntl()->gw_if.id, &obj_data, 1, 5));

    return rt;
}
VOID_T app_run_time_switch_local_upload(VOID_T)
{
    LOCAL_RUN_TIME_SWITCH_T local_data = {0};
    OPERATE_RET rt = OPRT_OK;
    UINT_T i = 0, success_count = 0;
    INT_T nvs_rt = 0;

    nvs_rt = tbs_nvs_read(RUN_TIME_SWITCH_KEY, (UCHAR_T *)&local_data, sizeof(LOCAL_RUN_TIME_SWITCH_T));
    if (nvs_rt > 0 && local_data.num > 0) {
        for (i = 0; i < local_data.num; i++) {
            rt = app_run_time_switch_upload_sync(local_data.data[i].value, local_data.data[i].timestamp);
            if (OPRT_OK == rt) {
                success_count++;
                tal_system_sleep(100);
            } else {
                break;
            }
        }
        if (success_count > 0) {
            if (success_count < local_data.num) {
                memmove(&local_data.data[0], &local_data.data[success_count], sizeof(RUN_TIME_SWITCH_UPLOAD_DATA_T) * (local_data.num - success_count));
            }
            local_data.num -= success_count;
            tbs_nvs_write(RUN_TIME_SWITCH_KEY, (UCHAR_T *)&local_data, sizeof(LOCAL_RUN_TIME_SWITCH_T));
        }
    }
}

STATIC OPERATE_RET app_run_time_switch_local_write(UINT8_T value)
{
    LOCAL_RUN_TIME_SWITCH_T local_data = {0};
    UINT_T index = 0;
    OPERATE_RET rt = OPRT_OK;

    // 判断本地时间是否经过同步
    rt = tal_time_check_time_sync();
    if (OPRT_OK != rt) {
        TAL_PR_DEBUG("time not sync, no save !!!");
        return OPRT_OK;
    }

    // 读取本地缓存
    tbs_nvs_read(RUN_TIME_SWITCH_KEY, (UCHAR_T *)&local_data, sizeof(LOCAL_RUN_TIME_SWITCH_T));
    if (local_data.num >= 0 ){
        if (local_data.num >= ELEC_RUNTIME_SWITCH_NUM_MAX) {
            memmove(&local_data.data[0], &local_data.data[1], sizeof(RUN_TIME_SWITCH_UPLOAD_DATA_T) * (ELEC_RUNTIME_SWITCH_NUM_MAX - 1));
            local_data.num = ELEC_RUNTIME_SWITCH_NUM_MAX - 1;
        }
        
        index = local_data.num;
        local_data.data[index].timestamp = tal_time_get_posix();
        local_data.data[index].value = value;
        local_data.num++;

        tbs_nvs_write(RUN_TIME_SWITCH_KEY, (UCHAR_T *)&local_data, sizeof(LOCAL_RUN_TIME_SWITCH_T));
    }
#if __DEBUG
    app_run_time_local_data_print();
#endif
    return OPRT_OK;
}
VOID_T app_run_time_switch_upload(UINT8_T chan_idx)
{
    OPERATE_RET rt = OPRT_OK;
    GW_WORK_STAT_T gw_statue = UNREGISTERED;
    UINT8_T cur_status = 0;

    if (chan_idx > sg_elec_chan.num || chan_idx < 0) {
        return OPRT_INVALID_PARM;
    }

    // 未激活，不记录任何数据
    gw_statue = get_gw_active();
    if(gw_statue == UNREGISTERED) {
        TAL_PR_ERR("device not register");
        return;
    }
#if __DEBUG
    app_run_time_local_data_print();
#endif
    //目前需求仅为1路
    if((1 == sg_elec_chan.num) &&(1 >= chan_idx)){

        cur_status = (sg_elec_chan.status & 0x00000001) ? (TRUE) : (FALSE);
        rt = app_run_time_switch_upload_sync(cur_status, 0);
        if (OPRT_OK == rt) {
            TAL_PR_DEBUG("run time switch upload success");
            app_run_time_switch_local_upload();
            
        }else{
            // 上报失败，保存本地
            app_run_time_switch_local_write(cur_status);
        }
    }
}
#endif
STATIC VOID_T __app_channel_status_auto_save_cb(CHAR_T *p_key, VOID_T *arg)
{
    OPERATE_RET rt = OPRT_OK;
    UINT8_T i = 0;
    UINT8_T channel_status[ELEC_CHANNEL_NUM_MAX] = {0};

    if (0 == strcmp(POWER_ON_STATUS_NAME, p_key)) {
        for (i=0; i<sg_elec_chan.num; i++) {
            if (sg_elec_chan.status & (0x00000001<<i)) {
                channel_status[i] = 1;
            }
        }
        TAL_PR_HEXDUMP_DEBUG("channel status auto save", channel_status, ELEC_CHANNEL_NUM_MAX);
        TUYA_CALL_ERR_LOG(tfm_kv_uf_storage_write_data(POWER_ON_STATUS_NAME, channel_status, ELEC_CHANNEL_NUM_MAX));
    }

    return;
}

STATIC VOID_T __app_elec_channel_status_init(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    UINT_T read_len = 0;
    UINT8_T read_value[20] = {0};
    UINT8_T i=0;

#if defined (ENABLE_ELEC_COMPATIBLE_DATA_FORMAT) && (ENABLE_ELEC_COMPATIBLE_DATA_FORMAT == 1)
    app_elec_channel_mode_data_compatible(&sg_elec_chan.mode);
    app_elec_channel_data_compatible(sg_elec_chan.num);
#endif

    read_len = tfm_kv_uf_storage_read_data(POWER_ON_MODE_NAME, read_value, 20);
    if (read_len > 0 && read_value[0] < MODE_MAX) {
        sg_elec_chan.mode = read_value[0];
    }

    TAL_PR_DEBUG("power-up mode: %d", sg_elec_chan.mode);

    if (MODE_MEMORY == sg_elec_chan.mode) {
        read_len = tfm_kv_uf_storage_read_data(POWER_ON_STATUS_NAME, read_value, 20);
        if (read_len > 0) {
            // 将 uf 中存储的数据格式转换成本地的数据格式
            for (i=0; i<sg_elec_chan.num; i++) {
                sg_elec_chan.status = (read_value[i] == 1) ? (sg_elec_chan.status | (0x00000001<<i)) : (sg_elec_chan.status & (~(0x00000001<<i)));
            }
        }
    } else if (MODE_TURN_ON == sg_elec_chan.mode) {
        sg_elec_chan.status = ~(0xFFFFFFFF<<sg_elec_chan.num);
    } else {
        sg_elec_chan.status = 0;
    }

    TAL_PR_DEBUG("channel status :0x%x", sg_elec_chan.status);

    TUYA_CALL_ERR_LOG(tfm_timing_storage_init(AUTO_SAVE_TIME_MS));
    TUYA_CALL_ERR_LOG(tfm_timing_storage_register(POWER_ON_STATUS_NAME, __app_channel_status_auto_save_cb, NULL));

    return;
}

OPERATE_RET app_elec_channel_init(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    UINT_T chan_idx = 0;
    UINT8_T cur_chan_state = 0;
    TDL_LED_CONFIG_T led_cfg = {0};
    RELAY_STATUS_E relay_status = RELAY_STATUS_OFF;

    // 读取上电状态
    __app_elec_channel_status_init();

    // 初始化各个通道
    for (chan_idx=0; chan_idx<sg_elec_chan.num; chan_idx++) {
        cur_chan_state = sg_elec_chan.status & (0x00000001 << chan_idx);
        relay_status = (cur_chan_state != 0) ? (RELAY_STATUS_ON) : (RELAY_STATUS_OFF);
        led_cfg.stat = TDL_LED_OFF;

        TUYA_CALL_ERR_RETURN(tdl_relay_dev_find(sg_elec_chan.channel[chan_idx].relay_name, &sg_elec_chan.channel[chan_idx].relay_hdl));
        TUYA_CALL_ERR_RETURN(tdl_relay_dev_open(sg_elec_chan.channel[chan_idx].relay_hdl));
        TUYA_CALL_ERR_RETURN(tdl_relay_dev_write(sg_elec_chan.channel[chan_idx].relay_hdl, relay_status));

        if (NULL != sg_elec_chan.channel[chan_idx].led_name) {
            tdl_led_dev_find(sg_elec_chan.channel[chan_idx].led_name, &sg_elec_chan.channel[chan_idx].led_hdl);
            if (NULL != sg_elec_chan.channel[chan_idx].led_hdl) {
                TUYA_CALL_ERR_LOG(tdl_led_ctrl(sg_elec_chan.channel[chan_idx].led_hdl, &led_cfg));
                // 计算通道 LED 数量
                sg_elec_chan.led_num++;
            }
        }
    }

    return OPRT_OK;
}

STATIC OPERATE_RET __app_elec_single_channel_set(UINT8_T chan_idx, APP_CHANNEL_STATE_E status)
{
    OPERATE_RET rt = OPRT_OK;

    RELAY_STATUS_E relay_status = RELAY_STATUS_OFF;

    TAL_PR_DEBUG("chan_id: %d, status: %d", chan_idx+1, status);

    if (NULL == sg_elec_chan.channel[chan_idx].relay_hdl || chan_idx >= sg_elec_chan.num) {
        return OPRT_COM_ERROR;
    }

    if (STATE_TOGGLE == status) {
        status = (sg_elec_chan.status & (0x00000001 << chan_idx)) ? (STATE_OFF) : (STATE_ON);
    }

    if (STATE_ON == status) {
        relay_status = RELAY_STATUS_ON;
    } else {
        relay_status = RELAY_STATUS_OFF;
    }

    TUYA_CALL_ERR_RETURN(tdl_relay_dev_write(sg_elec_chan.channel[chan_idx].relay_hdl, relay_status));

    // 更新对应通道状态
    sg_elec_chan.status = (status == STATE_ON) ? (sg_elec_chan.status | (0x00000001<<chan_idx)) : (sg_elec_chan.status & (~(0x00000001<<chan_idx)));

    return rt;
}

OPERATE_RET app_elec_channel_status_set(APP_ELEC_CHANNEL_CFG_T *cfg)
{
    OPERATE_RET rt = OPRT_OK;

    UINT32_T i=0;
    UINT_T chan_idx = 0;

    TUYA_CHECK_NULL_RETURN(cfg, OPRT_INVALID_PARM);

    if (cfg->chan_id > sg_elec_chan.num) {
        return OPRT_INVALID_PARM;
    }

    if (0 == cfg->chan_id) { // 设置所有通道状态
        APP_CHANNEL_STATE_E status = STATE_OFF;
        if (STATE_TOGGLE == cfg->status) {
            status = (sg_elec_chan.status) ? (STATE_OFF) : (STATE_ON);
        } else {
            status = cfg->status;
        }

        for (i=0; i<sg_elec_chan.num; i++) {
            TUYA_CALL_ERR_RETURN(__app_elec_single_channel_set(i, status));
        }
    } else {
        chan_idx = cfg->chan_id - 1;
        TUYA_CALL_ERR_RETURN(__app_elec_single_channel_set(chan_idx, cfg->status));
    }

    // 断电记忆模式，自动保存通道状态
    if (sg_elec_chan.mode == MODE_MEMORY) {
        TUYA_CALL_ERR_RETURN(tfm_storage_start_timer(POWER_ON_STATUS_NAME));
    }

    // 更新通道 LED 状态
    app_elec_channel_led_refresh();

    return rt;
}

OPERATE_RET app_elec_chan_led_handle_set(UINT_T chan_id, PVOID_T led_hdl)
{
    OPERATE_RET rt = OPRT_OK;

    if (chan_id > sg_elec_chan.num) {
        TAL_PR_ERR("chan id > chan num, %d", chan_id);
        return OPRT_COM_ERROR;
    }

    sg_elec_chan.channel[chan_id-1].led_hdl = led_hdl;

    if (led_hdl != NULL) {
        // 更新对应继电器状态
    }

    return rt;
}

OPERATE_RET app_elec_channel_led_refresh(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    UINT8_T i = 0;
    UINT8_T cur_chan_status = 0;
    TDL_LED_CONFIG_T led_status = {0};

    ELEC_LIGHT_MODE_E led_mode = app_elec_power_led_mode_get();

    for (i=0; i<sg_elec_chan.num; i++) {
        cur_chan_status = (sg_elec_chan.status & (0x00000001 << i)) ? (STATE_ON) : (STATE_OFF);

        switch (led_mode) {
            case LIGHT_MODE_RELAY:
                led_status.stat = (cur_chan_status == STATE_ON) ? (TDL_LED_ON) : (TDL_LED_OFF);
            break;
            case LIGHT_MODE_POSITION:
                led_status.stat = (cur_chan_status == STATE_ON) ? (TDL_LED_OFF) : (TDL_LED_ON);
            break;
            case LIGHT_MODE_ALWAYS_OFF:
                led_status.stat = TDL_LED_OFF;
            break;
            case LIGHT_MODE_ALWAYS_ON:
                led_status.stat = TDL_LED_ON;
            break;
            default: break;
        }
        if (NULL != sg_elec_chan.channel[i].led_hdl) {
            TUYA_CALL_ERR_RETURN(tdl_led_ctrl(sg_elec_chan.channel[i].led_hdl, &led_status));
        }
    }

    return rt;
}

VOID_T app_elec_channel_mode_default_set(APP_CHANNEL_MODE_E mode)
{
    TAL_PR_DEBUG("channel mode set: %d", mode);
    sg_elec_chan.mode = mode;

    return;
}

OPERATE_RET app_elec_channel_mode_set(APP_CHANNEL_MODE_E mode)
{
    OPERATE_RET rt = OPRT_OK;
    UINT8_T write_buf[ELEC_CHANNEL_NUM_MAX] = {0};
    UINT8_T i = 0;

    if (mode >= MODE_MAX) {
        return OPRT_INVALID_PARM;
    }

    TAL_PR_DEBUG("mode : %d", mode);

    sg_elec_chan.mode = mode;
    TUYA_CALL_ERR_RETURN(tfm_kv_uf_storage_write_data(POWER_ON_MODE_NAME, &sg_elec_chan.mode, 1));

    memset(write_buf, 0, ELEC_CHANNEL_NUM_MAX*sizeof(UINT8_T));
    if (MODE_MEMORY == sg_elec_chan.mode) {
        for (i=0; i<sg_elec_chan.num; i++) {
            if (sg_elec_chan.status & (0x00000001<<i)) {
                write_buf[i] = 1;
            }
        }
        TUYA_CALL_ERR_RETURN(tfm_kv_uf_storage_write_data(POWER_ON_STATUS_NAME, write_buf, ELEC_CHANNEL_NUM_MAX));
    }

    return rt;
}

VOID_T app_elec_channel_data_erase(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

    TAL_PR_DEBUG("erase channel data");

    TUYA_CALL_ERR_LOG(tfm_storage_stop_timer());
    TUYA_CALL_ERR_LOG(tfm_kv_uf_storage_erase_data(POWER_ON_STATUS_NAME));
    TUYA_CALL_ERR_LOG(tfm_kv_uf_storage_erase_data(POWER_ON_MODE_NAME));

    return;
}

OPERATE_RET app_elec_get_all_channel_status(UINT_T *status)
{
    if (NULL == status) {
        return OPRT_INVALID_PARM;
    }

    *status = sg_elec_chan.status;

    return OPRT_OK;
}

APP_CHANNEL_STATE_E app_elec_channel_status_get(UINT_T chan_id)
{
    if (chan_id > sg_elec_chan.num || chan_id == 0) {
        TAL_PR_ERR("channel %d not open");
        return STATE_OFF;
    }

    return ((sg_elec_chan.status&(0x00000001<<(chan_id-1))) != 0) ? (STATE_ON) : (STATE_OFF);
}

OPERATE_RET app_elec_channel_config(ELEC_CHANNEL_CMD_E cmd, VOID_T *arg)
{
    OPERATE_RET rt = OPRT_OK;

    switch (cmd) {
        case ELEC_CHANNEL_NUM_SET:
            TUYA_CHECK_NULL_RETURN(arg, OPRT_INVALID_PARM);
            UINT8_T channel_num = *((UINT8_T *)arg);
            if (channel_num > ELEC_CHANNEL_NUM_MAX) {
                return OPRT_COM_ERROR;
            }
            sg_elec_chan.num = channel_num;
        break;
        case ELEC_CHANNEL_NUM_GET:
            TUYA_CHECK_NULL_RETURN(arg, OPRT_INVALID_PARM);
            *((UINT8_T *)arg) = sg_elec_chan.num;
        break;
        default: return OPRT_NOT_SUPPORTED;
    }

    return rt;
}

OPERATE_RET app_elec_channel_status_upload(UINT_T chan_id)
{
    OPERATE_RET rt = OPRT_OK;

    UINT_T chan_idx = chan_id-1;
    TY_OBJ_DP_S dp_obj_data = {0};

    UINT8_T cur_status = 0;

    if (chan_idx >= sg_elec_chan.num || chan_idx < 0) {
        return OPRT_INVALID_PARM;
    }

    cur_status = (sg_elec_chan.status & (0x00000001 << chan_idx)) ? (TRUE) : (FALSE);

    dp_obj_data.dpid = sg_elec_chan.channel[chan_idx].dpid;
    dp_obj_data.type = PROP_BOOL;
    dp_obj_data.value.dp_bool = cur_status;
    dp_obj_data.time_stamp = 0;

    dev_report_dp_json_async(NULL, &dp_obj_data, 1);
    return rt;
}

OPERATE_RET app_elec_channel_status_upload_by_dpid(BYTE_T dpid)
{
    OPERATE_RET rt = OPRT_OK;
    UINT_T target_index = 0;
    TY_OBJ_DP_S dp_obj_data = {0};
    UINT8_T cur_status = 0;

    for (target_index=0; target_index<sg_elec_chan.num; target_index++) {
        if (dpid == sg_elec_chan.channel[target_index].dpid) {
            break;
        }
    }

    if (target_index >= sg_elec_chan.num) {
        // 没有发现对应 DPID，退出
        TAL_PR_DEBUG("channel not find dpid %d", dpid);
        return OPRT_COM_ERROR;
    }

    cur_status = (sg_elec_chan.status & (0x00000001 << target_index)) ? (TRUE) : (FALSE);

    dp_obj_data.dpid = sg_elec_chan.channel[target_index].dpid;
    dp_obj_data.type = PROP_BOOL;
    dp_obj_data.value.dp_bool = cur_status;
    dp_obj_data.time_stamp = 0;

    dev_report_dp_json_async(NULL, &dp_obj_data, 1);

    return rt;
}

OPERATE_RET app_elec_all_channel_status_upload(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

    UINT_T i =0;
    UINT8_T single_status = 0;
    TY_OBJ_DP_S *dp_obj_data = NULL;

    dp_obj_data = tal_malloc(sg_elec_chan.num * SIZEOF(TY_OBJ_DP_S));
    TUYA_CHECK_NULL_RETURN(dp_obj_data, OPRT_MALLOC_FAILED);

    for (i=0; i<sg_elec_chan.num; i++) {
        single_status = (sg_elec_chan.status & (0x00000001 << i)) ? (TRUE) : (FALSE);

        dp_obj_data[i].dpid = sg_elec_chan.channel[i].dpid;
        dp_obj_data[i].type = PROP_BOOL;
        dp_obj_data[i].value.dp_bool = single_status;
        dp_obj_data[i].time_stamp = 0;
    }

    dev_report_dp_json_async(NULL, dp_obj_data, sg_elec_chan.num);

    if (NULL != dp_obj_data) {
        tal_free(dp_obj_data);
        dp_obj_data = NULL;
    }

    return rt;
}

OPERATE_RET app_elec_channel_mode_upload(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

    TY_OBJ_DP_S dp_obj_data = {0};

    dp_obj_data.dpid = ELEC_CHANNEL_POWER_ON_DPID;
    dp_obj_data.type = PROP_ENUM;
    dp_obj_data.value.dp_enum = sg_elec_chan.mode;
    dp_obj_data.time_stamp = 0;

    TUYA_CALL_ERR_LOG(dev_report_dp_json_async(NULL, &dp_obj_data, 1));

    return rt;
}

BOOL_T app_elec_channel_data_is_save(VOID_T)
{
    if (!ufexist(POWER_ON_MODE_NAME)) {
        return 0;
    }
    if (!ufexist(POWER_ON_STATUS_NAME)) {
        return 0;
    }

    return 1;
}

VOID_T app_elec_channel_dpid_set(UINT_T chan_id, BYTE_T dpid)
{
    if (chan_id > ELEC_CHANNEL_NUM_MAX || 0 >= chan_id) {
        TAL_PR_ERR("input params fail");
        return;
    }

    sg_elec_single_chan[chan_id - 1].dpid = dpid;

    return;
}

UINT8_T app_elec_channel_led_num_get(VOID_T)
{
    return sg_elec_chan.led_num;
}
