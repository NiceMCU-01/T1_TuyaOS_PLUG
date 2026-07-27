/**
 * @file ty_app_starts_up_intf.h
 * @author www.tuya.com
 * @brief ty_app_starts_up_intf module is used to declare the interface of Application。这些接口均需要应用根据自身的业务模块实现。
 * @version 0.1
 * @date 2022-12-21
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#ifndef __TY_APP_STARTS_UP_INTF_H__
#define __TY_APP_STARTS_UP_INTF_H__

#include "ty_sys.h"
#include "tuya_app_config.h"
#include "ty_app_mf_test.h"

#ifdef __cplusplus
extern "C" {
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
/**
 * @brief SDK 初始化前的预初始化 注册快速启动等 SDK 事件
 *
 * @param none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_pre_sdk_init(VOID_T);

/**
 * @brief 产品业务功能组件初始化
 *
 * @param none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_component_init(VOID_T);

/**
 * @brief Wi-Fi 状态改变通知回调
 *
 * @param stat   Wi-Fi 状态
 *
 * @return none
 */
VOID_T ty_app_wf_nw_stat_cb(IN CONST GW_WIFI_NW_STAT_E stat);

/**
 * @brief 产品重置通知回调
 *
 * @param type   重置类型 
 *
 * @return none
 */
VOID_T ty_app_gw_reset_cb(GW_RESET_TYPE_E type);

/**
 * @brief 产品激活状态通知回调
 *
 * @param status  产品激活状态
 *
 * @return none
 */
VOID_T ty_app_gw_status_changed_cb(IN CONST GW_STATUS_E status);

/**
 * @brief obejct 类型 DP 指令控制回调
 *        obejct DP : Bool 布尔/String 字符串/Enum 枚举/Bitmap 故障
 *
 * @param dp  控制指令
 *
 * @return none
 */
VOID_T ty_app_obj_dp_handle(IN CONST TY_RECV_OBJ_DP_S *dp);

/**
 * @brief 透传类型(hex) DP 指令控制回调
 *
 * @param dp  控制指令
 *
 * @return none
 */
VOID_T ty_app_raw_dp_handle(IN CONST TY_RECV_RAW_DP_S *dp);

/**
 * @brief 查询 DP 当前状态回调
 *
 * @param dp  需要查询的 DP 
 *
 * @return none
 */
VOID_T ty_app_dp_query_handle(IN CONST TY_DP_QUERY_S *dp_qry);

/**
* @brief 蓝牙链路dp查询回调
*
* @param[in] none
*
* @return 
*/
VOID_T ty_app_ble_dp_query_handle(VOID_T);

#if defined(ENABLE_TY_PROD_INIT_NORMAL_PROCESS) && (ENABLE_TY_PROD_INIT_NORMAL_PROCESS)

#if defined(ENABLE_TY_SOC_DEV) && (ENABLE_TY_SOC_DEV)
/**
 * @brief    获取 SOC 产品配置参数
 *
 * @param[in] : prod    产品配置
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_get_soc_product_cfg(OUT TY_WIFI_SOC_PROD_CFG_S *prod);
#else 
/**
 * @brief    获取 MCU 产品配置参数
 *
 * @param[in] : prod    产品配置
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_get_mcu_product_cfg(OUT TY_WIFI_MCU_PROD_CFG_S *prod);
#endif
#endif

#if defined(ENABLE_MF_TEST_STATUS) && (ENABLE_MF_TEST_STATUS) 
/**
 * @brief   模组产测状态通知回调
 *
 * @param[in] : status   产测各阶段的状态
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_mf_test_status_cb(TY_APP_MF_STATUS_E status);
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
OPERATE_RET ty_app_mf_test_user_param_cb(IN CONST BYTE_T *data, IN CONST UINT_T len);
#endif

#if defined(ENABLE_PRODUCT_TEST_SCAN_WIFI) && (ENABLE_PRODUCT_TEST_SCAN_WIFI)
/**
 * @brief      获取成品产测时指定扫描的 Wi-Fi 配置信息
 *
 * @param[out] :    wifi_info    需要扫描的 Wi-Fi 配置信息
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_get_product_test_scan_wifi(OUT TY_WIFI_TEST_SCAN_INFO_T *wifi_info);
#endif 

#if defined(ENABLE_PRODUCT_TEST_MF) && (ENABLE_PRODUCT_TEST_MF)
/**
 * @brief       成品产测指令下发回调(需要涂鸦上位机参与)
 *
 * @param[in] :   cmd         成品产测命令字
 *                data        指令数据
 *                len         数据长度
 * @param[out] :  ret_data    产测结果返回
 *                ret_len     返回数据的长度
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_product_test_mf_cb(USHORT_T cmd, UCHAR_T *data, UINT_T len, OUT UCHAR_T **ret_data, OUT USHORT_T *ret_len);
#endif


#if defined(ENABLE_TY_INFORM_OTA_REQ) && (ENABLE_TY_INFORM_OTA_REQ)
/**
 * @brief 有 MCU OTA 和设备本身 OTA 请求时，通知应用的预处理回调函数
 *
 * @param[in] fw OTA请求信息
 * @details
 * - 该函数用于注册一个回调函数，当系统准备进行OTA升级时会调用该回调。
 * - 该回调函数可以用于在升级前执行一些预处理操作，如保存状态、通知用户等。
 * @return none
 */
VOID_T ty_app_pre_upgrade_notice(IN CONST FW_UG_S *fw);
#endif

#if defined(ENABLE_TY_MCU_OTA) && (ENABLE_TY_MCU_OTA)
/**
 * @brief   MCU OTA 开始时，通知应用的处理回调函数，
 *
 * @param[in] :    fw       MCU 固件升级信息
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */

  int ty_app_mcu_upgrade_cb(IN CONST FW_UG_S *fw);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __TY_APP_STARTS_UP_INTF_H__ */
