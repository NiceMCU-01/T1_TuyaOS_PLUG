/**
 * @file tbl_countdown_timer.h
 * @author www.tuya.com
 * @brief tbl_countdown_timer module is used to
 * @version 1.0.0
 * @date 2022-05-11
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#ifndef __TBL_COUNTDOWN_TIMER_H__
#define __TBL_COUNTDOWN_TIMER_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
*************************micro define***********************
***********************************************************/
#define TBL_CD_DEFAULT_CYCLE_TIME_S               (30u)

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef PVOID_T TBL_CD_TM_HANDLE;

typedef VOID_T (*COUNTDOWN_TIME_CB)(TBL_CD_TM_HANDLE cd_hdl, IN UINT32_T remain_s, VOID_T *args);

/***********************************************************
***********************variable define**********************
***********************************************************/

/***********************************************************
***********************function define**********************
***********************************************************/

/**
 * @brief 倒计时初始化
 *
 * @param[in] fun_cb: 周期回调函数
 * @param[in] args: 回调函数参数
 * @param[out] cd_hdl: 倒计时句柄
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tbl_countdown_time_create(IN COUNTDOWN_TIME_CB fun_cb, VOID_T *args, OUT TBL_CD_TM_HANDLE *cd_hdl);

/**
 * @brief 开启倒计时
 *
 * @param[in] cd_hdl: 倒计时句柄
 * @param[in] cycle_s: 回调周期（单位秒），推荐值30
 * @param[in] cd_time_s 倒计时时间，单位秒。 0-86340 (0 - 23h:59m)
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tbl_countdown_time_start(IN TBL_CD_TM_HANDLE cd_hdl, IN UINT8_T cycle_s, IN UINT32_T cd_time_s);

/**
 * @brief 停止倒计时
 *
 * @param[in] cd_hdl: 倒计时句柄
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tbl_countdown_time_stop(IN TBL_CD_TM_HANDLE cd_hdl);

/**
 * @brief 停止所有倒计时
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tbl_countdown_time_all_stop(VOID_T);

/**
 * @brief 获取倒计剩余时间
 *
 * @param[in] cd_hdl: 倒计时句柄
 * 
 * @return 倒计时剩余时间，单位秒
 */
UINT32_T tbl_countdown_time_get_remain_sec(IN TBL_CD_TM_HANDLE cd_hdl);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__TBL_COUNTDOWN_TIMER_H__*/
