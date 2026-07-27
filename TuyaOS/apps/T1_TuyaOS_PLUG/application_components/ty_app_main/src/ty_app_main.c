/**
 * @file app_main.c
 * @author www.tuya.com
 * @brief app_main module is used to
 * @version 0.1
 * @date 2022-04-14
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */
#include "ty_sys.h"
#include "tuya_app_config.h"

#include "ty_app_starts_up_intf.h"
#include "ty_app_mf_test.h"
#include "ty_meta_report.h"

#include "tkl_thread.h"
#if defined(ENABLE_UART_AI) && (ENABLE_UART_AI==1)
#include "tuya_ai_func_config.h"
#include "gfw_mcu_ota.h"
#endif
#if (defined(ELEC_JSON_UPGRADE_EN) && (ELEC_JSON_UPGRADE_EN==1))
#include "ty_app_elec_json_upgrade.h"
#endif
/***********************************************************
*************************micro define***********************
***********************************************************/
#define EXAMPLES_ENABLE 0

#if defined(EXAMPLES_ENABLE) && (EXAMPLES_ENABLE)
#include "examples_main.h"
#endif

/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC TKL_THREAD_HANDLE ty_app_thread = NULL;

/***********************************************************
***********************function define**********************
***********************************************************/
#if defined(ENABLE_TY_INFORM_OTA_REQ) && (ENABLE_TY_INFORM_OTA_REQ)
TUYA_WEAK_ATTRIBUTE VOID_T ty_app_pre_upgrade_notice(IN CONST FW_UG_S *fw)
{
    TAL_PR_NOTICE("OTA pre upgrade notice, Implemented in applications");
    return;
}

/**
 * @brief OTA升级请求处理回调（支持多通道分发）
 *
 * @param[in] fw OTA请求信息结构体指针，包含升级通道号tp等信息
 * @return 0 固定返回
 *
 * @details
 * 该回调用于处理MCU OTA和设备本身OTA等多种升级请求。
 * 根据fw->tp通道号，分发到不同的升级处理函数：
 *  - 通道号100：调用app_json_upgrade_pre_cb（如JSON配置升级）
 *  - 通道号1：调用gfw_mcu_ota_pre_ug_inform_cb（如MCU OTA升级）
 *  - 其他通道：
 * 需确保相关功能宏已开启时才会调用对应处理函数。
 */
int ty_app_upgrade_pre_cb(IN CONST FW_UG_S *fw)
{
    TAL_PR_NOTICE("app upgrade pre cb");
    TAL_PR_NOTICE("fw->tp:%d", fw->tp);

    ty_app_pre_upgrade_notice(fw);

    switch (fw->tp) {
        case 100:
#if defined(ELEC_JSON_UPGRADE_EN) && (ELEC_JSON_UPGRADE_EN == 1)
            TAL_PR_NOTICE("Handle by app_json_upgrade_pre_cb");
            app_json_upgrade_pre_cb(fw);
#endif
        break;
        case 10:
#if defined(ENABLE_UART_AI) && (ENABLE_UART_AI==1)
            TAL_PR_NOTICE("Handle by gfw_mcu_ota_pre_ug_inform_cb");
            gfw_mcu_ota_pre_ug_inform_cb(fw);
#endif
        break;
        default:
            TAL_PR_NOTICE("Unknown channel, default handle");
        break;
    }
    return 0;
}
#endif

/**
* @brief ble-dp查询回调-使用时需要实现改接口内容
*
* @param[in] dp_qry
* @return 
*/
__attribute__((weak)) VOID_T ty_app_ble_dp_query_handle(VOID_T)
{
    TAL_PR_NOTICE("ble dp query cb");
}

STATIC VOID __devos_run_work(IN VOID *data)
{
    // report security version
    ty_cJSON *meta = ty_cJSON_CreateObject();
    ty_cJSON_AddStringToObject(meta, "secureVersion", "1.0.0");
    ty_meta_report(meta, REPORT_MODE_DEFAULT);
    ty_cJSON_Delete(meta);
    TAL_PR_NOTICE("Main Module: v1.0.0 (%s)", USER_SW_VER);
}

