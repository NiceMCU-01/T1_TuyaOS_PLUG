/**
 * @file ty_sys.c
 * @author www.tuya.com
 * @brief ty_sys module is used to
 * @version 0.1
 * @date 2022-04-15
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */
#include "ty_sys.h"
#include "tuya_app_config.h"
#include "ty_app_mf_test.h"
#include "prod_test.h"
#include "tuya_bt.h"

#if defined(ENABLE_TY_MATTER) && (ENABLE_TY_MATTER == 1) 
#include "tuya_iot_matter_wifi_api.h"
#endif

/***********************************************************
*************************micro define***********************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    CHAR_T dev_id[DEV_ID_LEN + 1];
    BYTE_T dpid;
    UINT_T timeout;
    UINT_T data_len;
    BYTE_T data[0];
} DEV_RAW_DP_REPORT_ASYNC_DATA_T;

typedef struct {
    TY_WIFI_TEST_SCAN_INFO_T scan_info;
} TY_WIFI_TEST_CFG_S;

/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC TY_WIFI_TEST_SCAN_INFO_T sg_sys_wifi_test = {0};
STATIC TY_IOT_CBS_S             sg_sys_iot_cbs = {0};
STATIC TUYA_BLE_BT_PARA         sg_sys_ble_bt_para = {0};
STATIC GET_WF_NW_STAT_CB        sg_sys_nw_stat_cb = NULL; 
STATIC BOOL_T                   sg_is_dev_cbs_reg = FALSE;
/***********************************************************
***********************function define**********************
***********************************************************/
#if defined(ENABLE_PRODUCT_TEST_SCAN_WIFI) && (ENABLE_PRODUCT_TEST_SCAN_WIFI)

#if !defined(ENABLE_PRODUCT_TEST_MF)||\
    !defined(ENABLE_PRODUCT_AUTOTEST)
// scan wifi test
STATIC BOOL_T __is_allowed_scan_wifi_test(GW_WF_CFG_MTHD_SEL gwcm_mode)
{
    BOOL_T mf_close = FALSE;
    GW_WORK_STAT_MAG_S read_gw_wsm = {0};

    if(FALSE == sg_sys_wifi_test.is_ignore_mf_close) {
        mf_close = mf_test_is_timeout();
        if (TRUE == mf_close) {
            TAL_PR_NOTICE("have actived over 15min, don't scan prod test ssid");
            return FALSE;
        }  
    }
	
	if(GWCM_OLD == gwcm_mode) {          
        return FALSE;
    }

#if (defined(ENABLE_TY_MATTER) && (ENABLE_TY_MATTER == 1))
    wd_gw_wsm_read(&read_gw_wsm);
    TAL_PR_NOTICE("gwcm_mode:[%d], nc_tp:[%d].", gwcm_mode, read_gw_wsm.nc_tp);
    if ((gwcm_mode == GWCM_SPCL_MATTER) || (gwcm_mode == GWCM_LOW_POWER_AUTOCFG) ||
        (gwcm_mode == GWCM_SPCL_AUTOCFG)) { /* 上电默认配网或者第一次是配网的模式 */
        if (((read_gw_wsm.nc_tp >= GWNS_TY_SMARTCFG) && (read_gw_wsm.nc_tp != GWNS_UNCFG_SMC_AP) && (read_gw_wsm.nc_tp != GWNS_OTHER_UNCFG)) || read_gw_wsm.md > GWM_NORMAL) { /* 已经存在ssid等配网信息但是并不是EZ和AP共存配网 */
            return FALSE;
        }
    } else if (gwcm_mode == GWCM_SPCL_MODE || gwcm_mode == GWCM_LOW_POWER || gwcm_mode == GWCM_SPCL_MATTER) { /* 上电默认不配网 */
        if (read_gw_wsm.nc_tp >= GWNS_UNCFG_SMC) {                           /* 处于配网的状态 */
            return FALSE;
        }
    } else {
        ;
    }
#else
    wd_gw_wsm_read(&read_gw_wsm);
    if ((gwcm_mode == GWCM_OLD_PROD) || (gwcm_mode == GWCM_LOW_POWER_AUTOCFG) ||
        (gwcm_mode == GWCM_SPCL_AUTOCFG)) { /* 上电默认配网或者第一次是配网的模式 */
        if (((read_gw_wsm.nc_tp >= GWNS_TY_SMARTCFG) && (read_gw_wsm.nc_tp != GWNS_UNCFG_SMC_AP)) || read_gw_wsm.md > GWM_NORMAL) { /* 已经存在ssid等配网信息但是并不是EZ和AP共存配网 */
            return FALSE;
        }
    } else if (gwcm_mode == GWCM_SPCL_MODE || gwcm_mode == GWCM_LOW_POWER) { /* 上电默认不配网 */
        if (read_gw_wsm.nc_tp >= GWNS_UNCFG_SMC) {                           /* 处于配网的状态 */
            return FALSE;
        }
    } else {
        ;
    }
#endif

    return TRUE;
}

