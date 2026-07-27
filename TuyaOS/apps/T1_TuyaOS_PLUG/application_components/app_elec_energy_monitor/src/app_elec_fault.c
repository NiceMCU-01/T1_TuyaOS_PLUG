/**
 * @file app_elec_fault.c
 * @author www.tuya.com
 * @brief app_elec_fault module is used to 
 * @version 0.1
 * @date 2023-07-07
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_app_config.h"
#include "ty_sys.h"

#if (defined(ENABLE_ENERGY_FAULT) && (ENABLE_ENERGY_FAULT==1))

#include "app_elec_fault.h"

#include "tdl_energy_monitor_manage.h"

/***********************************************************
************************macro define************************
***********************************************************/
// 故障检测持续时间，超过该时间才会认为发生了故障
#define OVER_VOLTAGE_MS (5*1000U) // 过压故障持续时间
#define LESS_VOLTAGE_MS (5*1000U) // 欠压故障持续时间
#define OVER_CURRENT_MS (5*1000U) // 过流故障持续时间
#define OVER_POWER_MS   (5*1000U) // 过流故障持续时间

typedef UINT8_T ALARM_STATUS_T;
#define STATUS_NORMAL   0
#define STATUS_ALARM    1

typedef UINT8_T COMP_DIR_T;
#define LOWER_LIMIT     0
#define UPPER_LIMIT     1

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    ALARM_STATUS_T alarm_status;
    UINT32_T keep_ms; // 保持时间，只有超过这个时间才会触发故障
    SYS_TIME_T start_time;
    UINT32_T target_value;
    COMP_DIR_T dir;
} __FAULT_T;

typedef struct {
    ENERGY_MONITOR_HANDLE_T device_hdl;
    FAULT_CALLBACK          cb;
    BYTE_T                  dpid;
    TIMER_ID                timer_id;
    APP_ELEC_FAULT_T        fault_value;

    __FAULT_T               over_voltage;
    __FAULT_T               less_voltage;
    __FAULT_T               over_current;
    __FAULT_T               over_power;
} APP_ELE_FAULT_T;

/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC APP_ELE_FAULT_T sg_ele_fault = {
    .device_hdl     = NULL,
    .cb             = NULL,
    .timer_id       = NULL,
    .dpid           = ENERGY_METER_FAULT_DPID,
    .fault_value    = 0,
    .over_voltage = {
        .alarm_status = STATUS_NORMAL,
        .keep_ms = OVER_VOLTAGE_MS,
        .start_time = 0,
        .target_value = ENERGY_METER_FAULT_OVER_VOLTAGE * 10,
        .dir = UPPER_LIMIT,
    },
    .less_voltage = {
        .alarm_status = STATUS_NORMAL,
        .keep_ms = LESS_VOLTAGE_MS,
        .start_time = 0,
        .target_value = ENERGY_METER_FAULT_LESS_VOLTAGE * 10,
        .dir = LOWER_LIMIT,
    },
    .over_current = {
        .alarm_status = STATUS_NORMAL,
        .keep_ms = OVER_CURRENT_MS,
        .start_time = 0,
        .target_value = ENERGY_METER_FAULT_OVER_CURRENT,
        .dir = UPPER_LIMIT,
    },
    .over_power = {
        .alarm_status = STATUS_NORMAL,
        .keep_ms = OVER_POWER_MS,
        .start_time = 0,
        .target_value = ENERGY_METER_FAULT_OVER_POWER * 10,
        .dir = UPPER_LIMIT,
    },
};

/***********************************************************
***********************function define**********************
***********************************************************/

