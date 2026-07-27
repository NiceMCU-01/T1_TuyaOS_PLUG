/**
 * @file app_elec_child_lock.c
 * @author www.tuya.com
 * @brief app_elec_child_lock module is used to 
 * @version 0.1
 * @date 2023-04-21
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_app_config.h"
#include "ty_sys.h"

#if (defined(ENABLE_ELEC_CHILD_LOCK) && (ENABLE_ELEC_CHILD_LOCK==1))
#include "app_elec_child_lock.h"

/***********************************************************
************************macro define************************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    UINT16_T dpid;
    MUTEX_HANDLE mutex_hdl;
    APP_ELEC_CHILD_LOCK_CB usr_cb;
    CHILD_LOCK_STATUS_E lock_status;
    TIMER_ID auto_lock_timer_id;
    UINT8_T auto_lock_enable;
    UINT32_T auto_lock_time_ms;
} APP_ELEC_CHILD_LOCK_T;

/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC APP_ELEC_CHILD_LOCK_T sg_child_lock ={
    .dpid = ELEC_CHILD_LOCK_DPID,
    .mutex_hdl = NULL,
    .usr_cb = NULL,
    .lock_status = STATUS_UNLOCK,
#if (defined(ELEC_CHILD_LOCK_AUTO_LOCK_ENABLE) && (ELEC_CHILD_LOCK_AUTO_LOCK_ENABLE==1))
    .auto_lock_time_ms = ELEC_CHILD_LOCK_AUTO_LOCK_TIME_MS,
#else
    .auto_lock_enable = 0,
#endif
};

/***********************************************************
***********************function define**********************
***********************************************************/

STATIC VOID_T __child_lock_status_set(CHILD_LOCK_STATUS_E status)
{
    if (sg_child_lock.lock_status == status) {
        return;
    }

    tal_mutex_lock(sg_child_lock.mutex_hdl);
    sg_child_lock.lock_status = status;

    if (NULL != sg_child_lock.usr_cb) {
        sg_child_lock.usr_cb(sg_child_lock.lock_status);
    }

    tal_mutex_unlock(sg_child_lock.mutex_hdl);

    app_elec_child_lock_dp_data_upload();

    return;
}

STATIC VOID_T __auto_lock_timer_cb(TIMER_ID timer_id, VOID_T *arg)
{
    __child_lock_status_set(STATUS_LOCK);
    return;
}

OPERATE_RET app_elec_child_lock_init(APP_ELEC_CHILD_LOCK_CONFIG_T *usr_cfg, APP_ELEC_CHILD_LOCK_CB usr_cb)
{
    OPERATE_RET rt = OPRT_OK;

    TUYA_CHECK_NULL_RETURN(usr_cb, OPRT_INVALID_PARM);
    sg_child_lock.usr_cb = usr_cb;

    // 如果用户进行了配置，使用用户配置，否则使用默认配置
    if (NULL != usr_cfg) {
        sg_child_lock.auto_lock_enable = usr_cfg->auto_lock_enable;
        sg_child_lock.auto_lock_time_ms = usr_cfg->auto_lock_time_ms;
    }

    if (sg_child_lock.auto_lock_enable) {
        TUYA_CALL_ERR_RETURN(tal_sw_timer_create(__auto_lock_timer_cb, &sg_child_lock, &sg_child_lock.auto_lock_timer_id));
    }

    TUYA_CALL_ERR_RETURN(tal_mutex_create_init(&sg_child_lock.mutex_hdl));

    return rt;
}

OPERATE_RET app_elec_child_lock_status_set(CHILD_LOCK_STATUS_E status)
{
    OPERATE_RET rt = OPRT_OK;

    status = ((status == STATUS_UNLOCK) ? (STATUS_UNLOCK) : (STATUS_LOCK));

    __child_lock_status_set(status);
    TAL_PR_DEBUG("child lock status: %d", status);

    if (STATUS_UNLOCK == sg_child_lock.lock_status && sg_child_lock.auto_lock_enable) {
        TUYA_CALL_ERR_RETURN(tal_sw_timer_start(sg_child_lock.auto_lock_timer_id, sg_child_lock.auto_lock_time_ms, TAL_TIMER_ONCE));
        TAL_PR_DEBUG("child lock auto lock, remain time: %d", sg_child_lock.auto_lock_time_ms);
    }

    return rt;
}

CHILD_LOCK_STATUS_E app_elec_child_lock_status_get(VOID_T)
{
    return sg_child_lock.lock_status;
}

OPERATE_RET app_elec_child_auto_lock_status_set(UINT8_T auto_lock_enable)
{
    OPERATE_RET rt = OPRT_OK;

    if (sg_child_lock.auto_lock_enable == FALSE) {
        TAL_PR_DEBUG("child lock auto lock is not enable");
        return OPRT_OK;
    }

    auto_lock_enable = (auto_lock_enable>0) ? (TRUE) : (FALSE);

    if (sg_child_lock.auto_lock_enable == auto_lock_enable) {
        return OPRT_OK;
    }

    tal_mutex_lock(sg_child_lock.mutex_hdl);

    sg_child_lock.auto_lock_enable = auto_lock_enable;

    if (TRUE == sg_child_lock.auto_lock_enable && STATUS_UNLOCK == sg_child_lock.lock_status) {
        TUYA_CALL_ERR_LOG(tal_sw_timer_start(sg_child_lock.auto_lock_timer_id, sg_child_lock.auto_lock_time_ms, TAL_TIMER_ONCE));
    } else if (FALSE == sg_child_lock.auto_lock_enable && tal_sw_timer_is_running(sg_child_lock.auto_lock_timer_id)) {
        TUYA_CALL_ERR_LOG(tal_sw_timer_stop(sg_child_lock.auto_lock_timer_id));
    }

    tal_mutex_unlock(sg_child_lock.mutex_hdl);

    return rt;
}

UINT8_T app_elec_child_auto_lock_status_get(VOID_T)
{
    return sg_child_lock.auto_lock_enable;
}

OPERATE_RET app_elec_child_lock_dp_data_upload(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    TY_OBJ_DP_S dp_obj_data = {0};

    dp_obj_data.dpid = sg_child_lock.dpid;
    dp_obj_data.type = PROP_BOOL;
    dp_obj_data.value.dp_bool = sg_child_lock.lock_status;
    dp_obj_data.time_stamp = 0;

    TUYA_CALL_ERR_LOG(dev_report_dp_json_async(NULL, &dp_obj_data, 1));

    return rt;
}

OPERATE_RET app_elec_child_auto_lock_refresh(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

    if (FALSE == sg_child_lock.auto_lock_enable) {
        return OPRT_OK;
    }

    tal_mutex_lock(sg_child_lock.mutex_hdl);

    TAL_PR_DEBUG("child lock status: %d", sg_child_lock.lock_status);
    if (STATUS_UNLOCK == sg_child_lock.lock_status) {
        TAL_PR_DEBUG("refresh auto lock timer");
        TUYA_CALL_ERR_LOG(tal_sw_timer_start(sg_child_lock.auto_lock_timer_id, sg_child_lock.auto_lock_time_ms, TAL_TIMER_ONCE));
    }

    tal_mutex_unlock(sg_child_lock.mutex_hdl);

    return rt;
}

#endif /* ENABLE_ELEC_CHILD_LOCK */