STATIC BOOL_T __is_dev_authorized(VOID_T)
{
    OPERATE_RET  ret = OPRT_OK;
    GW_BASE_IF_S *p_gw_base = NULL;
    BOOL_T is_authorized = TRUE;

    p_gw_base = (GW_BASE_IF_S *)tal_malloc(SIZEOF(GW_BASE_IF_S));
    if(NULL == p_gw_base) {
        TAL_PR_ERR("malloc failed");
        return FALSE;
    }
    memset((UCHAR_T *)p_gw_base, 0x00,  SIZEOF(GW_BASE_IF_S));

    ret = wd_gw_base_if_read(p_gw_base);
    if (OPRT_OK != ret) {
        TAL_PR_ERR("read flash err");
        tal_free(p_gw_base), p_gw_base = NULL;
        return FALSE;
    }

    // gateway base info verify
    if (0 ==  p_gw_base->auth_key[0] || 0 == p_gw_base->uuid[0]) {
        TAL_PR_NOTICE("please write uuid and auth_key first");
        is_authorized = FALSE;
    }else {
        is_authorized = TRUE;
    }

    tal_free(p_gw_base), p_gw_base = NULL;

    return is_authorized;
}

STATIC UINT_T __get_target_ap_from_all_ap(TY_WIFI_TEST_SCAN_INFO_T *scan_info, AP_IF_S *all_ap, UINT_T all_ap_num,\
                                          OUT prodtest_ssid_info_t **target_ap)
{
    prodtest_ssid_info_t *ap_tmp_buf = NULL;
    UINT_T i = 0, j =0 , target_ap_num = 0;
    BOOL_T  add_target_ap = FALSE;

    if(NULL == target_ap) {
        return 0;
    }

    //prepare ap info buf
    ap_tmp_buf = (prodtest_ssid_info_t *)tal_malloc(SIZEOF(prodtest_ssid_info_t) * scan_info->ssid_count);
    if (NULL == ap_tmp_buf) {
        return 0;
    }
    memset(ap_tmp_buf, 0, SIZEOF(prodtest_ssid_info_t) * scan_info->ssid_count);

    for (i = 0; i<scan_info->ssid_count; i++) {
        for (j = 0; j < all_ap_num; j++) {
            if (0 != strcmp((char *)scan_info->ssid_list[i], (char *)all_ap[j].ssid)) {  
                continue;
            }  

            //当前扫描到的ssid与扫描列表一致
            if (0 == strcmp((char *)ap_tmp_buf[target_ap_num].ssid, (char *)all_ap[j].ssid)) {
                //当前扫描到的ssid已存储,判断rssi是否需要更新
                if (all_ap[j].rssi > ap_tmp_buf[target_ap_num].rssi) {
                    TAL_PR_DEBUG("[update rssi] new:%d old:%d", all_ap[j].rssi, ap_tmp_buf[target_ap_num].rssi);
                    ap_tmp_buf[target_ap_num].rssi = all_ap[j].rssi;
                }
            }else {
                // 存储目标 ssid 信息
                add_target_ap = TRUE;
                strcpy((char *)ap_tmp_buf[target_ap_num].ssid, (char *)all_ap[j].ssid);
                ap_tmp_buf[target_ap_num].rssi = all_ap[j].rssi;
                TAL_PR_DEBUG("new index:%d ssid%s", target_ap_num, ap_tmp_buf[target_ap_num].ssid);
            }
        }

        if(TRUE == add_target_ap) {
            add_target_ap = FALSE;
            target_ap_num++;
        }
    }

    if(0 == target_ap_num) {
        tal_free(ap_tmp_buf), ap_tmp_buf = NULL;
        *target_ap = NULL;
    }else {
        *target_ap = ap_tmp_buf;
    }

    return target_ap_num;
}

