/***********************************************************
*  File: tuya_iot_wifi_api.c
*  Author: nzy
*  Date: 20170922
***********************************************************/

#include "tuya_app_config.h"

#if (defined(ENABLE_TY_MATTER) && (ENABLE_TY_MATTER==1))

#include "uni_log.h"
#include "sdk_version.h"
#include "smart_frame.h"
#include "gw_intf.h"
// #include "tuya_iot_wifi_api.h"
#include "tuya_ws_db.h"
#include "uni_base64.h"
#include "tuya_cloud_wifi_defs.h"
#include "mix_method.h"
#include "tal_workq_service.h"
#include "tuya_svc_upgrade.h"
#include "base_event.h"
#include "ws_db_gw.h"
#include "mqc_app.h"
#if defined(ENABLE_BT_SERVICE) && (ENABLE_BT_SERVICE == 1)
#include "tuya_bt_link.h"
#endif
#include "tuya_wifi_link.h"
#include "tuya_wifi_netcfg.h"
#include "tuya_wifi_connect.h"
#include "tuya_wifi_status.h"
#include "tuya_wifi_reset.h"
#include "tuya_svc_devos.h"
#include "netcfg_module.h"
#if defined(ENABLE_ASTRO_TIMER) && (ENABLE_ASTRO_TIMER == 1)
#include "astro_timer.h"
#endif
#if defined(ENABLE_QRCODE_ACTIVE) && (ENABLE_QRCODE_ACTIVE==1)
#include "svc_netcfg_qrcode.h"
#endif
#if defined(ENABLE_RTSP_SERVER)
#include "tuya_svc_lan_rtsp.h"
#endif
#if defined(ENABLE_WIFI_PEGASUS) && (ENABLE_WIFI_PEGASUS == 1)
#include "svc_pegasus.h"
#include "pegasus_netcfg.h"
#endif

#if defined(ENABLE_LWIP) && (ENABLE_LWIP == 1)
#include "ethernetif.h"
#endif

#if defined(ENABLE_RTSP_SERVER)
STATIC OPERATE_RET  __lan_enable_evt(VOID *data)
{
    tuya_svc_lan_rtsp_init();
    return OPRT_OK;
}
#endif

#if defined(ENABLE_MATTER) && (ENABLE_MATTER == 1) 
OPERATE_RET tal_matter_commission_stop(void);
#endif

// STATIC OPERATE_RET __iot_wf_reset_fast(IN CONST GW_WF_START_MODE wifi_start_mode, CONST GW_WF_CFG_MTHD_SEL mthd, IN CONST BOOL_T force_clean);

STATIC OPERATE_RET tuya_iot_matter_reset_init(CONST GW_WF_CFG_MTHD_SEL mthd);

STATIC OPERATE_RET  __devos_init_evt(VOID *data)
{
#if defined(ENABLE_RTSP_SERVER)
    ty_subscribe_event(EVENT_LAN_ENABLE, "api.wifi", __lan_enable_evt, 0);
#endif

    return OPRT_OK;
}

STATIC OPERATE_RET  __devos_run_evt(VOID *data)
{
#if defined(ENABLE_ASTRO_TIMER) && (ENABLE_ASTRO_TIMER==1)
    tuya_astro_timer_init();
#endif

    return OPRT_OK;
}

STATIC OPERATE_RET  __devos_reset_evt(VOID *data)
{
#if defined(ENABLE_ASTRO_TIMER) && (ENABLE_ASTRO_TIMER == 1)
    tuya_astro_timer_reset();
#endif

    return OPRT_OK;
}

/**
 * 监听DevOS状态切换事件，注册指定的配网方式
 *
 * 此处注册的配网方式，主要包括2类:
 * 1) 非特定连接独有的
 * 2) 非特定连接默认的
 */
STATIC OPERATE_RET  __devos_state_evt(VOID *data)
{
    if ((DEVOS_STATE_E)data != DEVOS_STATE_UNREGISTERED) {
        return OPRT_OK;
    }

#if defined(ENABLE_QRCODE_ACTIVE) && (ENABLE_QRCODE_ACTIVE==1)
    tuya_svc_netcfg_qrcode_init();
#endif

    return OPRT_OK;
}

