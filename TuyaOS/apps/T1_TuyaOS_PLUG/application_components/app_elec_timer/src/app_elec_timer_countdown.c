/**
 * @file app_elec_timer_countdown.c
 * @author www.tuya.com
 * @brief app_elec_timer_countdown module is used to 
 * @version 0.1
 * @date 2023-05-04
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_app_config.h"
#include "ty_sys.h"

#if (defined(ENABLE_ELEC_COUNTDOWN_TIMER) && (ENABLE_ELEC_COUNTDOWN_TIMER==1))
#include "tbl_countdown_timer.h"
#include "app_elec_timer_countdown.h"

#if (defined(ENABLE_TY_MATTER) && (ENABLE_TY_MATTER == 1))
#include "app_matter_proc.h"
#endif
/***********************************************************
************************macro define************************
***********************************************************/
typedef struct {
    UINT8_T cd_id;
    UINT8_T dpid;
    TBL_CD_TM_HANDLE tbl_cd_hdl;
} APP_CD_TM_T;

/***********************************************************
***********************typedef define***********************
***********************************************************/
#define ELEC_COUNTDOWN_INIT(seq)            \
{                                           \
    .dpid = ELEC_COUNTDOWN_TM_##seq##_DPID, \
    .tbl_cd_hdl = NULL,                     \
}                                           \

/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/

STATIC UINT8_T sg_cd_time_num = ELEC_COUNTDOWN_TIMER_NUM;

STATIC APP_ELEC_COUNTDOWN_CB sg_countdown_cb = NULL;

STATIC APP_CD_TM_T sg_cd[ELEC_COUNTDOWN_TIMER_NUM] = {
#ifdef ELEC_COUNTDOWN_TM_1_DPID
    ELEC_COUNTDOWN_INIT(1),
#endif
#ifdef ELEC_COUNTDOWN_TM_2_DPID
    ELEC_COUNTDOWN_INIT(2),
#endif
#ifdef ELEC_COUNTDOWN_TM_3_DPID
    ELEC_COUNTDOWN_INIT(3),
#endif
#ifdef ELEC_COUNTDOWN_TM_4_DPID
    ELEC_COUNTDOWN_INIT(4),
#endif
#ifdef ELEC_COUNTDOWN_TM_5_DPID
    ELEC_COUNTDOWN_INIT(5),
#endif
#ifdef ELEC_COUNTDOWN_TM_6_DPID
    ELEC_COUNTDOWN_INIT(6),
#endif
#ifdef ELEC_COUNTDOWN_TM_7_DPID
    ELEC_COUNTDOWN_INIT(7),
#endif
#ifdef ELEC_COUNTDOWN_TM_8_DPID
    ELEC_COUNTDOWN_INIT(8),
#endif
};

/***********************************************************
***********************function define**********************
***********************************************************/
VOID_T __tbl_countdown_time_cb(TBL_CD_TM_HANDLE cd_hdl, IN UINT32_T remain_s, VOID_T *args)
{
    APP_CD_TM_T *app_cd_data = (APP_CD_TM_T *)args;

    if (0 == remain_s && NULL != sg_countdown_cb) {
        sg_countdown_cb(app_cd_data->cd_id);
    }
    app_elec_countdown_time_upload(app_cd_data->cd_id, remain_s);

    return;
}

VOID_T app_elec_countdown_time_num_set(UINT8_T num)
{
    sg_cd_time_num = num;
    TAL_PR_DEBUG("countdown time number: %d", sg_cd_time_num);
    return;
}

OPERATE_RET app_elec_countdown_time_init(APP_ELEC_COUNTDOWN_CB cb)
{
    OPERATE_RET rt = OPRT_OK;
    UINT8_T i = 0;

    TUYA_CHECK_NULL_RETURN(cb, OPRT_INVALID_PARM);

    sg_countdown_cb = cb;

    for (i=0; i<sg_cd_time_num; i++) {
        sg_cd[i].cd_id = i+1;
        TUYA_CALL_ERR_LOG(tbl_countdown_time_create(__tbl_countdown_time_cb, &sg_cd[i], &sg_cd[i].tbl_cd_hdl));
    }

    return rt;
}

