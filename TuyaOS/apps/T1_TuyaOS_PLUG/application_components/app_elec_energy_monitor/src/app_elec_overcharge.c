/**
 * @file app_elec_overcharge.c
 * @author www.tuya.com
 * @brief app_elec_overcharge module is used to 
 * @version 0.1
 * @date 2023-09-11
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_app_config.h"
#include "ty_sys.h"

#if (defined(ENABLE_ENERGY_OVERCHARGE) && (ENABLE_ENERGY_OVERCHARGE==1))

#include "app_elec_overcharge.h"
#include "tfm_timing_storage.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define OVERCHARGE_KEY          "over_charge"

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    BOOL_T enable;

    BOOL_T is_start;
    BOOL_T chan_state;

    TIMER_ID timer_id;
    MUTEX_HANDLE mutex_hdl;

    UINT32_T power_threshold;
    UINT32_T overcharge_s;
    OVERCHARGE_CB cb;

    UINT32_T last_power_value;
} APP_ELE_OVERCHARGE_T;

/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC APP_ELE_OVERCHARGE_T sg_overcharge = {
    .enable = FALSE,
    .is_start = TRUE,
    .chan_state = FALSE,
    .timer_id = NULL,
    .power_threshold = OVERCHARGE_POWER_THRESHOLD * 10,
    .overcharge_s = OVERCHARGE_DURATION_MINS*60,
    .cb = NULL,
    .last_power_value = 0,
};

/***********************************************************
***********************function define**********************
***********************************************************/
STATIC VOID_T __app_overcharge_timer_cb(TIMER_ID timer_id, VOID_T *arg)
{
    APP_ELE_OVERCHARGE_T *p_overcharge = NULL;

    if (NULL == arg) {
        TAL_PR_ERR("p_overcharge is NULL");
        return;
    }

    p_overcharge = (APP_ELE_OVERCHARGE_T *)arg;

    if (NULL == p_overcharge->cb || 0 == p_overcharge->enable || 0 == p_overcharge->is_start) {
        TAL_PR_ERR("overcharge cb is null or not enable or not start");
        return;
    }

    TAL_PR_DEBUG("overcharge cb");
    p_overcharge->is_start = FALSE;
    p_overcharge->cb();

    return;
}

OPERATE_RET app_elec_overcharge_init(OVERCHARGE_CB cb)
{
    OPERATE_RET rt;

    TUYA_CHECK_NULL_RETURN(cb, OPRT_INVALID_PARM);

    sg_overcharge.cb = cb;

    TUYA_CALL_ERR_LOG(tal_sw_timer_create(__app_overcharge_timer_cb, &sg_overcharge, &sg_overcharge.timer_id));

    TUYA_CALL_ERR_LOG(tal_mutex_create_init(&sg_overcharge.mutex_hdl));

    tfm_kv_uf_storage_read_data(OVERCHARGE_KEY, (UCHAR_T *)&sg_overcharge.enable, SIZEOF(BOOL_T));

    return rt;
}

OPERATE_RET app_elec_overcharge_start(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

    app_elec_overcharge_detect(sg_overcharge.last_power_value);

    return rt;
}

OPERATE_RET app_elec_overcharge_stop(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

    tal_mutex_lock(sg_overcharge.mutex_hdl);

    sg_overcharge.is_start = FALSE;

    if(TRUE == tal_sw_timer_is_running(sg_overcharge.timer_id)) {
        TUYA_CALL_ERR_LOG(tal_sw_timer_stop(sg_overcharge.timer_id));
        TAL_PR_DEBUG("stop overcharge timer");
    }

    tal_mutex_unlock(sg_overcharge.mutex_hdl);

    return rt;
}


OPERATE_RET app_elec_overcharge_channel_sync(UINT_T chan_status)
{
    OPERATE_RET rt = OPRT_OK;

    tal_mutex_lock(sg_overcharge.mutex_hdl);

    if(!chan_status){
        if (TRUE == tal_sw_timer_is_running(sg_overcharge.timer_id)) {
            TAL_PR_DEBUG("chan_status Close,stop overcharge time!");
            tal_sw_timer_stop(sg_overcharge.timer_id);
        }
        sg_overcharge.is_start = FALSE;
    }
    sg_overcharge.chan_state = chan_status;
    //TAL_PR_DEBUG("chan_status =%d!!!!!!!!!!!!!!!!",chan_status);

    tal_mutex_unlock(sg_overcharge.mutex_hdl);

    return rt;
}