/***********************************************************
*************************micro define***********************
***********************************************************/

/***********************************************************
*************************variable define********************
***********************************************************/

/***********************************************************
*************************function define********************
***********************************************************/
/***********************************************************

***********************************************************/

STATIC UINT_T g_long_timer_timeout_val = (900);    /*默认15分钟 900s*/

/***********************************************************
*  Function: tuya_iot_wf_timeout_set
*  Desc:     set wifi timeout
*  Input:    upload in sec
*  Note: must call first
***********************************************************/
VOID tuya_iot_matter_wf_timeout_set(IN CONST UINT_T timeout)
{
    g_long_timer_timeout_val = timeout;
    return;
}

/*
+    set network hostname to ap name from mf_test when EVENT_INIT occur that 
+    indicated that gw_wsm is read from flash and the network is not configured
+*/
STATIC OPERATE_RET __wifi_dev_set_network_hostname(PVOID_T data)
{
    /*set default network hostname: ap name from mf_test*/
    return tuya_iot_wf_set_network_hostname(NULL);
}

/***********************************************************
*  Function: tuya_iot_matter_wf_dev_init->tuya iot virtual initialization
*  Input: cfg
*         cbs->tuya wifi sdk user callbacks
*         product_key->product key/proudct id,get from tuya open platform
*         wf_sw_ver->wifi module software version format:xx.xx.xx (0<=x<=9)
*         attr
*         attr_num
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
STATIC OPERATE_RET tuya_iot_matter_wf_dev_init(IN CONST GW_WF_CFG_MTHD_SEL cfg, IN CONST GW_WF_START_MODE start_mode,
                                 IN CONST TY_IOT_CBS_S *cbs, IN CHAR_T *firmware_key,
                                 IN CHAR_T *product_key, IN CHAR_T *wf_sw_ver, IN CONST DEV_TYPE_T tp,
                                 IN GW_ATTACH_ATTR_T *attr, IN CONST UINT_T attr_num)
{
    if (NULL == product_key || NULL == wf_sw_ver || NULL == cbs) {
        return OPRT_INVALID_PARM;
    }
    OPERATE_RET rt = OPRT_OK;
    // 4.运行DevOS
    devos_init_t init_param = {0};

    // 1.订阅DevOS通用流程事件
    TUYA_CALL_ERR_RETURN(ty_subscribe_event(EVENT_INIT, "api.wifi", __devos_init_evt, SUBSCRIBE_TYPE_ONETIME));
    TUYA_CALL_ERR_RETURN(ty_subscribe_event(EVENT_RUN, "api.wifi", __devos_run_evt, SUBSCRIBE_TYPE_ONETIME));
#if defined(ENABLE_MATTER) && (ENABLE_MATTER == 1) 
    TUYA_CALL_ERR_RETURN(ty_subscribe_event(EVENT_INIT, "api.wifi", __wifi_dev_set_network_hostname, SUBSCRIBE_TYPE_ONETIME));
#endif
    TUYA_CALL_ERR_RETURN(ty_subscribe_event(EVENT_RESET, "api.wifi", __devos_reset_evt, SUBSCRIBE_TYPE_ONETIME));
    TUYA_CALL_ERR_RETURN(ty_subscribe_event(EVENT_DEVOS_STATE_CHANGE, "api.wifi", __devos_state_evt, SUBSCRIBE_TYPE_ONETIME));
 
    // 2.向DevOS注册网络连接
    TUYA_WIFI_CFG_PARAM cfg_param = {
        .mthd = cfg,
        .start_mode = start_mode,
#if defined(ENABLE_STATION_AP_MODE) && (ENABLE_STATION_AP_MODE==1)
        .enable_station_ap = TRUE,
#else
        .enable_station_ap = FALSE,
#endif
    };

#ifndef ENABLE_WIFI_EZ
    if (WF_START_SMART_ONLY == start_mode || WF_START_SMART_FIRST == start_mode || WF_START_AP_FIRST == start_mode) {
        cfg_param.start_mode = WF_START_SMART_AP_CONCURRENT;
    }
#endif

#if defined(ENABLE_MATTER) && (ENABLE_MATTER == 1) 
    GW_WORK_STAT_MAG_S gw_wsm  = {0};
    TUYA_CALL_ERR_RETURN(wd_gw_wsm_read(&gw_wsm));
        if (((GWCM_LOW_POWER_AUTOCFG == cfg_param.mthd) || (GWCM_SPCL_AUTOCFG == cfg_param.mthd)) && \
            GWNS_LOWPOWER == gw_wsm.nc_tp) {
            PR_NOTICE("reset wifi nc type to special");
            wifi_factory_reset_nc_type(&cfg_param, &gw_wsm);
            TUYA_CALL_ERR_RETURN(wd_gw_wsm_write(&gw_wsm));
        }
#endif

    if (OPRT_SVC_WIFI_NEED_FACTORY_RESET == tuya_svc_wifi_init(cfg_param)) {
        init_param.factory_reset = TRUE;
    }
#if defined(ENABLE_BT_SERVICE) && (ENABLE_BT_SERVICE == 1)
#if !defined(ENABLE_MATTER) || (ENABLE_MATTER == 0)
    TUYA_CALL_ERR_RETURN(tuya_svc_bt_init());
#endif
#endif

#if defined(ENABLE_WIFI_NETCFG) && (ENABLE_WIFI_NETCFG == 1)
    GW_WORK_STAT_MAG_S gw_wsm  = {0};
    TUYA_CALL_ERR_RETURN(wd_gw_wsm_read(&gw_wsm));
    TUYA_CALL_ERR_RETURN(tuya_iot_netcfg_init(cfg_param.start_mode, &gw_wsm));
#endif
    TUYA_CALL_ERR_RETURN(tuya_iot_matter_reset_init(cfg_param.mthd));

    // 3.向DevOS注册回调
#if defined(ENABLE_WIFI_PEGASUS) && (ENABLE_WIFI_PEGASUS == 1)
    /*注册闪电二次配网重连*/
    TUYA_CALL_ERR_LOG(pegasus_register_second_config());
