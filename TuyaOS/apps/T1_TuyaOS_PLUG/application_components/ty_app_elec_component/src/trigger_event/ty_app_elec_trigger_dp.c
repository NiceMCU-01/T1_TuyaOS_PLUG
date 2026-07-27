/**
 * @file ty_app_elec_trigger_dp.c
 * @author www.tuya.com
 * @brief ty_app_elec_trigger_dp module is used to 
 * @version 0.1
 * @date 2023-04-17
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_app_config.h"

#include "ty_sys.h"
#include "ty_app_starts_up_intf.h"

#include "ty_app_elec_event_code.h"

#include "app_elec_channel.h"
#include "app_elec_delay_off_timer.h"
#include "app_elec_timer_random.h"
#include "app_elec_timer_cycle.h"
#include "app_elec_timer_countdown.h"

/***********************************************************
************************macro define************************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    BYTE_T dpid;
    UINT32_T event_id;
    UINT_T chan_id;
} __CHANNEL_INFO_T;

typedef struct {
    BYTE_T dpid;
    UINT32_T event_id;
    UINT_T cd_id;
} __COUNTDOWN_TIME_INFO_T;

/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC __CHANNEL_INFO_T sg_chan_info[] = {
#ifdef ELEC_CHANNEL_1_SWITCH_DPID
    {ELEC_CHANNEL_1_SWITCH_DPID, EVT_ELEC_CHANNEL_STATUS_SET, 1},
#endif
#ifdef ELEC_CHANNEL_2_SWITCH_DPID
    {ELEC_CHANNEL_2_SWITCH_DPID, EVT_ELEC_CHANNEL_STATUS_SET, 2},
#endif
#ifdef ELEC_CHANNEL_3_SWITCH_DPID
    {ELEC_CHANNEL_3_SWITCH_DPID, EVT_ELEC_CHANNEL_STATUS_SET, 3},
#endif
#ifdef ELEC_CHANNEL_4_SWITCH_DPID
    {ELEC_CHANNEL_4_SWITCH_DPID, EVT_ELEC_CHANNEL_STATUS_SET, 4},
#endif
#ifdef ELEC_CHANNEL_5_SWITCH_DPID
    {ELEC_CHANNEL_5_SWITCH_DPID, EVT_ELEC_CHANNEL_STATUS_SET, 5},
#endif
#ifdef ELEC_CHANNEL_6_SWITCH_DPID
    {ELEC_CHANNEL_6_SWITCH_DPID, EVT_ELEC_CHANNEL_STATUS_SET, 6},
#endif
#ifdef ELEC_CHANNEL_7_SWITCH_DPID
    {ELEC_CHANNEL_7_SWITCH_DPID, EVT_ELEC_CHANNEL_STATUS_SET, 7},
#endif
#ifdef ELEC_CHANNEL_8_SWITCH_DPID
    {ELEC_CHANNEL_8_SWITCH_DPID, EVT_ELEC_CHANNEL_STATUS_SET, 8},
#endif
};

STATIC __COUNTDOWN_TIME_INFO_T sg_cd_info[] = {
#ifdef ELEC_COUNTDOWN_TM_1_DPID
    {ELEC_COUNTDOWN_TM_1_DPID, EVT_ELEC_COUNTDOWN_TIME_SET, 1},
#endif
#ifdef ELEC_COUNTDOWN_TM_2_DPID
    {ELEC_COUNTDOWN_TM_2_DPID, EVT_ELEC_COUNTDOWN_TIME_SET, 2},
#endif
#ifdef ELEC_COUNTDOWN_TM_3_DPID
    {ELEC_COUNTDOWN_TM_3_DPID, EVT_ELEC_COUNTDOWN_TIME_SET, 3},
#endif
#ifdef ELEC_COUNTDOWN_TM_4_DPID
    {ELEC_COUNTDOWN_TM_4_DPID, EVT_ELEC_COUNTDOWN_TIME_SET, 4},
#endif
#ifdef ELEC_COUNTDOWN_TM_5_DPID
    {ELEC_COUNTDOWN_TM_5_DPID, EVT_ELEC_COUNTDOWN_TIME_SET, 5},
#endif
#ifdef ELEC_COUNTDOWN_TM_6_DPID
    {ELEC_COUNTDOWN_TM_6_DPID, EVT_ELEC_COUNTDOWN_TIME_SET, 6},
#endif
#ifdef ELEC_COUNTDOWN_TM_7_DPID
    {ELEC_COUNTDOWN_TM_7_DPID, EVT_ELEC_COUNTDOWN_TIME_SET, 7},
#endif
#ifdef ELEC_COUNTDOWN_TM_8_DPID
    {ELEC_COUNTDOWN_TM_8_DPID, EVT_ELEC_COUNTDOWN_TIME_SET, 8},
#endif
};

/***********************************************************
***********************function define**********************
***********************************************************/

