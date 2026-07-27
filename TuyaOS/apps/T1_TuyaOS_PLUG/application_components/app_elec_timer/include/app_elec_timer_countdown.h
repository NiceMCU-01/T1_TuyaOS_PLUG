/**
 * @file app_elec_timer_countdown.h
 * @author www.tuya.com
 * @brief app_elec_timer_countdown module is used to 
 * @version 0.1
 * @date 2023-05-04
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __APP_ELEC_TIMER_COUNTDOWN_H__
#define __APP_ELEC_TIMER_COUNTDOWN_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef VOID_T (*APP_ELEC_COUNTDOWN_CB)(UINT8_T countdown_id);

typedef struct {
    /* data */
    UINT8_T id; // 倒计时 ID (1-N)
    UINT32_T time_s; // 倒计时时间，单位秒
}APP_ELEC_COUNTDOWN_CFG_T;


/***********************************************************
********************function declaration********************
***********************************************************/

VOID_T app_elec_countdown_time_num_set(UINT8_T num);

/**
 * @brief 电工倒计时功能初始化
 *
 * @param[in] cb: 倒计时时间到达后回调函数
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_countdown_time_init(APP_ELEC_COUNTDOWN_CB cb);

/**
 * @brief 电工倒计时设置函数
 *
 * @param[in] countdown_cfg: 倒计时配置函数
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_countdown_time_set(APP_ELEC_COUNTDOWN_CFG_T countdown_cfg);

/**
 * @brief 电工倒计时上报函数
 *
 * @param[in] countdown_id: 倒计时 ID (1-N)
 * @param[in] remain_sec: 剩余时间，单位：秒
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_countdown_time_upload(UINT8_T countdown_id, UINT_T remain_sec);

/**
 * @brief 电工倒计时上报函数，通过 DPID 上报
 *
 * @param[in] dpid: 倒计时的 DPID
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_countdown_time_upload_by_dpid(BYTE_T dpid);

/**
 * @brief 上报所有倒计时状态函数
 *
 * @param[in] none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_countdown_time_all_upload(VOID_T);

/**
 * @brief 设置对应倒计时的 DP ID
 *
 * @param[in] countdown_id: 倒计时 ID (1-N)
 * @param[in] dpid: DP ID
 *
 * @return none
 */
VOID_T app_elec_countdown_dpid_set(UINT8_T countdown_id, BYTE_T dpid);

/**
 * @brief 获取对应倒计时剩余时间
 *
 * @param[in] countdown_id: 倒计时 ID (1-N)
 * @param[out] remain_sec: 剩余倒计时时间，单位：秒
 *
 * @return none
 */
OPERATE_RET app_elec_countdown_time_remain_sec_get(UINT8_T countdown_id, UINT32_T *remain_sec);

#ifdef __cplusplus
}
#endif

#endif /* __APP_ELEC_TIMER_COUNTDOWN_H__ */
