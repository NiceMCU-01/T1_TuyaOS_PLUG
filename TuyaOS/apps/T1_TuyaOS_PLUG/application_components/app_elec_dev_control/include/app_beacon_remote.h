/**
 * @file app_beacon_remote.h
 * @author panda (panzd@tuya.com)
 * @brief beacon remoter
 * @version 1.0
 * @date 2021-09-20
 *
 * @copyright Copyright © HANGZHOU 2020 Tuya Inc. All rights reserved.
 *
 */

#ifndef __APP_BEACON_REMOTE_H__
#define __APP_BEACON_REMOTE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_cloud_wifi_defs.h"

// 遥控命令
typedef enum {
    SW_CMD_CLOSE = 0,   // 关闭
    SW_CMD_OPEN = 1,    // 开启
    SW_CMD_STOP = 2,    // 停止，普通开关、插座不支持
    SW_CMD_TRIGGER = 3, // 翻转，暂不支持！
} BLE_SW_CMD_E;

/**
 * @brief 蓝牙遥控器状态
 *
 */
typedef enum {
    UNCON_BLE = 0, // 解绑成功
    UNCON_BLE_ERR, // 解绑失败
    CON_BLE,       // 连接状态
    CON_BLE_ERR,   // 连接错误
} BLE_CON_STATE;

// 蓝牙遥控器状态回调
typedef VOID (*__ble_ctrl_status_cb)(IN BLE_CON_STATE state);

// 设备控制回调
typedef VOID (*__ble_ctrl_cb)(IN UCHAR_T ctrl_ch, IN BLE_SW_CMD_E code);

// 蓝牙回调配置结构
typedef struct {
    __ble_ctrl_status_cb status_cb;
    __ble_ctrl_cb ctrl_cb;
} ble_ctrl_reg_cfg;


/**
 * @function: app_elec_beacon_remote_enable
 * @description: app_elec_beacon_remote_enable
 * @param[in]: en 
 * @param[out]: none
 * @retval: none
 */
VOID app_elec_beacon_remote_enable(BOOL_T en);

/**
 * @function: app_elec_beacon_remote_is_enable
 * @description: app_elec_beacon_remote_is_enable
 * @param[in]: none 
 * @param[out]: none
 * @retval: TURE/FALSE
 */
BOOL_T app_elec_beacon_remote_is_enable(VOID);

/**
 * @name: get_ble_pair_time
 * @msg:  蓝牙配对时间
 * @param {*}
 * @return {*}
 */
UINT_T get_ble_pair_time(VOID);
/**
 * @function: app_elec_set_ble_pair_time
 * @description: app_elec_set_ble_pair_time
 * @param[in]: pair_time 
 * @param[out]: none
 * @retval: none
 */
VOID app_elec_set_ble_pair_time(IN UINT_T pair_time);
/**
 * @brief 蓝牙遥控器初始化
 *
 * @param cfg 回调注册
 * @param is_active_bind_timeout 是否开启配网成功，蓝牙配对超时
 * @return OPERATE_RET
 */
OPERATE_RET tuya_ble_remote_ctrl_init(ble_ctrl_reg_cfg* cfg);
/**
 * @brief 初始化
 *
 * @param 
 * @param  
 * @return OPERATE_RET
 */
OPERATE_RET app_beacon_remote_init(UINT_T pair_time);
#ifdef __cplusplus
} // extern "C"
#endif

#endif