STATIC VOID_T __release_target_ap(prodtest_ssid_info_t *target_ap)
{
    if(target_ap) {
        tal_free(target_ap);
    }

    return;
}

STATIC BOOL_T __scan_wifi_test_ssid(TY_WIFI_TEST_SCAN_INFO_T *scan_info)
{
    OPERATE_RET ret = OPRT_OK;
    prodtest_ssid_info_t *target_ap = NULL;
    AP_IF_S *ap = NULL;
    UINT_T ap_num = 0, target_ap_num = 0;
    BOOL_T flag = TRUE;

    if (NULL == scan_info || scan_info->ssid_count == 0 || \
        scan_info->ssid_list == NULL || scan_info->scan_info_cb == NULL) {
        return FALSE;
    }

    //scan all ap
    tal_wifi_set_work_mode(WWM_STATION);
    ret = tal_wifi_all_ap_scan(&ap, &ap_num);
    tal_wifi_station_disconnect();
    if (OPRT_OK != ret || 0 == ap_num) {
        TAL_PR_NOTICE("tal_wifi_all_ap_scan failed(%d) ap_num(%d)", ret, ap_num);
        return FALSE;
    }

    //check dev authorized
    flag = __is_dev_authorized();

    //compare ssid information
    target_ap_num = __get_target_ap_from_all_ap(scan_info, ap, ap_num, &target_ap);
    tal_wifi_release_ap(ap);
    ap = NULL, ap_num = 0;
    if(0 == target_ap_num) {
        TAL_PR_DEBUG("cant find target from scan result");
        return FALSE;
    }

    if (scan_info->scan_info_cb) {
        TAL_PR_DEBUG("gw cfg flash info reset factory!");
        GW_WORK_STAT_MAG_S *wsm = (GW_WORK_STAT_MAG_S *)tal_malloc(SIZEOF(GW_WORK_STAT_MAG_S));
        if (NULL != wsm) {
            memset(wsm, 0, SIZEOF(GW_WORK_STAT_MAG_S));
            ret = wd_gw_wsm_write(wsm);
            if (OPRT_OK != ret) {
                TAL_PR_ERR("wd_gw_wsm_write err:%d!", ret);
            }
            tal_free(wsm);
        }

        ret = scan_info->scan_info_cb(flag, target_ap, target_ap_num);
    }

    __release_target_ap(target_ap);
    target_ap = NULL, target_ap_num = 0;

    return ((OPRT_OK == ret) ? TRUE : FALSE);
}
#endif

#endif

/**
 * @brief tuya wifi sdk 初始化
 *
 * @param[in]
 * @return
 */
VOID tuyaos_wifi_sdk_init(VOID)
{
    OPERATE_RET rt = OPRT_OK;
    TY_INIT_PARAMS_S init_param = {0};

    //涂鸦系统服务初始化
    init_param.init_db = FALSE;
    strcpy(init_param.sys_env, TARGET_PLATFORM);
    TUYA_CALL_ERR_LOG(tuya_iot_init_params(NULL, &init_param));

    TAL_PR_NOTICE("sdk_info:%s", tuya_iot_get_sdk_info()); /* print SDK information */
    TAL_PR_NOTICE("name:%s:%s", APP_BIN_NAME, USER_SW_VER);                                    /* print the firmware name and version */
    TAL_PR_NOTICE("firmware compiled at %s %s", __DATE__, __TIME__);              /* print firmware compilation time */
    TAL_PR_NOTICE("system reset reason:[%d]", tal_system_get_reset_reason(NULL)); /* print system reboot causes */

    return ;
}

