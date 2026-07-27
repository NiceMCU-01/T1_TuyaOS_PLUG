/**
 * @file app_elec_timer_random.h
 * @author www.tuya.com
 * @brief app_elec_timer_random module is used to 
 * @version 0.1
 * @date 2023-04-27
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __APP_ELEC_TIMER_RANDOM_H__
#define __APP_ELEC_TIMER_RANDOM_H__

#include "tuya_cloud_types.h"
#include "tbl_senior_timer_random.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
typedef UINT8_T RANDOM_STATE_E;
#define RANDOM_STATE_START  0
#define RANDOM_STATE_END    1

/***********************************************************
***********************typedef define***********************
***********************************************************/

typedef VOID_T (*TM_RANDOM_INFORM_CB)(UINT8_T channel_id, RANDOM_STATE_E state);

/***********************************************************
********************function declaration********************
***********************************************************/

/**
 * @brief 电工随机定时功能初始化
 *
 * @param[in] inform_cb: 随机定时回调函数
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_random_timer_init(TM_RANDOM_INFORM_CB inform_cb);

/**
 * @brief 电工随机定时设置函数
 *
 * @param[in] p_cfg: 随机定时配置参数
 * @param[in] cfg_num: 配置参数个数
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_random_timer_set(TBL_TM_RANDOM_POINT_CFG_T *p_cfg, UINT8_T cfg_num);

/**
 * @brief 电工随机定时设置函数，使用 IoT 平台下发的数据进行配置
 *
 * @param[in] dp_data: 随机定时下发的 DP 参数
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_random_timer_dp_data_set(CHAR_T *dp_data);

/**
 * @brief 电工随机定时存储数据擦除函数
 *
 * @param[in] none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_random_timer_memory_easer(VOID_T);

/**
 * @brief 获取随机定时数据，获取后 p_data 若不为 NULL 应调用 tal_free()
 *
 * @param[in] p_data: 随机定时 base64 后的数据
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_random_timer_str_get(CHAR_T **p_data);

/**
 * @brief 电工随机定时数据上报函数
 *
 * @param[in] none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_random_timer_upload(VOID_T);

#ifdef __cplusplus
}
#endif

#endif /* ENABLE_ELEC_RANDOM_TIMER */