OPERATE_RET app_elec_countdown_time_set(APP_ELEC_COUNTDOWN_CFG_T countdown_cfg)
{
    OPERATE_RET rt = OPRT_OK;
    UINT8_T index = 0;

    if (countdown_cfg.id <= 0 || countdown_cfg.id > sg_cd_time_num) {
        TAL_PR_ERR("countdown id %d invalid, max cd number: %d", countdown_cfg.id, sg_cd_time_num);
        return OPRT_INVALID_PARM;
    }
    index = countdown_cfg.id-1;

    TUYA_CALL_ERR_RETURN(tbl_countdown_time_stop(sg_cd[index].tbl_cd_hdl));

    if (countdown_cfg.time_s > 0) {
        TUYA_CALL_ERR_RETURN(tbl_countdown_time_start(sg_cd[index].tbl_cd_hdl, 30, countdown_cfg.time_s));
    }

    TUYA_CALL_ERR_RETURN(app_elec_countdown_time_upload(countdown_cfg.id, countdown_cfg.time_s));

    return rt;
}

OPERATE_RET app_elec_countdown_time_upload(UINT8_T countdown_id, UINT_T remain_sec)
{
    OPERATE_RET rt = OPRT_OK;
    UINT8_T index = countdown_id - 1;

    if (countdown_id<=0 || countdown_id > sg_cd_time_num) {
        return OPRT_INVALID_PARM;
    }

    TY_OBJ_DP_S dp_obj_data = {0};

    dp_obj_data.dpid = sg_cd[index].dpid;
    dp_obj_data.type = PROP_VALUE;
    dp_obj_data.value.dp_value = remain_sec;
    dp_obj_data.time_stamp = 0;

    TAL_PR_DEBUG("upload dp %d, index: %d", sg_cd[index].dpid, index);

    TUYA_CALL_ERR_LOG(dev_report_dp_json_async(NULL, &dp_obj_data, 1));
#if (defined(ENABLE_TY_MATTER) && (ENABLE_TY_MATTER == 1))
    /*matter support*/
    app_matter_private_countdown_timer_set(index,remain_sec, MATTER_OPR_DPCODE);
#endif
    return rt;
}

OPERATE_RET app_elec_countdown_time_upload_by_dpid(BYTE_T dpid)
{
    OPERATE_RET rt = OPRT_OK;
    UINT8_T target_index = 0;
    TY_OBJ_DP_S dp_obj_data = {0};

    for (target_index=0; target_index<sg_cd_time_num; target_index++) {
        if (dpid == sg_cd[target_index].dpid) {
            break;
        }
    }

    if (target_index>=sg_cd_time_num) {
        TAL_PR_DEBUG("cd time not find dpid %d", dpid);
        return OPRT_COM_ERROR;
    }

    dp_obj_data.dpid = sg_cd[target_index].dpid;
    dp_obj_data.type = PROP_VALUE;
    dp_obj_data.value.dp_value = tbl_countdown_time_get_remain_sec(sg_cd[target_index].tbl_cd_hdl);
    dp_obj_data.time_stamp = 0;

    TAL_PR_DEBUG("upload dp %d, value: %d", dp_obj_data.dpid, dp_obj_data.value.dp_value);

    TUYA_CALL_ERR_LOG(dev_report_dp_json_async(NULL, &dp_obj_data, 1));

    return rt;
}

OPERATE_RET app_elec_countdown_time_all_upload(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    UINT8_T i = 0;

    TY_OBJ_DP_S dp_obj_data[ELEC_COUNTDOWN_TIMER_NUM] = {{0}};

    for (i=0; i<sg_cd_time_num; i++) {
        dp_obj_data[i].dpid = sg_cd[i].dpid;
        dp_obj_data[i].type = PROP_VALUE;
        dp_obj_data[i].value.dp_value = tbl_countdown_time_get_remain_sec(sg_cd[i].tbl_cd_hdl);
        dp_obj_data[i].time_stamp = 0;
    }

    TUYA_CALL_ERR_LOG(dev_report_dp_json_async(NULL, dp_obj_data, sg_cd_time_num));

    return rt;
}

VOID_T app_elec_countdown_dpid_set(UINT8_T countdown_id, BYTE_T dpid)
{
    UINT8_T index = countdown_id - 1;

    if (countdown_id<=0 || countdown_id > sg_cd_time_num) {
        TAL_PR_ERR("input params fail");
        return;
    }

    sg_cd[index].dpid = dpid;

    return;
}

OPERATE_RET app_elec_countdown_time_remain_sec_get(UINT8_T countdown_id, UINT32_T *remain_sec)
{
    UINT8_T idx = countdown_id - 1;

    if (countdown_id<=0 || countdown_id > sg_cd_time_num) {
        return OPRT_INVALID_PARM;
    }

    *remain_sec = tbl_countdown_time_get_remain_sec(sg_cd[idx].tbl_cd_hdl);

    return OPRT_OK;;
}

#endif /* ENABLE_ELEC_COUNTDOWN_TIMER */
