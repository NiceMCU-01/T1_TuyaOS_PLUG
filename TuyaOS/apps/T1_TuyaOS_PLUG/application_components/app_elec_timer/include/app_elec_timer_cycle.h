/**
 * @file app_elec_timer_cycle.h
 * @author www.tuya.com
 * @brief app_elec_timer_cycle module is used to 
 * @version 0.1
 * @date 2023-05-04
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __APP_ELEC_TIMER_CYCLE_H__
#define __APP_ELEC_TIMER_CYCLE_H__

#include "tuya_cloud_types.h"

#include "tbl_senior_timer_cycle.h"
#include "tfm_basic_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef VOID_T (*TM_CYCLE_INFORM_CB)(UINT8_T channel_id, BOOL_T state);

/***********************************************************
********************function declaration********************
***********************************************************/

/**
 * @brief 电工循环定时功能初始化
 *
 * @param[in] cb: 循环定时回调函数
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_cycle_timer_init(TM_CYCLE_INFORM_CB inform_cb);

/**
 * @brief 电工循环定时设置函数，使用 IoT 平台下发的数据进行配置
 *
 * @param[in] dp_data: 循环定时下发的 DP 参数
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_cycle_timer_dp_data_set(CHAR_T *dp_data);

/**
 * @brief 电工循环定时存储数据擦除函数
 *
 * @param[in] none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_cycle_timer_memory_easer(VOID_T);

/**
 * @brief 获取循环定时数据，获取后 p_data 若不为 NULL 应调用 tal_free()
 *
 * @param[in] p_data: 循环定时 base64 后的数据
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_cycle_timer_str_get(CHAR_T **p_data);

/**
 * @brief 电工循环定时数据上报函数
 *
 * @param[in] none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_cycle_timer_upload(VOID_T);

#ifdef __cplusplus
}
#endif

#endif /* ENABLE_ELEC_CYCLE_TIMER */