#endif

    gw_register_cbs(cbs);

    init_param.abi = GW_VIRTUAL;
    init_param.tp = tp;
    init_param.firmware_key = firmware_key;
    init_param.product_key = product_key;
    init_param.sw_ver = wf_sw_ver;
    init_param.attr_num = attr_num;
    init_param.attrs = attr;
    TUYA_CALL_ERR_RETURN(tuya_svc_devos_init(&init_param));

    return rt;
}

/***********************************************************
*  Function: tuya_iot_matter_wf_soc_dev_init_param->The devcie consists of wifi soc
*  Input: cfg
*         cbs->tuya wifi sdk user callbacks,note cbs->dev_ug_cb is useless
*         product_key->product key/proudct id,get from tuya open platform
*         wf_sw_ver->wifi module software version format:xx.xx.xx (0<=x<=9)
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET tuya_iot_matter_wf_soc_dev_init_param(IN CONST GW_WF_CFG_MTHD_SEL cfg, IN CONST GW_WF_START_MODE start_mode,
                                           IN CONST TY_IOT_CBS_S *cbs, IN CHAR_T *firmware_key,
                                           IN CHAR_T *product_key, IN CHAR_T *wf_sw_ver)
{
    if (NULL == product_key || NULL == wf_sw_ver || NULL == cbs) {
        return OPRT_INVALID_PARM;
    }

    PR_NOTICE("wifi soc init. pid:%s firmwarekey:%s ver:%s",
        product_key, (NULL == firmware_key)?"NULL":firmware_key, wf_sw_ver);

    GW_ATTACH_ATTR_T attr = {0};
    attr.tp = DEV_NM_NOT_ATH_SNGL;
    strncpy(attr.ver, wf_sw_ver, SW_VER_LEN);

    return tuya_iot_matter_wf_dev_init(cfg, start_mode, cbs, firmware_key, product_key, wf_sw_ver, DEV_NM_ATH_SNGL, &attr, 1);
}


STATIC OPERATE_RET tuya_iot_netcfg_init(IN CONST GW_WF_START_MODE start_mode, IN GW_WORK_STAT_MAG_S* p_gw_wsm)
{
    OPERATE_RET rt = OPRT_OK;

    tuya_wifi_params_validate(p_gw_wsm);

    /*已配网，直接返回*/

    if ( (start_mode == WF_START_OTHER_CFG) || !(p_gw_wsm->nc_tp == GWNS_UNCFG_AP || p_gw_wsm->nc_tp == GWNS_UNCFG_SMC || p_gw_wsm->nc_tp == GWNS_UNCFG_SMC_AP ) )
    {
        PR_DEBUG("saved nc_tp:%d, start_mode:%d",p_gw_wsm->nc_tp,start_mode);
        return OPRT_OK;   
    }

