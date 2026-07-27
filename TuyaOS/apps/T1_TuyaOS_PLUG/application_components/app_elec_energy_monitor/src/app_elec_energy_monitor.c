/**
 * @file app_elec_energy_monitor.c
 * @author www.tuya.com
 * @brief app_elec_energy_monitor module is used to 
 * @version 0.1
 * @date 2023-03-24
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_app_config.h"
#include "ty_sys.h"

#if (defined(ENERGY_MONITOR_ENABLE) && (ENERGY_MONITOR_ENABLE==1))

#include "tuya_wifi_status.h"

#include "app_elec_energy_monitor.h"
#include "app_energy_monitor_upload.h"
#include "tfm_timing_storage.h"
#include "tdl_energy_monitor_manage.h"
#include "app_elec_overcharge.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define ENERGY_CAL_RESULT   "measure_rslt"
#define ENERGY_CAL_VALUE    "measure_coe"

// 产测结果
#define ENERGY_CAL_SUCCESS  1
#define ENERGY_CAL_FAIL     2

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    UINT8_T                     enable;
    ENERGY_MONITOR_HANDLE_T     device_hdl;
    TFM_ENERGY_HANDLE           tfm_hdl;
    BOOL_T                      overcharge_enable;
    ENERGY_MONITOR_CAL_DATA_T   cal_data;
} APP_ENERGY_MONITOR_T;


/***********************************************************
********************function declaration********************
***********************************************************/
STATIC OPERATE_RET __app_elec_pvi_data_upload(UINT_T voltage, UINT_T current, UINT_T power);

/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC APP_ENERGY_MONITOR_T sg_energy_monitor = {
    .enable = 1,
    .device_hdl = NULL,
    .overcharge_enable = 0,
    .cal_data = {
        .voltage = ENERGY_MONITOR_CAL_VOLTAGE,
        .current = ENERGY_MONITOR_CAL_CURRENT,
        .power   = ENERGY_MONITOR_CAL_POWER,
        .resval  = ENERGY_MONITOR_SAMPLE_RESISTOR,
    },
};

/***********************************************************
***********************function define**********************
***********************************************************/

STATIC OPERATE_RET __upload_pvi_data(UINT_T voltage, UINT_T current, UINT_T power)
{
    OPERATE_RET rt = OPRT_OK;
    TAL_PR_DEBUG("update tfm vol: %u, cur: %u, power: %u", voltage, current, power);
#if (defined(ENABLE_ENERGY_OVERCHARGE) && (ENABLE_ENERGY_OVERCHARGE==1))
    app_elec_overcharge_detect(power);
#endif
    TUYA_CALL_ERR_RETURN(__app_elec_pvi_data_upload(voltage, current, power));

    return rt;
}

STATIC OPERATE_RET __upload_energy_data(UINT_T electric_quantity, UINT_T timestamp)
{
    OPERATE_RET rt = OPRT_OK;
    TY_OBJ_DP_S obj_data = {0};
    GW_WIFI_NW_STAT_E cur_nw_stat;

    // 判断 MQTT 是否连接，累计电量值统计必须通过 MQTT 上报，不可以使用蓝牙上报接口
    TUYA_CALL_ERR_RETURN(get_wf_gw_nw_status(&cur_nw_stat));
    if (STAT_CLOUD_CONN != cur_nw_stat) {
        TAL_PR_ERR("mqtt not connect, not upload energy");
        return OPRT_COM_ERROR;
    }

    // 等待时间同步
    while (OPRT_OK != tal_time_check_time_sync()) {
        tal_system_sleep(500);
    }

    TAL_PR_DEBUG("upload_energy: %d, %d", electric_quantity, timestamp);

    obj_data.dpid = ENERGY_MONITOR_ADD_ELE_DPID;
    obj_data.type = PROP_VALUE;
    obj_data.value.dp_value = electric_quantity;
    obj_data.time_stamp = timestamp;

    TUYA_CALL_ERR_LOG(dev_report_dp_stat_sync(get_gw_cntl()->gw_if.id, &obj_data, 1, 5));

    return rt;
}

