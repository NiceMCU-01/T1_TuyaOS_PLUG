/**
 * @file ty_app_elec_trigger_factory.c
 * @author www.tuya.com
 * @brief ty_app_elec_trigger_factory module is used to 
 * @version 0.1
 * @date 2023-05-10
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "ty_sys.h"
#include "tuya_app_config.h"

#include "ty_app_elec_event_code.h"
#include "ty_app_elec_trigger.h"
#include "ty_app_elec_factory.h"

#include "app_elec_energy_monitor.h"

#include "app_elec_channel.h"
#include "app_elec_button.h"

#if defined(ENABLE_TY_LOAD_OEM_PAR) && (ENABLE_TY_LOAD_OEM_PAR)
#include "ty_app_elec_oem_config.h"
#endif

#if defined(ENABLE_MF_TEST_STATUS) && (ENABLE_MF_TEST_STATUS)
#include "ty_app_mf_test.h"
#endif

/***********************************************************
************************macro define************************
***********************************************************/
#define PRODUCT_TEST_WIFI_1                 "product_test_wifi_1"
#define PRODUCT_TEST_WIFI_2                 "product_test_wifi_2"

#define PROD_TEST_WEAK_SIGNAL               (-60)

/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
#if defined(ENABLE_PRODUCT_TEST_SCAN_WIFI) && (ENABLE_PRODUCT_TEST_SCAN_WIFI)
CONST CHAR_T *cWIFI_SCAN_SSID_LIST[] = {
    PRODUCT_TEST_WIFI_1,
    PRODUCT_TEST_WIFI_2,
};
#endif

/***********************************************************
***********************function define**********************
***********************************************************/
#if defined(ENABLE_MF_TEST_STATUS) && (ENABLE_MF_TEST_STATUS)
/**
 * @brief   模组产测状态通知回调
 *
 * @param[in] : status   产测各阶段的状态
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_mf_test_status_cb(TY_APP_MF_STATUS_E status)
{
    OPERATE_RET rt = OPRT_OK;

    switch (status) {
        case MF_STATUS_ENTER: {
            ty_app_event_post_synchronous(APP_EVT_GROUP_ELE, EVT_ELEC_PT_DATA_ERASE, NULL, 0);
        } 
        break;
        case MF_STATUS_WR_CFG_FINISH: 
        break;
        case MF_STATUS_GPIOTEST_START:
        break;
        default: 
        break;
    }

    return rt;
}
#endif

#if defined(ENABLE_GET_MF_TEST_USER_PARAM) && (ENABLE_GET_MF_TEST_USER_PARAM)
/**
 * @brief      模组产测时上位机下发的 oem 配置参数通知
 *
 * @param[in] :   data     oem 配置参数数据
 *                len      数据长度
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */ 
OPERATE_RET ty_app_mf_test_user_param_cb(IN CONST BYTE_T *data, IN CONST UINT_T len)
{
    OPERATE_RET rt = OPRT_OK;

#if (defined(ENABLE_TY_LOAD_OEM_PAR) && (ENABLE_TY_LOAD_OEM_PAR))
    //将 oem 配置存入 nvs 区
    ty_app_elec_oem_nvs_write_config(data, len);
#endif

    return rt;
}
#endif


prodtest_ssid_info_t *__ty_app_find_target_ssid(CHAR_T *target_ssid, prodtest_ssid_info_t *wifi_arr, UINT8_T arr_cnt)
{
    int rt = 0;
    UINT8_T i = 0 ;

    if(NULL == target_ssid || NULL == wifi_arr || 0 == arr_cnt) {
        return NULL;
    }

    for(i = 0;  i<arr_cnt; i++) {
        TAL_PR_DEBUG("ssid:%s, target ssid:%s", wifi_arr[i].ssid, target_ssid);
        rt = strcmp(wifi_arr[i].ssid, target_ssid);
        TAL_PR_DEBUG("rt: %d", rt);
        if(0 == strcmp(wifi_arr[i].ssid, target_ssid)) {
            return &wifi_arr[i];
        }
    }

    return NULL;
}

#if defined(ENABLE_PRODUCT_TEST_SCAN_WIFI) && (ENABLE_PRODUCT_TEST_SCAN_WIFI)