#if defined(ENABLE_WIFI_EZ) && (ENABLE_WIFI_EZ == 1)
    /*判断ez first 或 ap first模式下，是否执行ap配网*/
    BOOL_T isApCfg = false;
    if ((p_gw_wsm->nc_tp == GWNS_UNCFG_AP) || (p_gw_wsm->md == GWM_SPECIAL_AP_CFG)) {
        isApCfg = true;
    }
#endif

#if defined(ENABLE_WIFI_NETCFG) && (ENABLE_WIFI_NETCFG == 1)

    TUYA_CALL_ERR_RETURN(netcfg_module_init());

    rt |= user_netcfg_init();

    /*执行配网模块初始化*/
    switch (start_mode) {
#if defined(ENABLE_TUYA_LAN) && (ENABLE_TUYA_LAN==1)
    case WF_START_AP_ONLY:
        rt |= ap_netcfg_init(start_mode);
        break;
#endif

#if defined(ENABLE_WIFI_EZ) && (ENABLE_WIFI_EZ == 1)
    case WF_START_SMART_ONLY:
        rt |= smart_netcfg_init(start_mode);
        break;

    case WF_START_SMART_FIRST:
    case WF_START_AP_FIRST:
        if (isApCfg) {
            rt |= ap_netcfg_init(start_mode);
        } else {
            rt |= smart_netcfg_init(start_mode);
        }
#endif

        break;
    case WF_START_SMART_AP_CONCURRENT:
#if defined(ENABLE_WIFI_EZ) && (ENABLE_WIFI_EZ == 1)
        rt |= smart_netcfg_init(start_mode);
#endif
#if defined(ENABLE_TUYA_LAN) && (ENABLE_TUYA_LAN==1)
        rt |= ap_netcfg_init(start_mode);
#endif
#if defined(ENABLE_WIFI_FFS) && (ENABLE_WIFI_FFS == 1)
        rt |= ffs_netcfg_init();
#endif
#if defined(ENABLE_WIFI_PEGASUS) && (ENABLE_WIFI_PEGASUS == 1)
        rt |= pegasus_netcfg_init();
#endif
        break;

    default:
        break;
    }
#endif    
    return rt;
}

STATIC VOID_T lowpower_short_timeout_handler(VOID_T)
{
    tuya_wifi_reset_timer_expired_proc(GWNS_LOWPOWER, GWM_NORMAL, FALSE);

    PR_AUTOTEST("short timer timeout, mthd:%d, nc_tp:%d ,md:%d", get_wifi_config_params()->mthd, get_gw_cntl()->gw_wsm.nc_tp, get_gw_cntl()->gw_wsm.md);
}
STATIC VOID_T lowpower_long_timeout_handler(VOID_T)
{
    tuya_wifi_netcfg_stop();
#if defined(ENABLE_MATTER) && (ENABLE_MATTER == 1) 
    /*try to close matter commission window*/
    tal_matter_commission_stop();
#endif
    tuya_wifi_netcfg_enter_lowpower();

    PR_AUTOTEST("long  timer timeout, mthd:%d, nc_tp:%d, md:%d", get_wifi_config_params()->mthd, get_gw_cntl()->gw_wsm.nc_tp, get_gw_cntl()->gw_wsm.md);

}