/**
 * @brief   忽略关闭产测入口的标志 
 *       （建议调试时使用，正式固件不建议使用）
 *
 * @param  none
 *
 * @return none
 */
VOID_T ty_sys_ignore_close_factory_test(VOID_T)
{
    mf_test_ignore_close_flag();
    sg_sys_wifi_test.is_ignore_mf_close = TRUE;

    return;
}

/**
 * @brief      扫描指定wifi的成品产测回调注册
 *
 * @param[in] : wifi_scan_info   指定的扫描信息以及结果回调函数
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_sys_product_test_scan_wifi_reg(TY_WIFI_TEST_SCAN_INFO_T *wifi_scan_info)
{
    if(NULL == wifi_scan_info) {
        return OPRT_INVALID_PARM;
    }

    if (0 == wifi_scan_info->ssid_count || NULL == wifi_scan_info->ssid_list\
        || NULL == wifi_scan_info->scan_info_cb) {
        return OPRT_INVALID_PARM;
    }

    sg_sys_wifi_test.wf_cfg_mthd  = wifi_scan_info->wf_cfg_mthd;
    sg_sys_wifi_test.ssid_count   = wifi_scan_info->ssid_count;
    sg_sys_wifi_test.ssid_list    = wifi_scan_info->ssid_list;
    sg_sys_wifi_test.scan_info_cb = wifi_scan_info->scan_info_cb;

    sg_sys_wifi_test.is_enter_prod_test = FALSE;

    return OPRT_OK;
}

/**
 * @brief      工厂测试 (模组产测/成品产测)
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_sys_factory_test(VOID_T)
{
    MF_IMPORT_INTF_S intf;
    OPERATE_RET rt = OPRT_OK;

    // mf产测初始化
    memset((UCHAR_T *)&intf, 0x00, SIZEOF(MF_IMPORT_INTF_S));
    ty_sys_mf_test_get_intf(&intf);
    TUYA_CALL_ERR_RETURN(mf_init(&intf, APP_BIN_NAME, USER_SW_VER, TRUE));
    TAL_PR_NOTICE("mf_init succ");

    ty_publish_event(EVENT_APP_MF_INIT_SUCC, NULL);
    // 成品产测
#if defined(ENABLE_PRODUCT_TEST_SCAN_WIFI) && (ENABLE_PRODUCT_TEST_SCAN_WIFI)

#if defined(ENABLE_PRODUCT_TEST_MF) && (ENABLE_PRODUCT_TEST_MF) &&\
    defined(ENABLE_PRODUCT_AUTOTEST) && (ENABLE_PRODUCT_AUTOTEST == 1)

    prodtest_app_cfg_t prodtest_cfg = {.gwcm_mode  = sg_sys_wifi_test.wf_cfg_mthd,
                                        .file_name  = APP_BIN_NAME,
                                        .file_ver   = USER_SW_VER,
                                        .ssid_list  = sg_sys_wifi_test.ssid_list,
                                        .ssid_count = sg_sys_wifi_test.ssid_count,
                                        .app_cb     = sg_sys_wifi_test.scan_info_cb,
                                        .product_cb = ty_sys_user_product_test_cb};

    prodtest_app_register(&prodtest_cfg);

    if (TRUE == prodtest_ssid_scan(500)) {
        //所有产测路由都要在6信道
        sg_sys_wifi_test.is_enter_prod_test = TRUE;
        return OPRT_OK;
    }
#else 

    TAL_PR_DEBUG("gwcm_mode %d", sg_sys_wifi_test.wf_cfg_mthd); 
    if(TRUE == __is_allowed_scan_wifi_test(sg_sys_wifi_test.wf_cfg_mthd)) {
        //扫描产测路由，如果扫到则进入产测模式，不需要连接路由
        if (TRUE == __scan_wifi_test_ssid(&sg_sys_wifi_test)) {
            sg_sys_wifi_test.is_enter_prod_test = TRUE;
            return OPRT_OK;
        }
    }   
#endif     
#endif

    return OPRT_OK;
}

/**
 * @brief    是否扫描到产测 ssid 进入成品产测
 *
 * @param  none
 *
 * @return  TRUE:进入成品产测  FALSE: 没有进入成品产测
 */
