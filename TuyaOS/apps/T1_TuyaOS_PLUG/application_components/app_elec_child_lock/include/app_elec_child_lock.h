/**
 * @file app_elec_child_lock.h
 * @author www.tuya.com
 * @brief app_elec_child_lock module is used to 
 * @version 0.1
 * @date 2023-04-21
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __APP_ELEC_CHILD_LOCK_H__
#define __APP_ELEC_CHILD_LOCK_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
typedef UINT8_T CHILD_LOCK_STATUS_E;
#define STATUS_UNLOCK   0
#define STATUS_LOCK     1

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef VOID_T (*APP_ELEC_CHILD_LOCK_CB)(CHILD_LOCK_STATUS_E status);

typedef struct {
    UINT8_T auto_lock_enable;
    UINT32_T auto_lock_time_ms; // 单位：ms
} APP_ELEC_CHILD_LOCK_CONFIG_T;

/***********************************************************
********************function declaration********************
***********************************************************/

/**
 * @brief 童锁初始化函数
 *
 * @param[in] usr_cfg: 设置为 NULL，使用默认参数（Kconfig 配置参数）；不为 NULL 使用配置的参数
 * @param[in] cb: 童锁状态改变时回调函数
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_child_lock_init(APP_ELEC_CHILD_LOCK_CONFIG_T *usr_cfg, APP_ELEC_CHILD_LOCK_CB cb);

/**
 * @brief 童锁状态设置函数
 *
 * @param[in] status: 要设置的童锁状态
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_child_lock_status_set(CHILD_LOCK_STATUS_E status);

/**
 * @brief 童锁状态获取函数
 *
 * @param[in] none
 *
 * @return 当前童锁状态
 */
CHILD_LOCK_STATUS_E app_elec_child_lock_status_get(VOID_T);

/**
 * @brief 童锁自动上锁设置函数。开启自动上锁后，设备解锁童锁后 auto_lock_time_ms 时间后会自动上锁
 *
 * @param[in] auto_lock_enable: 0: 关闭自动上锁功能；1：开启自动上锁功能
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_child_auto_lock_status_set(UINT8_T auto_lock_enable);

/**
 * @brief 童锁打开自动自动上锁功能，解锁后，调用该函数会重置自动上锁定时器，重新计时
 *
 * @param[in] none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_child_auto_lock_refresh(VOID_T);

/**
 * @brief 童锁自动上锁状态获取函数
 *
 * @param[in] none
 *
 * @return 当前童锁自动上锁状态
 */
UINT8_T app_elec_child_auto_lock_status_get(VOID_T);

/**
 * @brief 童锁 DP 上报函数
 *
 * @param[in] none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_child_lock_dp_data_upload(VOID_T);

#ifdef __cplusplus
}
#endif

#endif /* __APP_ELEC_CHILD_LOCK_H__ */