STATIC VOID_T lowpower_autocfg_long_timeout_handler(VOID_T)
{
    tuya_wifi_netcfg_stop();

#if defined(ENABLE_MATTER) && (ENABLE_MATTER == 1) 
    /*try to close matter commission window*/
    tal_matter_commission_stop();
#endif
    tuya_wifi_netcfg_enter_lowpower();
    tuya_wifi_reset_timer_expired_proc(GWNS_LOWPOWER, GWM_NORMAL, TRUE);

    PR_AUTOTEST("long  timer timeout, mthd:%d, nc_tp:%d, md:%d", get_wifi_config_params()->mthd, get_gw_cntl()->gw_wsm.nc_tp, get_gw_cntl()->gw_wsm.md);
}


STATIC VOID_T special_short_timeout_handler(VOID_T)
{
    GW_CNTL_S *gw_cntl = get_gw_cntl();
    GW_WF_MD_T md = get_gw_cntl()->gw_wsm.md;
    GW_WF_NWC_STAT_T new_nc_tp = GWNS_LOWPOWER;

    PR_NOTICE("gw_cntl->gw_wsm.stat:%d md:%d nc_tp:%d", gw_cntl->gw_wsm.stat, gw_cntl->gw_wsm.md, gw_cntl->gw_wsm.nc_tp);

    if (GWM_NORMAL < md) {
        new_nc_tp = tuya_wifi_reset_md_to_nc_tp(md);
    }
    
    tuya_wifi_reset_timer_expired_proc(new_nc_tp, md, FALSE);

    PR_AUTOTEST("short timer timeout, mthd:%d, nc_tp:%d, md:%d", get_wifi_config_params()->mthd, new_nc_tp, get_gw_cntl()->gw_wsm.md);
}

STATIC VOID_T special_long_timeout_handler(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    GW_WORK_STAT_MAG_S read_gw_wsm;
    GW_WF_NWC_STAT_T new_nc_tp = GWNS_LOWPOWER;
    GW_CNTL_S *gw_cntl = get_gw_cntl();

    PR_NOTICE("gw_cntl->gw_wsm.stat:%d md:%d nc_tp:%d", gw_cntl->gw_wsm.stat, gw_cntl->gw_wsm.md, gw_cntl->gw_wsm.nc_tp);
    TUYA_CALL_ERR_LOG(wd_gw_wsm_read(&read_gw_wsm));

    tuya_wifi_netcfg_stop();

    PR_DEBUG("active stat:%d", read_gw_wsm.stat);
    if (get_gw_cntl()->gw_wsm.md > GWM_NORMAL) {
        new_nc_tp = tuya_wifi_reset_md_to_nc_tp(get_gw_cntl()->gw_wsm.md);
        TUYA_CALL_ERR_LOG(tuya_wifi_reset_timer_expired_proc(new_nc_tp, get_gw_cntl()->gw_wsm.md, TRUE));

        TUYA_CALL_ERR_LOG(tuya_wifi_connect_start());

        /*send activated event*/
        TUYA_CALL_ERR_LOG(do_wifi_activate(NULL, 0));
    } else {
        tuya_wifi_netcfg_enter_lowpower();
        tuya_wifi_reset_timer_expired_proc(GWNS_LOWPOWER, get_gw_cntl()->gw_wsm.md, TRUE);
    }
    PR_AUTOTEST("long  timer timeout, mthd:%d, nc_tp:%d, md:%d", get_wifi_config_params()->mthd, new_nc_tp, get_gw_cntl()->gw_wsm.md);
}