VOID_T event_dp_trigger_channel_dpid_set(UINT8_T index, BYTE_T dpid)
{
    sg_chan_info[index].dpid = dpid;
    return;
}

VOID_T __ty_app_channel_proc(CONST TY_OBJ_DP_S *root)
{
    UINT8_T i = 0, chan_num = 0;
    APP_ELEC_CHANNEL_CFG_T chan_cfg = {0};

    chan_num = CNTSOF(sg_chan_info);

    for (i=0; i<chan_num; i++) {
        if (sg_chan_info[i].dpid == root->dpid) {
            chan_cfg.chan_id = sg_chan_info[i].chan_id;
            chan_cfg.status = root->value.dp_bool;
            ty_app_event_post(APP_EVT_GROUP_ELE, sg_chan_info[i].event_id, &chan_cfg, SIZEOF(APP_ELEC_CHANNEL_CFG_T));
            break;
        }
    }

    return;
}

VOID_T event_dp_trigger_cd_time_dpid_set(UINT8_T index, BYTE_T dpid)
{
    sg_cd_info[index].dpid = dpid;
    return;
}

VOID_T __ty_app_countdown_time_proc(CONST TY_OBJ_DP_S *root)
{
    UINT8_T i = 0, cd_num = 0;
    APP_ELEC_COUNTDOWN_CFG_T countdown_cfg = {0};

    cd_num = CNTSOF(sg_cd_info);

    for (i=0; i<cd_num; i++) {
        if (sg_cd_info[i].dpid == root->dpid) {
            countdown_cfg.id = sg_cd_info[i].cd_id;
            countdown_cfg.time_s =  root->value.dp_value;
            ty_app_event_post(APP_EVT_GROUP_ELE, sg_cd_info[i].event_id, &countdown_cfg, SIZEOF(APP_ELEC_COUNTDOWN_CFG_T));
            
            break;
        }
    }

    return;
}

STATIC VOID_T __ty_app_elec_obj_dp_proc(CONST TY_OBJ_DP_S *root)
{
    BYTE_T dpid = 0;

    if (NULL == root) {
        return;
    }

    dpid = root->dpid;

    TAL_PR_DEBUG("dp_id: %d", dpid);

    switch (dpid) {
#if (defined(ENABLE_ELEC_DELAY_OFF_TIMER) && (ENABLE_ELEC_DELAY_OFF_TIMER==1))
        case ELEC_DELAY_OFF_DPID: {// 点动开关
            APP_ELEC_DELAY_OFF_DATA_T *delay_off_data = NULL;
            UINT32_T rt_size = 0;
            rt_size = app_elec_delay_off_dp_data_parse(root->value.dp_str, &delay_off_data);
            if (rt_size < OPRT_OK) { break; }
            TAL_PR_DEBUG("delay_off_data num: %d", delay_off_data->num);
            ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_DELAY_OFF_SET, delay_off_data, rt_size);
            if (NULL != delay_off_data) {
                tal_free(delay_off_data);
                delay_off_data = NULL;
            }
        } break;
#endif
        case ELEC_CHANNEL_POWER_ON_DPID: {
            UINT_T power_on_mode = root->value.dp_enum;
            ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_POWER_ON_MODE_SET, &power_on_mode, sizeof(UINT_T));
        } break;
#if (defined(ENABLE_ELEC_CHILD_LOCK) && (ENABLE_ELEC_CHILD_LOCK==1))
        case ELEC_CHILD_LOCK_DPID : {
            if (root->value.dp_bool == FALSE) {
                ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_CHILD_LOCK_DISABLE, NULL, 0);
#if (defined(ELEC_CHILD_LOCK_AUTO_LOCK_ENABLE ) && (ELEC_CHILD_LOCK_AUTO_LOCK_ENABLE==1))
                // APP 上关闭童锁功能，会将自动上锁也给关闭
                ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_CHILD_AUTO_LOCK_DISABLE, NULL, 0);