VOID_T __app_fault_check(__FAULT_T *fault_data, UINT32_T cur_value)
{
    SYS_TIME_T cur_time = 0;
    UINT8_T is_fault = 0;

    if (fault_data->dir == LOWER_LIMIT) {
        if (cur_value < fault_data->target_value) {
            is_fault = 1;
        }
    } else {
        if (cur_value > fault_data->target_value) {
            is_fault = 1;
        }
    }

    if (is_fault) {
        if (STATUS_NORMAL == fault_data->alarm_status && 0 == fault_data->start_time) {
            fault_data->start_time = tal_system_get_millisecond();
        } else if (STATUS_NORMAL == fault_data->alarm_status) {
            cur_time = tal_system_get_millisecond();
            if (cur_time > fault_data->start_time + fault_data->keep_ms) {
                fault_data->alarm_status = STATUS_ALARM;
                fault_data->start_time = 0;
            }
        } else {
            fault_data->start_time = 0;
        }
    } else {
        if (STATUS_ALARM == fault_data->alarm_status && 0 == fault_data->start_time) {
            fault_data->start_time = tal_system_get_millisecond();
        } else if (STATUS_ALARM == fault_data->alarm_status) {
            cur_time = tal_system_get_millisecond();
            if (cur_time > fault_data->start_time + fault_data->keep_ms) {
                fault_data->alarm_status = STATUS_NORMAL;
                fault_data->start_time = 0;
            }
        } else {
            fault_data->start_time = 0;
        }
    }

    return;
}

STATIC OPERATE_RET __app_fault_pvi_data_upload(UINT_T voltage, UINT_T current, UINT_T power)
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

    dev_report_dp_stat_sync(get_gw_cntl()->gw_if.id, dp_obj_data, 3, 5);
    return rt;
}

VOID_T __app_fault_timer_cb(TIMER_ID timer_id, VOID_T *arg)
{
    OPERATE_RET rt = OPRT_OK;
    ENERGY_MONITOR_VCP_T vcp_data = {0};
    APP_ELEC_FAULT_T temp_val = 0;
    
    ALARM_STATUS_T last_status = STATUS_NORMAL;
    UINT8_T is_fault_flag = 0;

    if (NULL == sg_ele_fault.cb) {
        return;
    }

    rt = tdl_energy_monitor_read_vcp(sg_ele_fault.device_hdl, &vcp_data);
    if (OPRT_OK != rt) {
        TAL_PR_ERR("energy monitor read vcp fail");
        return;
    }

    if (sg_ele_fault.over_voltage.target_value > 0) {
        // 过压故障使能
        last_status = sg_ele_fault.over_voltage.alarm_status;
        __app_fault_check(&sg_ele_fault.over_voltage, vcp_data.voltage);
        if (last_status != sg_ele_fault.over_voltage.alarm_status) {
            if (STATUS_ALARM == sg_ele_fault.over_voltage.alarm_status) {
                TAL_PR_DEBUG("over_voltage alarm");
                sg_ele_fault.fault_value |= APP_ELEC_FAULT_OVER_VOLTAGE;
                sg_ele_fault.cb(sg_ele_fault.fault_value);
            } else if (STATUS_NORMAL == sg_ele_fault.over_voltage.alarm_status) {
                TAL_PR_DEBUG("over_voltage alarm -> normal");
                temp_val = ~APP_ELEC_FAULT_OVER_VOLTAGE;
                sg_ele_fault.fault_value &= temp_val;
                sg_ele_fault.cb(sg_ele_fault.fault_value);
            }
            is_fault_flag = 1;
        }
    }

    if (sg_ele_fault.less_voltage.target_value > 0) {
        // 欠压故障使能
        last_status = sg_ele_fault.less_voltage.alarm_status;
        __app_fault_check(&sg_ele_fault.less_voltage, vcp_data.voltage);
        if (last_status != sg_ele_fault.less_voltage.alarm_status) {
            if (STATUS_ALARM == sg_ele_fault.less_voltage.alarm_status) {
                TAL_PR_DEBUG("less_voltage alarm");
                sg_ele_fault.fault_value |= APP_ELEC_FAULT_LESS_VOLTAGE;
                sg_ele_fault.cb(sg_ele_fault.fault_value);
            } else if (STATUS_NORMAL == sg_ele_fault.less_voltage.alarm_status) {
                TAL_PR_DEBUG("less_voltage alarm -> normal");
                temp_val = ~APP_ELEC_FAULT_LESS_VOLTAGE;
                sg_ele_fault.fault_value &= temp_val;
                sg_ele_fault.cb(sg_ele_fault.fault_value);
            }
            is_fault_flag = 1;
        }
    }

    if (sg_ele_fault.over_current.target_value > 0) {
        // 过流故障使能
        last_status = sg_ele_fault.over_current.alarm_status;
        __app_fault_check(&sg_ele_fault.over_current, vcp_data.current);
        if (last_status != sg_ele_fault.over_current.alarm_status) {
            if (STATUS_ALARM == sg_ele_fault.over_current.alarm_status) {
                TAL_PR_DEBUG("over_current alarm");
                sg_ele_fault.fault_value |= APP_ELEC_FAULT_OVER_CURRENT;
                sg_ele_fault.cb(sg_ele_fault.fault_value);
            } else if (STATUS_NORMAL == sg_ele_fault.over_current.alarm_status) {
                TAL_PR_DEBUG("over_current alarm -> normal");
                temp_val = ~APP_ELEC_FAULT_OVER_CURRENT;
                sg_ele_fault.fault_value &= temp_val;
                sg_ele_fault.cb(sg_ele_fault.fault_value);
            }
            is_fault_flag = 1;
        }
    }

    if (sg_ele_fault.over_power.target_value > 0) {
        // 过载故障使能
        last_status = sg_ele_fault.over_power.alarm_status;
        __app_fault_check(&sg_ele_fault.over_power, vcp_data.power);
        if (last_status != sg_ele_fault.over_power.alarm_status) {
            if (STATUS_ALARM == sg_ele_fault.over_power.alarm_status) {
                TAL_PR_DEBUG("over_power alarm");
                sg_ele_fault.fault_value |= APP_ELEC_FAULT_OVER_POWER;
                sg_ele_fault.cb(sg_ele_fault.fault_value);
            } else if (STATUS_NORMAL == sg_ele_fault.over_power.alarm_status) {
                TAL_PR_DEBUG("over_power alarm -> normal");
                temp_val = ~APP_ELEC_FAULT_OVER_POWER;
                sg_ele_fault.fault_value &= temp_val;
                sg_ele_fault.cb(sg_ele_fault.fault_value);
            }
            is_fault_flag = 1;
        }
    }

    if(is_fault_flag){
        __app_fault_pvi_data_upload(vcp_data.voltage,vcp_data.current,vcp_data.power);
    }

    return;
}