#if defined(ENABLE_MATTER) && (ENABLE_MATTER == 1) 
void tyClearActiveToken(void)
{
    memset(get_gw_cntl()->gw_wsm.token, 0, sizeof(get_gw_cntl()->gw_wsm.token));
}
STATIC VOID_T special_matter_long_timeout_handler(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    GW_WORK_STAT_MAG_S read_gw_wsm;
    GW_WF_NWC_STAT_T new_nc_tp = GWNS_LOWPOWER;
    GW_CNTL_S *gw_cntl = get_gw_cntl();

    PR_NOTICE("gw_cntl->gw_wsm.stat:%d md:%d nc_tp:%d", gw_cntl->gw_wsm.stat, gw_cntl->gw_wsm.md, gw_cntl->gw_wsm.nc_tp);
    TUYA_CALL_ERR_LOG(wd_gw_wsm_read(&read_gw_wsm));

    tuya_wifi_netcfg_stop();
    /*try to close matter commission window*/
    tal_matter_commission_stop();
    PR_DEBUG("active stat:%d", read_gw_wsm.stat);
    if (get_gw_cntl()->gw_wsm.md > GWM_NORMAL && read_gw_wsm.stat >= REGISTERED){     
        new_nc_tp = tuya_wifi_reset_md_to_nc_tp(get_gw_cntl()->gw_wsm.md);
        TUYA_CALL_ERR_LOG(tuya_wifi_reset_timer_expired_proc(new_nc_tp, get_gw_cntl()->gw_wsm.md, TRUE));

        if (gw_cntl->cbs.gw_reset_cb) {
            gw_cntl->cbs.gw_reset_cb(GW_LOCAL_UNACTIVE);
        }
        tal_system_reset();

    } else {
        tuya_wifi_netcfg_enter_lowpower();
        tuya_wifi_reset_timer_expired_proc(GWNS_LOWPOWER, get_gw_cntl()->gw_wsm.md, TRUE);
    }
    PR_AUTOTEST("long  timer timeout, mthd:%d, nc_tp:%d, md:%d", get_wifi_config_params()->mthd, new_nc_tp, get_gw_cntl()->gw_wsm.md);
}
#endif
/**
 * @brief set a flag to indicate that iot_wf_gw_unactive_custom_mode is called
 *
 * @param[in] bcustom iot_wf_gw_unactive_custom_mode is called
 *
 * @note if bcustom is set, nc_tp will not updated by wifi start mode when device powered on
 *
 * @return void
 */

STATIC OPERATE_RET normal_reset_handler(IN CONST GW_WF_START_MODE wifi_start_mode, IN CONST BOOL_T force_clean, OUT GW_WF_NWC_STAT_T *p_nc_tp, GW_WF_MD_T* p_md)
{

    if (WF_START_SMART_ONLY == wifi_start_mode) {
        *p_nc_tp = GWNS_UNCFG_SMC;
    } else if (WF_START_AP_ONLY == wifi_start_mode) {
        *p_nc_tp = GWNS_UNCFG_AP;
    } else if (WF_START_SMART_AP_CONCURRENT == wifi_start_mode) {
        /*In old, old prod or low power mode,after reset, device will enter smart and ap concurrent mode */
        *p_nc_tp = GWNS_UNCFG_SMC_AP;
    } else if (WF_START_OTHER_CFG == wifi_start_mode){
        *p_nc_tp = GWNS_OTHER_UNCFG;
    } else { // WRT_AUTO
        if (GWNS_UNCFG_SMC == *p_nc_tp) {
            *p_nc_tp = GWNS_UNCFG_AP;
        } else {
            if ((wifi_start_mode == WF_START_AP_FIRST) && (*p_nc_tp >= GWNS_TY_SMARTCFG)) {
                *p_nc_tp = GWNS_UNCFG_AP;
            } else {
                *p_nc_tp = GWNS_UNCFG_SMC;
            }
        }
    }

    *p_md = GWM_NORMAL;

    PR_DEBUG("<__wf_reset> nc_tp:%d, mthd:%d, wifi_start_mode:%d", *p_nc_tp, get_wifi_config_params()->mthd,wifi_start_mode);
    tuya_wifi_reset_clean_ssid_passwd_token();
    return OPRT_OK;
}