OPERATE_RET app_elec_overcharge_detect(UINT32_T power_value)
{
    OPERATE_RET rt = OPRT_OK;

    if (NULL == sg_overcharge.cb || NULL == sg_overcharge.timer_id) {
        TAL_PR_DEBUG("overcharge not init");
        return OPRT_OK;
    }

    tal_mutex_lock(sg_overcharge.mutex_hdl);

    sg_overcharge.last_power_value = power_value;
   
    if (sg_overcharge.enable && FALSE == sg_overcharge.is_start && TRUE == sg_overcharge.chan_state) {
        TAL_PR_DEBUG("overflow power threshold, start overcharge");
        sg_overcharge.is_start = TRUE;
    }

    if (FALSE == sg_overcharge.enable || FALSE == sg_overcharge.is_start || FALSE == sg_overcharge.chan_state) {
        TAL_PR_DEBUG("overcharge not enable or not start");
        goto __EXIT;
    }

    if (power_value > sg_overcharge.power_threshold) {
        TAL_PR_DEBUG("overcharge stop, cur power: %d", power_value);
        tal_sw_timer_stop(sg_overcharge.timer_id);
    } else {
        if (FALSE == tal_sw_timer_is_running(sg_overcharge.timer_id)) {
            TAL_PR_DEBUG("overcharge start");
            tal_sw_timer_start(sg_overcharge.timer_id, sg_overcharge.overcharge_s*1000, TAL_TIMER_ONCE);
        }
    }

__EXIT:
    tal_mutex_unlock(sg_overcharge.mutex_hdl);

    return rt;
}

OPERATE_RET app_elec_overcharge_status_set(BOOL_T is_enable)
{
    OPERATE_RET rt = OPRT_OK;

    if (NULL == sg_overcharge.cb) {
        TAL_PR_DEBUG("overcharge not init");
        return OPRT_OK;
    }

    tal_mutex_lock(sg_overcharge.mutex_hdl);

    sg_overcharge.enable = (is_enable==0) ? (0) : (1);

    if (FALSE == sg_overcharge.enable) {
        TUYA_CALL_ERR_GOTO(tal_sw_timer_stop(sg_overcharge.timer_id), __EXIT);
        TAL_PR_DEBUG("stop overcharge timer");
    } else {
        sg_overcharge.is_start = TRUE;
    }

    TUYA_CALL_ERR_GOTO(app_elec_overcharge_dp_upload(), __EXIT);
    TUYA_CALL_ERR_GOTO(tfm_kv_uf_storage_write_data(OVERCHARGE_KEY, (UCHAR_T *)&sg_overcharge.enable, SIZEOF(BOOL_T)), __EXIT);

__EXIT:
    tal_mutex_unlock(sg_overcharge.mutex_hdl);

    app_elec_overcharge_detect(sg_overcharge.last_power_value);

    return rt;
}

OPERATE_RET app_elec_overcharge_dp_upload(VOID_T)
{
    OPERATE_RET rt;

    TY_OBJ_DP_S dp_obj_data  ={0};

    if (NULL == sg_overcharge.cb) {
        TAL_PR_DEBUG("overcharge not init");
        return OPRT_OK;
    }

    dp_obj_data.dpid = ENERGY_METER_OVERCHARGE_DPID;
    dp_obj_data.type = PROP_BOOL;
    dp_obj_data.value.dp_bool = sg_overcharge.enable;
    dp_obj_data.time_stamp = 0;

    TUYA_CALL_ERR_LOG(dev_report_dp_json_async(NULL, &dp_obj_data, 1));

    return rt;
}

OPERATE_RET app_elec_overcharge_date_easer(VOID_T)
{
    return tfm_kv_uf_storage_erase_data(OVERCHARGE_KEY);
}

#endif /* ENABLE_ENERGY_OVERCHARGE */