BOOL_T app_elec_energy_monitor_coe_result_get(VOID_T)
{
    UINT8_T cal_result = 0;
    UINT_T ret_len = 0;

    ret_len = tfm_kv_uf_storage_read_data(ENERGY_CAL_RESULT, &cal_result, 1);
    if (0 >= ret_len || ENERGY_CAL_SUCCESS != cal_result) {
        TAL_PR_DEBUG("energy monitor get cal result fail");
        return FALSE;
    }

    return TRUE;
}

VOID_T app_elec_energy_monitor_cal_data_set(ENERGY_MONITOR_CAL_DATA_T cal_data)
{
    memcpy(&sg_energy_monitor.cal_data, &cal_data, SIZEOF(ENERGY_MONITOR_CAL_DATA_T));
    return;
}

VOID_T app_elec_energy_monitor_enable_set(UINT8_T is_enable)
{
    sg_energy_monitor.enable = (is_enable == 0) ? (0) : (1);
    return;
}

UINT8_T app_elec_energy_monitor_enable_get(VOID_T)
{
    return sg_energy_monitor.enable;
}

#if (defined(ENABLE_ZERO_DETECT) && (1 == ENABLE_ZERO_DETECT))
VOID_T app_elec_energy_monitor_set_zero(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    rt = tdl_energy_monitor_set_zero_cross(sg_energy_monitor.device_hdl);
    if(OPRT_OK != rt){
        TAL_PR_DEBUG("set_zero_cross error");
    }
}
#endif

OPERATE_RET app_elec_energy_monitor_init(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    BOOL_T cal_result = FALSE;
    UINT_T ret_len = 0;

    ENERGY_MONITOR_CAL_PARAMS_T cal_params = {0};

    if (sg_energy_monitor.enable == 0) {
        TAL_PR_DEBUG("energy monitor not enable");
        return OPRT_COM_ERROR;
    }

    TUYA_CALL_ERR_RETURN(tdl_energy_monitor_find(ENERGY_MONITOR_NAME, &sg_energy_monitor.device_hdl));

    tdl_energy_monitor_config(sg_energy_monitor.device_hdl, TDL_EM_CMD_CAL_DATA_SET, &sg_energy_monitor.cal_data);

    cal_result = app_elec_energy_monitor_coe_result_get();
    if (TRUE == cal_result) {
        ret_len = tfm_kv_uf_storage_read_data(ENERGY_CAL_VALUE, (UCHAR_T *)&cal_params, SIZEOF(ENERGY_MONITOR_CAL_PARAMS_T));
        if (ret_len == SIZEOF(ENERGY_MONITOR_CAL_PARAMS_T)) {
            tdl_energy_monitor_config(sg_energy_monitor.device_hdl, TDL_EM_CMD_CAL_PARAMS_SET, &cal_params);
        }
    }

    TUYA_CALL_ERR_RETURN(tdl_energy_monitor_open(sg_energy_monitor.device_hdl));
/////////////////////////////0942过零信号配置/////////////////////////
#if (defined(ENABLE_ZERO_DETECT) && (1 == ENABLE_ZERO_DETECT))
    app_elec_energy_monitor_set_zero();
#endif
/////////////////////////////////////////////////////
    TFM_ENERGY_CB_T tfm_energy_cb = {
        .pvi_upload_cb = __upload_pvi_data,
        .add_energy_upload_cb = __upload_energy_data,
    };
    TUYA_CALL_ERR_RETURN(app_energy_monitor_upload_init(sg_energy_monitor.device_hdl, tfm_energy_cb, &sg_energy_monitor.tfm_hdl));

    return rt;
}

OPERATE_RET app_elec_energy_monitor_data_easer(VOID_T)
{
    return app_energy_monitor_data_erase(sg_energy_monitor.tfm_hdl);
}