VOID_T app_elec_fault_params_set(UINT32_T over_voltage, UINT32_T less_voltage, UINT32_T over_current,UINT32_T over_power)
{
    sg_ele_fault.over_voltage.target_value = over_voltage;
    sg_ele_fault.less_voltage.target_value = less_voltage;
    sg_ele_fault.over_current.target_value = over_current;
    sg_ele_fault.over_power.target_value = over_power;
    return;
}

OPERATE_RET app_elec_fault_init(FAULT_CALLBACK cb)
{
    OPERATE_RET rt = OPRT_OK;

    TUYA_CHECK_NULL_RETURN(cb, OPRT_INVALID_PARM);

    sg_ele_fault.cb = cb;

    TUYA_CALL_ERR_RETURN(tdl_energy_monitor_find(ENERGY_MONITOR_NAME, &sg_ele_fault.device_hdl));

    if (NULL == sg_ele_fault.timer_id) {
        TUYA_CALL_ERR_RETURN(tal_sw_timer_create(__app_fault_timer_cb, NULL, &sg_ele_fault.timer_id));
    }

    TUYA_CALL_ERR_RETURN(tal_sw_timer_start(sg_ele_fault.timer_id, 500, TAL_TIMER_CYCLE));

    TAL_PR_DEBUG("fault: over_vol:%d, less_vol: %d, over_curr: %d", sg_ele_fault.over_voltage.target_value, sg_ele_fault.less_voltage.target_value, sg_ele_fault.over_current.target_value);
    TAL_PR_DEBUG("fault init success");

    return rt;
}

APP_ELEC_FAULT_T app_elec_fault_value_get(VOID_T)
{
    return sg_ele_fault.fault_value;
}

OPERATE_RET app_elec_fault_upload(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

    TY_OBJ_DP_S dp_obj_data  ={0};

    dp_obj_data.dpid = sg_ele_fault.dpid;
    dp_obj_data.type = PROP_BITMAP;
    dp_obj_data.value.dp_bitmap = sg_ele_fault.fault_value;
    dp_obj_data.time_stamp = 0;

    TUYA_CALL_ERR_LOG(dev_report_dp_json_async(NULL, &dp_obj_data, 1));

    return rt;
}

#endif /* ENABLE_ENERGY_FAULT */
