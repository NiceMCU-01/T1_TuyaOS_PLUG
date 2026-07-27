/**
 * @file ty_app_elec_trigger_network.c
 * @author www.tuya.com
 * @brief ty_app_elec_trigger_network module is used to 
 * @version 0.1
 * @date 2023-04-18
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_app_config.h"

#include "ty_app_starts_up_intf.h"

#include "ty_app_elec_event_code.h"

#include "tal_log.h"


#if (defined(ELEC_FFC_BEACON_REMOTE_EN) && (ELEC_FFC_BEACON_REMOTE_EN==1))
#include "app_elec_ffc.h"
#endif

/***********************************************************
************************macro define************************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/


/***********************************************************
***********************function define**********************
***********************************************************/
/**
* @brief gw 状态改变处理回调
*
* @param[in] status tuya sdk gateway status info
* @return 
*/
VOID_T ty_app_gw_status_changed_cb(IN CONST GW_STATUS_E status)
{
    return;
}

/**
* @brief 复位状态处理
*
* @param[in] type tuya sdk gateway reset type
* @return 
*/
VOID_T ty_app_gw_reset_cb(GW_RESET_TYPE_E type)
{
    TAL_PR_DEBUG("GW_RESET_TYPE_E: %d", type);

    if (type == GW_LOCAL_RESET_FACTORY || \
        type == GW_REMOTE_RESET_FACTORY || \
        type == GW_RESET_DATA_FACTORY) {
        ty_app_event_post_synchronous(APP_EVT_GROUP_ELE, EVT_ELEC_DEV_DATA_ERASE, NULL, 0x00);
    }

    return;
}

/**
* @brief 网络状态回调
*
* @param[in] stat tuya sdk definition of wifi-net status
* @return 
*/
VOID ty_app_wf_nw_stat_cb(IN CONST GW_WIFI_NW_STAT_E stat)
{
    STATIC BOOL_T s_syn_all_status = FALSE;
    STATIC BOOL_T s_elec_indicate_status = FALSE;
    TAL_PR_NOTICE("--->net state change to:%d", stat);
    
    switch (stat) {
    case STAT_UNPROVISION:
        s_elec_indicate_status = TRUE;
        ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_EZ_INDICATE, NULL, 0);
    break;
    case STAT_AP_STA_UNCFG:
    #ifdef STAT_UNPROVISION_AP_STA_UNCFG
    case STAT_UNPROVISION_AP_STA_UNCFG:
    #endif
        s_elec_indicate_status = TRUE;
        ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_AP_INDICATE, NULL, 0);
    break;
    case STAT_LOW_POWER:
    case STAT_AP_STA_DISC:
    case STAT_STA_DISC:
        ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_NET_INDICATE_NOT_CONNECT, NULL, 0);
    break;
    // connect router
    case STAT_AP_STA_CONN:
    case STAT_STA_CONN:
        ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_NET_INDICATE_CONNECTED, NULL, 0);
        //首次激活的时候，不能在连上路由之后就同步dp
        if (FALSE == s_syn_all_status && FALSE == s_elec_indicate_status) {
            s_syn_all_status = TRUE;
            ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_REPORT_DP_ALL, NULL, 0);
        }
    break;
    // connect cloud
    case STAT_CLOUD_CONN:
    case STAT_AP_CLOUD_CONN:
        if (FALSE == s_syn_all_status) {
            s_syn_all_status = TRUE;
            ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_REPORT_DP_ALL, NULL, 0);
        }
#if defined (ELEC_RUNTIME_SWITCH_EN) && (ELEC_RUNTIME_SWITCH_EN == 1)
        ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_RUNTIME_SWITCH_SET, NULL, 0);
#endif
    break;
    // BT ACTIVED
    case STAT_PROXY_ACTIVED:
        ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_NET_INDICATE_CONNECTED, NULL, 0);
        ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_REPORT_DP_ALL, NULL, 0);
        break;
    default:
        break;
    }
#if (ELEC_FFC_BEACON_REMOTE_EN == 1)
    if (stat >= STAT_STA_CONN){
        app_Wifi_Ffc_Register();
    }
#endif
}