STATIC OPERATE_RET __app_elec_pvi_data_upload(UINT_T voltage, UINT_T current, UINT_T power)
{
    OPERATE_RET rt = OPRT_OK;

    TY_OBJ_DP_S dp_obj_data[3] = {{0}};

    dp_obj_data[0].dpid = ENERGY_MONITOR_CUR_VOLTAGE_DPID;
    dp_obj_data[0].type = PROP_VALUE;
    dp_obj_data[0].value.dp_value = voltage;
    dp_obj_data[0].time_stamp = 0;
    dp_obj_data[1].dpid = ENERGY_MONITOR_CUR_CURRENT_DPID;
    dp_obj_data[1].type = PROP_VALUE;
    dp_obj_data[1].value.dp_value = current;
    dp_obj_data[1].time_stamp = 0;
    dp_obj_data[2].dpid = ENERGY_MONITOR_CUR_POWER_DPID;
    dp_obj_data[2].type = PROP_VALUE;
    dp_obj_data[2].value.dp_value = power;
    dp_obj_data[2].time_stamp = 0;

    dev_report_dp_json_async(NULL, dp_obj_data, 3);

    return rt;
}

VOID_T app_elec_overcharge_power_get(VOID_T)
{
    ENERGY_MONITOR_VCP_T realtime_data = {0}; 
    app_energy_monitor_pvi_get(sg_energy_monitor.tfm_hdl,&realtime_data);
    app_elec_overcharge_detect(realtime_data.power);
}

OPERATE_RET app_elec_energy_monitor_pvi_upload(VOID_T)
{
    return app_energy_monitor_pvi_sample_upload_now(sg_energy_monitor.tfm_hdl);
}

VOID_T app_elec_energy_monitor_add_energy_upload(VOID_T)
{
    return app_electric_quantity_upload(sg_energy_monitor.tfm_hdl);
}

OPERATE_RET app_elec_energy_monitor_coe_upload(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    UINT_T ret_len = 0;
    UINT8_T upload_num = 0;
    UINT8_T test_result = 0;
    ENERGY_MONITOR_CAL_PARAMS_T energy_coe = {0};

    TY_OBJ_DP_S dp_obj_data[5] = {{0}};

    // read test result
    ret_len = tfm_kv_uf_storage_read_data(ENERGY_CAL_RESULT, &test_result, 1);
    if (0 >= ret_len) {
        TAL_PR_ERR("energy monitor get cal result fail");
        test_result = ENERGY_CAL_FAIL; // 1:success; 2: fail
    }
    dp_obj_data[0].dpid = ENERGY_MONITOR_TEST_RESULT_DPID;
    dp_obj_data[0].type = PROP_VALUE;
    dp_obj_data[0].value.dp_value = test_result;
    dp_obj_data[0].time_stamp = 0;
    upload_num = 1;

    // read coe params
    tfm_kv_uf_storage_read_data(ENERGY_CAL_VALUE, (UCHAR_T *)&energy_coe, SIZEOF(ENERGY_MONITOR_CAL_PARAMS_T));

    dp_obj_data[1].dpid = ENERGY_MONITOR_COE_VOLTAGE_DPID;
    dp_obj_data[1].type = PROP_VALUE;
    dp_obj_data[1].value.dp_value = energy_coe.voltage_period;
    dp_obj_data[1].time_stamp = 0;
    dp_obj_data[2].dpid = ENERGY_MONITOR_COE_CURRENT_DPID;
    dp_obj_data[2].type = PROP_VALUE;
    dp_obj_data[2].value.dp_value = energy_coe.current_period;
    dp_obj_data[2].time_stamp = 0;
    dp_obj_data[3].dpid = ENERGY_MONITOR_COE_POWER_DPID;
    dp_obj_data[3].type = PROP_VALUE;
    dp_obj_data[3].value.dp_value = energy_coe.power_period;
    dp_obj_data[3].time_stamp = 0;
    dp_obj_data[4].dpid = ENERGY_MONITOR_COE_ENERGY_DPID;
    dp_obj_data[4].type = PROP_VALUE;
    dp_obj_data[4].value.dp_value = energy_coe.pf_num;
    dp_obj_data[4].time_stamp = 0;
    upload_num += 4;

    TUYA_CALL_ERR_LOG(dev_report_dp_json_async(NULL, dp_obj_data, upload_num));

    return rt;
}

#endif /* ENERGY_MONITOR_ENABLE */