BOOL_T ty_sys_is_enter_product_test(VOID_T)
{
    return sg_sys_wifi_test.is_enter_prod_test;
}

/**
 * @brief     注册联网单品设备必要的回调函数
 *
 * @param[in] : cbs wifi相关回调列表 (网络状态，dp控制等) 
 * @param[in] : ble_bt_para ble相关回调参数
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_sys_iot_dev_cbs_reg(TY_WIFI_PROD_CB_S *cbs, TUYA_BLE_BT_PARA *ble_bt_para)
{
    if(NULL == cbs) {
        return OPRT_INVALID_PARM;
    }

    memset((UCHAR_T *)&sg_sys_iot_cbs, 0x00, SIZEOF(TY_IOT_CBS_S));
    memset((UCHAR_T *)&sg_sys_ble_bt_para, 0x00, SIZEOF(TUYA_BLE_BT_PARA));

    sg_sys_iot_cbs.gw_status_cb = cbs->gw_status_cb;
    sg_sys_iot_cbs.gw_reset_cb  = cbs->gw_reset_cb;

    sg_sys_iot_cbs.dev_dp_query_cb = cbs->dev_dp_query_cb;
    sg_sys_iot_cbs.dev_obj_dp_cb   = cbs->dev_obj_dp_cb;
    sg_sys_iot_cbs.dev_raw_dp_cb   = cbs->dev_raw_dp_cb;

    // wifi state
    sg_sys_nw_stat_cb = cbs->wf_nw_stat_cb;

    //ota
    sg_sys_iot_cbs.pre_gw_ug_cb = cbs->pre_gw_mcu_ug_cb;
    sg_sys_iot_cbs.gw_ug_cb     = cbs->mcu_ug_cb;

    //ble
    sg_sys_ble_bt_para.query_dp_cb = ble_bt_para->query_dp_cb;

    sg_is_dev_cbs_reg = TRUE;

    return OPRT_OK;
}



/**
 * @brief     SOC 产品初始化
 *
 * @param[in] : prod_cfg SOC 产品信息 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_sys_wifi_soc_init(TY_WIFI_SOC_PROD_CFG_S *prod_cfg)
{
    OPERATE_RET rt = OPRT_OK;

    if (NULL == prod_cfg){
        return OPRT_INVALID_PARM;
    }
    
    if (FALSE == sg_is_dev_cbs_reg) {
        TAL_PR_ERR("please regester device iot cbs first!");
        return OPRT_COM_ERROR;
    }

    if(prod_cfg->wf_cfg_tm_s) {
        tuya_iot_wf_timeout_set(prod_cfg->wf_cfg_tm_s); 
    }

#if defined(ENABLE_TY_MATTER) && (ENABLE_TY_MATTER == 1) 
    rt = tuya_iot_matter_wf_soc_dev_init_param(prod_cfg->wf_cfg, prod_cfg->start_mode, &sg_sys_iot_cbs, prod_cfg->firmware_key,
                                        prod_cfg->product_id, USER_SW_VER);
#else
    // 初始化TuyaOS产品信息
    rt = tuya_iot_wf_soc_dev_init_param(prod_cfg->wf_cfg, prod_cfg->start_mode, &sg_sys_iot_cbs, prod_cfg->firmware_key,
                                        prod_cfg->product_id, USER_SW_VER);
    if (OPRT_OK != rt) {
        TAL_PR_ERR("tuya_iot_wf_soc_dev_init_param err:%d", rt);
    }
#endif

    tuya_ble_set_bt_para(&sg_sys_ble_bt_para);

    rt = tuya_iot_reg_get_wf_nw_stat_cb_params(sg_sys_nw_stat_cb, 1);
    if (OPRT_OK != rt) {
        TAL_PR_ERR("tuya_iot_reg_get_wf_nw_stat_cb err:%d", rt);
        return rt;
    }

    return OPRT_OK;
}

/**
 * @brief     MCU 产品初始化
 *
 * @param[in] : prod_cfg  MCU 产品信息 
 * @param[in] : cb_list  相关回调列表 (网络状态，dp控制等) 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_sys_wifi_mcu_init(TY_WIFI_MCU_PROD_CFG_S *prod_cfg)
{
    OPERATE_RET rt = OPRT_OK;

    if (NULL == prod_cfg){
        return OPRT_INVALID_PARM;
    }
    
    if (FALSE == sg_is_dev_cbs_reg) {
        TAL_PR_ERR("please regester device iot cbs first!");
        return OPRT_COM_ERROR;
    }

    if(prod_cfg->wf_cfg_tm_s) {
        tuya_iot_wf_timeout_set(prod_cfg->wf_cfg_tm_s); 
    }

    // 初始化TuyaOS产品信息
    rt = tuya_iot_wf_dev_init(prod_cfg->wf_cfg, prod_cfg->start_mode, &sg_sys_iot_cbs, prod_cfg->firmware_key,
                              prod_cfg->product_id,USER_SW_VER,DEV_NM_ATH_SNGL, prod_cfg->attach_arr, prod_cfg->attach_num);

    tuya_ble_set_bt_para(&sg_sys_ble_bt_para);
    
    if (OPRT_OK != rt) {
        TAL_PR_ERR("tuya_iot_wf_dev_init err:%d", rt);
    }

    rt = tuya_iot_reg_get_wf_nw_stat_cb_params(sg_sys_nw_stat_cb, 1);
    if (OPRT_OK != rt) {
        TAL_PR_ERR("tuya_iot_reg_get_wf_nw_stat_cb err:%d", rt);
        return rt;
    }

    return OPRT_OK;
}

STATIC VOID_T dev_report_dp_raw_async_cb(VOID_T *data)
{
    OPERATE_RET ret = OPRT_OK;

    if (NULL != data) {
        DEV_RAW_DP_REPORT_ASYNC_DATA_T *raw_data = (DEV_RAW_DP_REPORT_ASYNC_DATA_T *)data;
        CHAR_T *dev_id = NULL;

        if (strlen(raw_data->dev_id))
            dev_id = raw_data->dev_id;

        ret = dev_report_dp_raw_sync(dev_id, raw_data->dpid, raw_data->data, raw_data->data_len, raw_data->timeout);
        if (OPRT_OK != ret)
            TAL_PR_ERR("dev_report_dp_raw_sync error:%d", ret);

        tal_free(data);
    }
}

/**
 * @brief raw格式dp数据异步上报
 *
 * @param[in] dev_id: if sub-device, then devid = sub-device_id
 *                if gateway/soc/mcu, then devid = NULL
 * @param[in] dpid: raw dp id
 * @param[in] data: raw data
 * @param[in] len: len of raw data
 * @param[in] timeout: function blocks until timeout seconds
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET dev_report_dp_raw_async(IN CONST CHAR_T *dev_id, IN CONST BYTE_T dpid, IN CONST BYTE_T *data,
                                    IN CONST UINT_T len, IN CONST UINT_T timeout)
{
    if (NULL == data || len == 0)
        return OPRT_INVALID_PARM;

    DEV_RAW_DP_REPORT_ASYNC_DATA_T *raw_data = tal_malloc(SIZEOF(DEV_RAW_DP_REPORT_ASYNC_DATA_T) + len);
    if (NULL == raw_data)
        return OPRT_MALLOC_FAILED;

    memset(raw_data, 0, SIZEOF(DEV_RAW_DP_REPORT_ASYNC_DATA_T) + len);

    if (NULL != dev_id)
        strcpy(raw_data->dev_id, dev_id);

    raw_data->dpid = dpid;
    raw_data->timeout = timeout;
    raw_data->data_len = len;
    memcpy(raw_data->data, data, len);

    tal_workq_schedule(WORKQ_SYSTEM, dev_report_dp_raw_async_cb, raw_data);

    return OPRT_OK;
}