STATIC OPERATE_RET  __devos_run_evt(VOID *data)
{
    return tal_workq_schedule(WORKQ_SYSTEM, __devos_run_work, NULL);
}

/**
 * @brief  application entrance
 *
 * @return
 */
void app_main(void)
{
    OPERATE_RET rt = OPRT_OK;

    // base utilities init, such as log system, etc.
    tuya_base_utilities_init();

#if defined(ENABLE_FIRMWARE_DEBUG) && (ENABLE_FIRMWARE_DEBUG)
    tal_log_set_manage_attr(TAL_LOG_LEVEL_DEBUG);
#else 
    tal_log_set_manage_attr(TAL_LOG_LEVEL_NOTICE);
#endif

#if defined(ENABLE_LWIP) && (ENABLE_LWIP)
    TUYA_LwIP_Init();
#endif

    // 应用消息队列初始化
    TUYA_CALL_ERR_LOG(ty_app_msg_queue_init());

    // SDK 初始化前的预备工作，注册SDK事件等
    TUYA_CALL_ERR_LOG(ty_app_pre_sdk_init());

    tuyaos_wifi_sdk_init();

    // RED认证应用兼容处理
    ty_subscribe_event(EVENT_RUN, "dev.run", __devos_run_evt, SUBSCRIBE_TYPE_ONETIME);

    // 模组产测（mf test） 通知注册 
#if defined(ENABLE_MF_TEST_STATUS) && (ENABLE_MF_TEST_STATUS)
    TUYA_CALL_ERR_LOG(ty_sys_mf_status_cb_reg(ty_app_mf_test_status_cb));
#endif

    // 模组产测时获取应用配置参数(oem 参数) 回调注册
#if defined(ENABLE_GET_MF_TEST_USER_PARAM) && (ENABLE_GET_MF_TEST_USER_PARAM) 
    mf_user_param_cb_set(ty_app_mf_test_user_param_cb);
#endif

    // KV flash 数据存储模块初始化
    tuya_iot_kv_flash_init(NULL);

    // 中继功能注册
#if defined(ENABLE_TY_WIFI_BRIDGE) && (ENABLE_TY_WIFI_BRIDGE)
#if defined(ENABLE_WIFI_BRIDGE) && (ENABLE_WIFI_BRIDGE)
    OPERATE_RET wifi_bridge_init(VOID);
    wifi_bridge_init();
#else
#error "SDK does not support WiFi bridge. Please check!"
#endif
#endif


#if defined(ENABLE_UART_AI) && (ENABLE_UART_AI==1)
    tuya_ai_func_config_init();
#endif

    // 搭配涂鸦上位机进行成品产测
#if defined(ENABLE_PRODUCT_TEST_MF) && (ENABLE_PRODUCT_TEST_MF)
    TUYA_CALL_ERR_LOG(ty_sys_mf_product_test_reg(ty_app_product_test_mf_cb));
#endif

    // 成品产测扫描指定wifi
#if defined(ENABLE_PRODUCT_TEST_SCAN_WIFI) && (ENABLE_PRODUCT_TEST_SCAN_WIFI)
    TY_WIFI_TEST_SCAN_INFO_T wifi_scan_info;

    memset((TY_WIFI_TEST_SCAN_INFO_T *)&wifi_scan_info, 0x00, SIZEOF(TY_WIFI_TEST_SCAN_INFO_T));
    TUYA_CALL_ERR_LOG(ty_app_get_product_test_scan_wifi(&wifi_scan_info));

    TUYA_CALL_ERR_LOG(ty_sys_product_test_scan_wifi_reg(&wifi_scan_info));
#endif

#if defined(ENABLE_TY_FIRMWARE_OEM) && (ENABLE_TY_FIRMWARE_OEM)
    tuya_iot_oem_set(TRUE);
#endif

    //工厂测试
    TUYA_CALL_ERR_LOG(ty_sys_factory_test());

    //进入成品产测, 则不执行设备激活配网流程
    if(TRUE ==  ty_sys_is_enter_product_test()) {
        TAL_PR_NOTICE("factory product test");
        TUYA_CALL_ERR_LOG(ty_app_component_init());
        return;
    }

    //注册联网单品设备重要回调
    TY_WIFI_PROD_CB_S   cb_list = {0};
    TUYA_BLE_BT_PARA    ble_bt_para = {0};

    memset((UCHAR_T *)&cb_list, 0x00, SIZEOF(TY_WIFI_PROD_CB_S));
    memset((UCHAR_T *)&ble_bt_para, 0x00, SIZEOF(TUYA_BLE_BT_PARA));

    cb_list.wf_nw_stat_cb   = ty_app_wf_nw_stat_cb;

    cb_list.gw_status_cb    = ty_app_gw_status_changed_cb;
    cb_list.gw_reset_cb     = ty_app_gw_reset_cb;

    cb_list.dev_obj_dp_cb   = ty_app_obj_dp_handle; 
    cb_list.dev_raw_dp_cb   = ty_app_raw_dp_handle;
    cb_list.dev_dp_query_cb = ty_app_dp_query_handle;
#if defined(ENABLE_TY_INFORM_OTA_REQ) && (ENABLE_TY_INFORM_OTA_REQ)
    cb_list.pre_gw_mcu_ug_cb = ty_app_upgrade_pre_cb;
#endif
#if defined(ENABLE_TY_MCU_OTA) && (ENABLE_TY_MCU_OTA)
    cb_list.mcu_ug_cb = ty_app_mcu_upgrade_cb;
#elif defined(ENABLE_UART_AI) && (ENABLE_UART_AI==1)
    cb_list.mcu_ug_cb = gfw_mcu_ota_ug_inform_cb;
#endif

    ble_bt_para.query_dp_cb = ty_app_ble_dp_query_handle;

    TUYA_CALL_ERR_LOG(ty_sys_iot_dev_cbs_reg(&cb_list, &ble_bt_para));


#if defined(ENABLE_TY_PROD_INIT_NORMAL_PROCESS) && (ENABLE_TY_PROD_INIT_NORMAL_PROCESS)
    //product starts up 
#if defined(ENABLE_TY_SOC_DEV) && (ENABLE_TY_SOC_DEV)
    TY_WIFI_SOC_PROD_CFG_S  soc_prod_info;

    memset((TY_WIFI_SOC_PROD_CFG_S *)&soc_prod_info, 0x00, SIZEOF(TY_WIFI_SOC_PROD_CFG_S));
    TUYA_CALL_ERR_LOG(ty_app_get_soc_product_cfg(&soc_prod_info));

    TUYA_CALL_ERR_LOG(ty_sys_wifi_soc_init(&soc_prod_info));
#else 
    TY_WIFI_MCU_PROD_CFG_S mcu_prod_info;

    memset((TY_WIFI_MCU_PROD_CFG_S *)&mcu_prod_info, 0x00, SIZEOF(TY_WIFI_MCU_PROD_CFG_S));
    TUYA_CALL_ERR_LOG(ty_app_get_mcu_product_cfg(&mcu_prod_info));

    TUYA_CALL_ERR_LOG(ty_sys_wifi_mcu_init(&mcu_prod_info));
#endif
#endif

    TUYA_CALL_ERR_LOG(ty_app_component_init());

    return;
}

static void tuya_app_thread(void *arg)
{
#if !(defined(EXAMPLES_ENABLE) && (EXAMPLES_ENABLE))
    app_main();
#else
    examples_main();
#endif

    tkl_thread_release(ty_app_thread);
    ty_app_thread = NULL;

    return;
}

void tuya_app_main(void)
{
#if defined(ENABLE_EXT_RAM) && (ENABLE_EXT_RAM == 1)
    extern VOID_T tkl_system_psram_malloc_force_set(BOOL_T enable);
    tkl_system_psram_malloc_force_set(TRUE);
#endif
    tkl_thread_create(&ty_app_thread, "tuya_app_main", 4096, 4, tuya_app_thread, NULL);

    return;
}