/**
 * @brief 产测模式下组件初始化
 *
 * @param[in] : 
 *
 * @return none
 */
OPERATE_RET __app_component_prod_test_init()
{
    OPERATE_RET rt = OPRT_OK;

    // 如果是计量产测，则先不开启按键
#if defined(PRODUCT_TEST_ENERGY_MONITOR) && (PRODUCT_TEST_ENERGY_MONITOR==1)
    if (1 == app_elec_energy_monitor_enable_get()) {
        return OPRT_OK;
    }
#endif

    // 非计量产测使能按键
    TUYA_CALL_ERR_LOG(app_elec_button_init(ty_app_elec_button_factory_trigger));

    return rt;
}

OPERATE_RET __app_scan_wifi_test_info_cb(INT_T flag, prodtest_ssid_info_t *ssid_info, UINT8_T info_count)
{
    OPERATE_RET rt = OPRT_OK;
    ELEC_PROD_TEST_STATUS_T prod_test_status = PROD_NOT_TESTED;

    prodtest_ssid_info_t *p_ssid_info = NULL;
    UINT32_T  event_id = 0;

    //find target ssid 
    prod_test_status = ty_app_elec_prod_test_status_get();
    TAL_PR_DEBUG("prod_test_status: %d", prod_test_status);
    if(PROD_TEST_FAILED == prod_test_status || PROD_NOT_TESTED == prod_test_status) {
        p_ssid_info = __ty_app_find_target_ssid(PRODUCT_TEST_WIFI_1, ssid_info, info_count);
        event_id    = EVT_ELEC_PROD_TEST_1;
        TAL_PR_DEBUG("EVT_ELEC_PROD_TEST_1");
    } else if (PROD_TEST_SUCCESS == prod_test_status) {
        p_ssid_info = __ty_app_find_target_ssid(PRODUCT_TEST_WIFI_2, ssid_info, info_count);
        event_id    = EVT_ELEC_PROD_TEST_2;
        TAL_PR_DEBUG("EVT_ELEC_PROD_TEST_2");
    } else {
        return OPRT_COM_ERROR;
    }

    //cant find target ssid 
    if(NULL == p_ssid_info) {
        TAL_PR_DEBUG("not find product test wifi");
        return OPRT_COM_ERROR;
    }

    app_elec_prod_test_status_set(PROD_TESTING);

    //post event 
    TAL_PR_NOTICE("product test wifi ssid: %s, rssi:%d", p_ssid_info->ssid, p_ssid_info->rssi);
    if(p_ssid_info->rssi < PROD_TEST_WEAK_SIGNAL) { //check signal
        ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_PROD_WEAK_SIGNAL, NULL, 0);
    }else if(0 == flag) {  //check authorization
        ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_PROD_UNAUTHOR, NULL, 0);
    } else {
        // 初始化产测下会使用到的组件
        TUYA_CALL_ERR_LOG(__app_component_prod_test_init());
        ty_app_event_post(APP_EVT_GROUP_ELE, event_id, NULL, 0); 
    }

    return OPRT_OK;
}

/**
 * @brief      获取成品产测时指定扫描的 Wi-Fi 配置信息
 *
 * @param[out] :    wifi_info    需要扫描的 Wi-Fi 配置信息
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_get_product_test_scan_wifi(OUT TY_WIFI_TEST_SCAN_INFO_T *wifi_info)
{
    if(NULL == wifi_info) {
        return OPRT_INVALID_PARM;
    }

#if defined(ENABLE_TY_LOAD_OEM_PAR) && (ENABLE_TY_LOAD_OEM_PAR)
    wifi_info->wf_cfg_mthd = ty_app_elec_oem_get_wf_cfg();
#else
    wifi_info->wf_cfg_mthd = TY_WIFI_CFG_MTHD;
#endif

    wifi_info->ssid_count   = CNTSOF(cWIFI_SCAN_SSID_LIST);
    wifi_info->ssid_list    = cWIFI_SCAN_SSID_LIST;
    wifi_info->scan_info_cb = __app_scan_wifi_test_info_cb;

    return OPRT_OK;
}

#endif
