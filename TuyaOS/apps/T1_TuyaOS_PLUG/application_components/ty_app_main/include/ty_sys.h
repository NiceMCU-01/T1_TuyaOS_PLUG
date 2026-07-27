/**
 * @file ty_sys.h
 * @author www.tuya.com
 * @brief ty_sys module is used to
 * @version 0.1
 * @date 2022-04-15
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#ifndef __TY_SYS_H__
#define __TY_SYS_H__

#include <string.h>
#include <stdio.h>

#include "tuya_cloud_types.h"
#include "tuya_cloud_wifi_defs.h"
#include "tuya_cloud_com_defs.h"
#include "tuya_iot_com_api.h"
#include "tuya_iot_wifi_api.h"

#if defined(ENABLE_LWIP) && (ENABLE_LWIP==1)
#include "lwip_init.h"
#endif

#include "tal_system.h"
#include "tal_sw_timer.h"
#include "tal_thread.h"
#include "tal_mutex.h"
#include "tal_semaphore.h"
#include "tal_memory.h"
#include "tal_log.h"
#include "tal_sleep.h"
#include "tal_wifi.h"
#include "tal_workq_service.h"

#include "tuya_list.h"

#include "gw_intf.h"
#include "prod_test.h"
#include "ws_db_gw.h"

#if defined(ENABLE_TUYA_LAN) && (ENABLE_TUYA_LAN==1)
#include "tuya_svc_lan.h"
#endif

#if defined(ENABLE_BT_SERVICE) && (ENABLE_BT_SERVICE==1)
#include "tuya_bt.h"
#endif

#if defined(ENABLE_COMMUNICATE_PRIORITY) && (ENABLE_COMMUNICATE_PRIORITY)
#include "smart_frame.h"
#endif

#include "base_event.h"
#include "base_event_info.h"

#include "ty_event_loop.h"
#include "ty_app_msg.h"

#include "tuya_app_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
*************************micro define***********************
***********************************************************/
#ifdef LIGHT_PRRODUCT_DIMMER_PIXELS
    #define LIGHT_PRRODUCT_DIMMER 1
    #define LIGHT_PRRODUCT_PIXELS 1
#endif

#define EVENT_APP_MF_INIT_SUCC  "app_mf_init_succ" // mf exit and read enter product test

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef UCHAR_T APP_EVT_GROUP_E;
#define APP_EVT_GROUP_LIGHT                         1
#define APP_EVT_GROUP_ELE                           2
#define APP_EVT_GROUP_COMM_PROT                     3
#define APP_EVT_GROUP_SENSOR                        4
#define APP_EVT_GROUP_LOCK                          5
#define APP_EVT_GROUP_APPLIANCE                     6
#define APP_EVT_GROUP_REMOTE                        7
#define APP_EVT_GROUP_DOORBELL                      8

typedef enum {
    TY_DP_OBJ_RECV = 0,
    TY_DP_RAW_RECV,
    TY_DP_QUERY,
    HOMEKIT_DATA_RECV,
} RECV_DATA_HDNDLE_TYPE_E;

typedef struct {
    GW_WF_CFG_MTHD_SEL  wf_cfg_mthd;
    UINT8_T             ssid_count;
    CONST CHAR_T      **ssid_list;
    prodtest_app_cb_t   scan_info_cb;
    BOOL_T              is_enter_prod_test; 
    BOOL_T              is_ignore_mf_close;
} TY_WIFI_TEST_SCAN_INFO_T;
typedef struct {
    CHAR_T *name;
    CHAR_T *version;
} TY_FIRMWARE_INFO_T;

typedef struct {
    GW_WF_CFG_MTHD_SEL  wf_cfg;
    GW_WF_START_MODE    start_mode;
    UINT_T              wf_cfg_tm_s;  //unit: s
    CHAR_T             *firmware_key;
    CHAR_T             *product_id;
} TY_WIFI_SOC_PROD_CFG_S;

typedef struct {
    GW_WF_CFG_MTHD_SEL  wf_cfg;
    GW_WF_START_MODE    start_mode;
    UINT_T              wf_cfg_tm_s;
    CHAR_T             *firmware_key;
    CHAR_T             *product_id;
    UINT_T              attach_num;
    GW_ATTACH_ATTR_T   *attach_arr;
} TY_WIFI_MCU_PROD_CFG_S;

typedef struct {
    GET_WF_NW_STAT_CB wf_nw_stat_cb;
    /** status update */
    GW_STATUS_CHANGED_CB gw_status_cb;
    /** gateway reset */
    GW_RESET_IFM_CB gw_reset_cb;

    /** structured DP info */
    DEV_OBJ_DP_CMD_CB dev_obj_dp_cb;
    /** raw DP info */
    DEV_RAW_DP_CMD_CB dev_raw_dp_cb;
    /** DP query */
    DEV_DP_QUERY_CB dev_dp_query_cb;

    /** main netlink module and mcu upgrade pre-condition */
    GW_UG_INFORM_CB pre_gw_mcu_ug_cb;
    /** mcu upgrade */
    GW_UG_INFORM_CB mcu_ug_cb;
} TY_WIFI_PROD_CB_S;

/***********************************************************
***********************variable define**********************
***********************************************************/

/***********************************************************
***********************function define**********************
***********************************************************/

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
                                    IN CONST UINT_T len, IN CONST UINT_T timeout);

/**
 * @brief tuya wifi sdk 初始化
 *
 * @param none
 * @return
 */
VOID tuyaos_wifi_sdk_init(VOID);

/**
 * @brief   忽略关闭产测入口的标志 
 *       （建议调试时使用，正式固件不建议使用）
 *
 * @param  none
 *
 * @return none
 */
VOID_T ty_sys_ignore_close_factory_test(VOID_T);

/**
 * @brief      扫描指定wifi的成品产测回调注册
 *
 * @param[in] : wifi_scan_info   指定的扫描信息以及结果回调函数
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_sys_product_test_scan_wifi_reg(TY_WIFI_TEST_SCAN_INFO_T *wifi_scan_info);

/**
 * @brief      工厂测试 (模组产测/成品产测)
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_sys_factory_test(VOID_T);

/**
 * @brief    是否扫描到产测 ssid 进入成品产测
 *
 * @param  none
 *
 * @return  TRUE:进入成品产测  FALSE: 没有进入成品产测
 */
BOOL_T ty_sys_is_enter_product_test(VOID_T);

/**
 * @brief     注册联网单品设备必要的回调函数
 *
 * @param[in] : cbs wifi相关回调列表 (网络状态，dp控制等) 
 * @param[in] : ble_bt_para ble相关回调参数
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_sys_iot_dev_cbs_reg(TY_WIFI_PROD_CB_S *cbs, TUYA_BLE_BT_PARA *ble_bt_para);

/**
 * @brief     SOC 产品初始化
 *
 * @param[in] : prod_cfg SOC 产品信息 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_sys_wifi_soc_init(TY_WIFI_SOC_PROD_CFG_S *prod_cfg);

/**
 * @brief     MCU 产品初始化
 *
 * @param[in] : prod_cfg  MCU 产品信息 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_sys_wifi_mcu_init(TY_WIFI_MCU_PROD_CFG_S *prod_cfg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__TY_SYS_H__*/