STATIC OPERATE_RET special_reset_handler(IN CONST GW_WF_START_MODE wifi_start_mode, IN CONST BOOL_T force_clean, OUT GW_WF_NWC_STAT_T *p_nc_tp, GW_WF_MD_T* p_md)
{
    OPERATE_RET op_ret = OPRT_OK;

    GW_WORK_STAT_MAG_S read_gw_wsm;
    op_ret = wd_gw_wsm_read(&read_gw_wsm);
    if (OPRT_OK != op_ret) {
        PR_DEBUG("nc_tp in flash read failed");
        return op_ret;
    }

    if (WF_START_SMART_ONLY == wifi_start_mode) {
        *p_nc_tp = GWNS_UNCFG_SMC;
    } else if (WF_START_AP_ONLY == wifi_start_mode) {
        *p_nc_tp = GWNS_UNCFG_AP;
    } else if (WF_START_SMART_AP_CONCURRENT == wifi_start_mode) {
        /*In special clean mode, after reset, device will enter smart and ap concurrent mode */
        *p_nc_tp = GWNS_UNCFG_SMC_AP;
    }
    else if (WF_START_OTHER_CFG == wifi_start_mode)
    {
        *p_nc_tp = GWNS_OTHER_UNCFG;
    }
    else
    { // WRT_AUTO
        if (GWNS_LOWPOWER == *p_nc_tp) {
            if ((wifi_start_mode == WF_START_AP_FIRST) || (wifi_start_mode == WF_START_AP_ONLY)) {
                *p_nc_tp = GWNS_UNCFG_AP;
            } else {
                *p_nc_tp = GWNS_UNCFG_SMC;
            }
        } else if (GWNS_UNCFG_SMC == *p_nc_tp) {
            *p_nc_tp = GWNS_UNCFG_AP;
        } else {
            if ((wifi_start_mode == WF_START_AP_FIRST) && (*p_nc_tp >= GWNS_TY_SMARTCFG)) {
                *p_nc_tp = GWNS_UNCFG_AP;
            } else {
                *p_nc_tp = GWNS_UNCFG_SMC;
            }
        }
    }

    /*wifi未配置，或蓝牙激活态*/
    if ( TRUE == force_clean || *p_nc_tp == GWNS_PROXY_ACTIVED) { /*app触发的强制清除数据*/
        *p_md = GWM_NORMAL;
    }
    PR_DEBUG("<__wf_reset> nc_tp:%d, wifi_start_mode:%d,mthd:%d,md:%d", *p_nc_tp, wifi_start_mode, get_wifi_config_params()->mthd, *p_md);

    return OPRT_OK;

}

#define SHORT_TIMER_DEFAULT_TIMEEOUT    (10)

STATIC OPERATE_RET tuya_iot_matter_reset_init(CONST GW_WF_CFG_MTHD_SEL mthd)
{
    WIFI_RESET_PARAMS_T wf_rst_param = {
        .short_timer_val = SHORT_TIMER_DEFAULT_TIMEEOUT,
        .long_timer_val =  g_long_timer_timeout_val,
        .short_timer_handler = NULL,
        .long_timer_handler = NULL,
        .reset_handler = NULL,
    };

    switch (mthd)
    {
    case GWCM_OLD:
    case GWCM_OLD_PROD:
        wf_rst_param.reset_handler = normal_reset_handler;
        break;
    case GWCM_LOW_POWER:
        wf_rst_param.reset_handler = normal_reset_handler;
        wf_rst_param.short_timer_handler = lowpower_short_timeout_handler;
        wf_rst_param.long_timer_handler = lowpower_long_timeout_handler;
        break;
    case GWCM_LOW_POWER_AUTOCFG:
        wf_rst_param.long_timer_handler = lowpower_autocfg_long_timeout_handler;
        wf_rst_param.reset_handler = normal_reset_handler;
        break;
    case GWCM_SPCL_MODE:
        wf_rst_param.short_timer_handler = special_short_timeout_handler;
        wf_rst_param.long_timer_handler = special_long_timeout_handler;
        wf_rst_param.reset_handler = special_reset_handler;
        break;
#if defined(ENABLE_MATTER) && (ENABLE_MATTER == 1) 
    case GWCM_SPCL_AUTOCFG:
    case GWCM_SPCL_MATTER:
        wf_rst_param.short_timer_handler = special_short_timeout_handler;
        wf_rst_param.long_timer_handler = special_matter_long_timeout_handler;
        wf_rst_param.reset_handler = special_reset_handler;
        break;
#endif        
    default:
        break;
    }
    return tuya_wifi_reset_register_handler(wf_rst_param);    
}

#endif /* ENABLE_TY_MATTER */