#endif /* ELEC_CHILD_LOCK_AUTO_LOCK_ENABLE */
            } else {
                ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_CHILD_LOCK_ENABLE, NULL, 0);
#if (defined(ELEC_CHILD_LOCK_AUTO_LOCK_ENABLE ) && (ELEC_CHILD_LOCK_AUTO_LOCK_ENABLE==1))
                ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_CHILD_AUTO_LOCK_ENABLE, NULL, 0);
#endif /* ELEC_CHILD_LOCK_AUTO_LOCK_ENABLE */
            }
        } break;
#endif
        case ELEC_LIGHT_MODE_DPID: {
            UINT_T light_mode = root->value.dp_enum;
            TAL_PR_DEBUG("light mode: %d", light_mode);
            ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_LIGHT_MODE_SET, &light_mode, SIZEOF(UINT_T));
        } break;

#ifdef ELEC_CYCLE_TIMER_DPID
        case ELEC_CYCLE_TIMER_DPID:{
            TAL_PR_DEBUG("cycle timer: %s", root->value.dp_str);
            ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_CYCLE_TIMER_STR_SET, root->value.dp_str, strlen(root->value.dp_str)+1);
        }
        break;
#endif /* ELEC_CYCLE_TIMER_DPID */

#ifdef ELEC_RANDOM_TIMER_DPID
        case ELEC_RANDOM_TIMER_DPID: {
            TAL_PR_DEBUG("random timer: %s", root->value.dp_str);
            ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_RANDOM_TIMER_STR_SET, root->value.dp_str, strlen(root->value.dp_str)+1);
        }
        break;
#endif /* ELEC_RANDOM_TIMER_DPID */

#ifdef ENERGY_METER_OVERCHARGE_DPID
        case ENERGY_METER_OVERCHARGE_DPID: {
            BOOL_T overcharge_enable = root->value.dp_bool;
            ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_OVERCHARGE_SET, &overcharge_enable, SIZEOF(BOOL_T));
        } break;
#endif /* ENERGY_METER_OVERCHARGE_DPID */
    default:
        __ty_app_channel_proc(root);
        __ty_app_countdown_time_proc(root);
        
        break;
    }

    return;
}

/**
* @brief 普通dp回调
*
* @param[in] dp
* @return 
*/
VOID_T ty_app_obj_dp_handle(IN CONST TY_RECV_OBJ_DP_S *dp)
{
    UINT8_T i = 0;

    if (NULL == dp) {
        TAL_PR_ERR("dp error");
        return;
    }

    TAL_PR_DEBUG("## dp_cnt:%d,cmd_tp:%d,dtt_tp:%d ##,", dp->dps_cnt, dp->cmd_tp, dp->dtt_tp);

    for (i = 0; i < dp->dps_cnt; i++) {
        __ty_app_elec_obj_dp_proc(&dp->dps[i]);
    }

    return;
}

/**
* @brief raw dp 处理回调
*
* @param[in] dp
* @return 
*/
VOID_T ty_app_raw_dp_handle(IN CONST TY_RECV_RAW_DP_S *dp)
{
    if (NULL == dp) {
        TAL_PR_ERR("dp error");
        return;
    }

    TAL_PR_DEBUG("## raw dp, dpid:%d ##,", dp->dpid);

    return;
}

/**
* @brief dp查询回调
*
* @param[in] dp_qry
* @return 
*/
VOID_T ty_app_dp_query_handle(IN CONST TY_DP_QUERY_S *dp_qry)
{
    UINT32_T i = 0;
    UINT8_T dpid = 0;
    
    if (0 == dp_qry->cnt) {
        ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_REPORT_DP_ALL, NULL, 0);
    } else {
        for (i=0; i<dp_qry->cnt; i++) {
            dpid = dp_qry->dpid[i];
            ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_REPORT_DP, &dpid, SIZEOF(UINT8_T));
        }
    }

    return;
}

/**
* @brief ble dp查询回调
*
* @param[in] VOID_T
* @return VOID
*/
VOID_T ty_app_ble_dp_query_handle(VOID_T)
{
    TAL_PR_NOTICE("ble dp query");
    ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_REPORT_DP_ALL, NULL, 0);
    return;
}
