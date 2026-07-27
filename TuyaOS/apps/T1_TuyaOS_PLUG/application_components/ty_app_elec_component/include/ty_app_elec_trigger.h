/**
 * @file ty_app_elec_trigger.h
 * @author www.tuya.com
 * @brief ty_app_elec_trigger module is used to 
 * @version 0.1
 * @date 2023-03-21
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __TY_APP_ELEC_TRIGGER_H__
#define __TY_APP_ELEC_TRIGGER_H__

#include "tuya_cloud_types.h"

#include "app_elec_button.h"
#include "app_elec_child_lock.h"
#include "app_elec_timer_random.h"
#include "app_elec_timer_cycle.h"
#include "app_elec_fault.h"

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
 * @brief       检测到有效按键后，触发相应事件
 *
 * @param[in] :    mode     按键模式
 * @param[in] :    fun      按键功能
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T ty_app_elec_button_trigger(ELEC_BUTTON_MODE_E mode, ELEC_BUTTON_FUNC_E fun);

/**
 * @brief       产测模式下的按键回调
 *
 * @param[in] :    mode     按键模式
 * @param[in] :    fun      按键功能
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T ty_app_elec_button_factory_trigger(ELEC_BUTTON_MODE_E mode, ELEC_BUTTON_FUNC_E fun);

/**
 * @brief 延时关回调函数
 *
 * @param[in] delay_off_id: 延时关 ID，这里可以理解为通道 ID
 * @param[in] arg: 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T ty_app_elec_delay_off_trigger(UINT8_T channel_id);

/**
 * @brief 童锁状态改变回调函数
 *
 * @param[in] status: 童锁状态
 *
 * @return none
 */
VOID_T ty_app_elec_child_lock_trigger(CHILD_LOCK_STATUS_E status);

/**
 * @brief 随机定时触发函数
 *
 * @param[in] channel_id: 通道 ID
 * @param[in] state: 要设置的通道状态
 *
 * @return none
 */
VOID_T ty_app_elec_random_timer_trigger(UINT8_T channel_id, RANDOM_STATE_E state);

/**
 * @brief 循环定时触发函数
 *
 * @param[in] channel_id: 通道 ID
 * @param[in] state: 要设置的通道状态
 *
 * @return none
 */
VOID_T ty_app_elec_cycle_timer_trigger(UINT8_T channel_id, BOOL_T state);

/**
 * @brief 倒计时定时触发函数
 *
 * @param[in] countdown_id: 倒计时 ID
 *
 * @return none
 */
VOID_T ty_app_elec_time_countdown_trigger(UINT8_T countdown_id);

/**
 * @brief 过充触发函数
 *
 * @param[in] none
 *
 * @return none
 */
VOID_T ty_app_elec_overcharge_trigger(VOID_T);

/**
 * @brief 错误触发函数
 *
 * @param[in] fault: 错误事件
 *
 * @return none
 */
VOID_T ty_app_elec_fault_trigger(APP_ELEC_FAULT_T fault);

#ifdef __cplusplus
}
#endif

#endif /* __TY_APP_ELEC_TRIGGER_H__ */
