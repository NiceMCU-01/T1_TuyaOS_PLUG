/**
* @file tfm_basic_timer.h
* @author www.tuya.com
* @version 1.0.0
* @date 2022-05-11
*
* @copyright Copyright (c) tuya.inc 2022
*
*/

#ifndef __TFM_SENIOR_TIMER_BASIC_H__
#define __TFM_SENIOR_TIMER_BASIC_H__

#include "tuya_cloud_types.h"
#include "tal_time_service.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
*************************micro define***********************
***********************************************************/
#define BASIC_TM_POINT_INVALID_ID        0xFF
#define BASIC_TM_POINT_NUM_MAX           0xFF   //255

#define BASIC_TM_INVALID_TIME            0xFFFFFFFF

#define BASIC_TM_ONE_DAY_MINUTE          (24u*60u)
#define BASIC_TM_ONE_MIN_SEC             (60u)    // 一分钟秒数

#define BASIC_TM_GET_DIFF_TIME_MINUTE(start, end) \
        (((end) > (start)) ? ((end) - (start)) : (BASIC_TM_ONE_DAY_MINUTE - (start) + (end)))

#define BASIC_TM_WEEK_ROL(val, n)       \
        (((val << n) | (val >> (7 - n))) & 0x7F)     //循环左移 提前n天
#define BASIC_TM_WEEK_ROR(val, n)       \
        (((val >> n) | (val << (7 - n))) & 0x7F)     //循环右移 推迟n天

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef UCHAR_T BASIC_TM_START_TP_E;
#define BASIC_TM_START_TP_HALF                0     // 半周期执行:定时可从中段开始执行
#define BASIC_TM_START_TP_FULL                1     // 全周期执行:定时必须从开始时间开始执行

typedef UCHAR_T BASIC_TM_STATE_E;
#define BASIC_TM_STATE_START                  1
#define BASIC_TM_STATE_END                    2
#define BASIC_TM_STATE_END_FOREVER            3

typedef PVOID_T BASIC_TM_HANDLE_T;
typedef PVOID_T BASIC_TM_POINT_HANDLE_T;

typedef struct {
    UCHAR_T en      : 1;
    UCHAR_T obj_idx : 7;
}BASIC_TM_CFG_BIT_T;

//单点定时：只设置有效的起始时间，结束时间设为 TBL_SENIOR_TM_INVAILD_TIME
typedef struct {
    BASIC_TM_CFG_BIT_T       cfg_bit;
    UCHAR_T                  week;           // 00-单次 / 01-周日 / 02-周一 / 04-周二 / 08-周三 / 10-周四 / 20-周五 / 40-周六
    UINT_T                   start_time;     // 开始时间  单位/秒
    UINT_T                   end_time;       // 结束时间  单位/秒
}BASIC_TM_POINT_T;

typedef struct {
    BASIC_TM_HANDLE_T        basic_tm_hdl;
    BASIC_TM_POINT_HANDLE_T  tm_point_hdl;
    BASIC_TM_POINT_T        *p_point_cfg;
}BASIC_TM_INFORM_T;

/**
* @brief 定时时间到后的回调函数
*
* @param[in] handle       控制句柄
* @param[in] point_id     定时节点序列 从 1 开始
* @param[in] tm_state     定时状态
* @return 执行结果 TRUE:执行成功，正常运行 FALSE: 执行失败，则本次不再执行，延迟下一次再执行(单次定时并不会disable)
*/
typedef BOOL_T (*TFM_BASIC_TM_CB)(BASIC_TM_INFORM_T *p_infor, BASIC_TM_STATE_E tm_state, POSIX_TM_S *p_curr_time, PVOID_T args);

/***********************************************************
***********************variable define**********************
***********************************************************/


/***********************************************************
***********************function define**********************
***********************************************************/
/**
 * @brief       创建高级定时
 *
 * @param[in] : start_tp      定时启动类型
 * @param[in] : cb            定时到达后的回调
 * @param[in] : ext_len       扩展长度
 * @param[out] : handle       定时控制句柄
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tfm_basic_timer_create(BASIC_TM_START_TP_E start_tp, TFM_BASIC_TM_CB cb, BASIC_TM_HANDLE_T *handle);

/**
 * @brief    增加定时节点
 *
 * @param[in] :handle        定时控制句柄
 * @param[in] :p_point       定时节点控制参数
 * @param[in] :args          拓展参数
 * @param[out] : point_hdl   定时节点控制句柄
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tfm_basic_timer_add_point(BASIC_TM_HANDLE_T handle, BASIC_TM_POINT_T *p_point, PVOID_T args,\
                                      BASIC_TM_POINT_HANDLE_T *point_hdl);

/**
 * @brief    寻找定时节点的控制参数
 *
 * @param[in] :handle       定时控制句柄
 * @param[in] :id           定时节点序列
 * @param[out] :point_hdl   定时节点控制句柄
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
 OPERATE_RET tfm_basic_timer_find_point_handle(BASIC_TM_HANDLE_T handle, UCHAR_T id, BASIC_TM_POINT_HANDLE_T *point_hdl);

 /**
 * @brief    寻找定时节点序列
 *
 * @param[in] :handle       定时控制句柄
 * @param[in] :point_hdl   定时节点控制句柄
 * @param[out] :id         定时节点序列
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
 OPERATE_RET tfm_basic_timer_find_point_id(BASIC_TM_HANDLE_T handle, BASIC_TM_POINT_HANDLE_T point_hdl, UCHAR_T *id);

/**
 * @brief    设置定时节点的控制参数
 *
 * @param[in] :handle       定时控制句柄
 * @param[in] :point_hdl    定时节点控制句柄
 * @param[in] :p_point      定时节点控制参数
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tfm_basic_timer_set_point_cfg(BASIC_TM_HANDLE_T handle, BASIC_TM_POINT_HANDLE_T point_hdl,\
                                           BASIC_TM_POINT_T *p_point);

/**
 * @brief    获取定时节点的控制参数
 *
 * @param[in] :handle       定时控制句柄
 * @param[in] :point_hdl    定时节点控制句柄
 * @param[out] :p_point     定时节点控制参数
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tfm_basic_timer_get_point_cfg(BASIC_TM_HANDLE_T handle, BASIC_TM_POINT_HANDLE_T point_hdl,\
                                           BASIC_TM_POINT_T *p_point);

/**
 * @brief    获取定时节点的扩展参数
 *
 * @param[in] :handle       定时控制句柄
 * @param[in] :point_hdl    定时节点控制句柄
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
PVOID_T tfm_basic_timer_get_point_args(BASIC_TM_HANDLE_T handle, BASIC_TM_POINT_HANDLE_T point_hdl);

/**
 * @brief    开始启动定时
 *
 * @param[in] :handle       定时控制句柄
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tfm_basic_timer_start(BASIC_TM_HANDLE_T handle);

/**
 * @brief    停止定时
 *
 * @param[in] :handle       定时控制句柄
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tfm_basic_timer_stop(BASIC_TM_HANDLE_T handle);

/**
 * @brief    暂停定时
 *
 * @param[in] :handle       定时控制句柄
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tfm_basic_timer_pause(BASIC_TM_HANDLE_T handle);

/**
 * @brief    获取定时节点数量
 *
 * @param[in] :handle       定时控制句柄
 *
 * @return UCHAR_T  定时节点数量
 */
UCHAR_T tfm_basic_timer_get_point_num(BASIC_TM_HANDLE_T handle);

/**
 * @brief    是否有定时正在定时期间
 *
 * @param[in] :handle       定时控制句柄
 *
 * @return TRUE: 有定时节点处于定时期间 FALSE: 没有定时节点处于定时期间
 */
BOOL_T tfm_basic_timer_is_timing(BASIC_TM_HANDLE_T handle);

/**
 * @brief    定时节点是否是单次定时
 *
 * @param[in] :handle       定时控制句柄
 * @param[in] :point_hdl    定时节点控制句柄
 *
 * @return TRUE: once timer FALSE: cycle timer
 */
BOOL_T tfm_basic_timer_is_point_once(BASIC_TM_HANDLE_T handle, BASIC_TM_POINT_HANDLE_T point_hdl);

/**
 * @brief    暂停运行中的定时节点任务 (单次任务则永远停止 循环任务等下一个周期)
 *
 * @param[in] :handle       定时控制句柄
 * @param[in] :point_hdl    定时节点控制句柄
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tfm_basic_timer_pause_running_point(BASIC_TM_HANDLE_T handle, BASIC_TM_POINT_HANDLE_T point_hdl);

/**
 * @brief    删除所有定时节点
 *
 * @param[in] :handle       定时控制句柄
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tfm_basic_timer_delete_all_point(BASIC_TM_HANDLE_T handle);

/**
 * @brief    删除指定定时节点
 *
 * @param[in] :handle       定时控制句柄
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tfm_basic_timer_delete_point(BASIC_TM_HANDLE_T handle, BASIC_TM_POINT_HANDLE_T point_hdl);

/**
 * @brief       打印定时节点数据
 *
 * @param[in] : p_cfg 定时节点控制参数
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T tfm_basic_timer_print_point(IN BASIC_TM_POINT_T *p_cfg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__TBL_TBL_SENIOR_TIMER_BASIC_H__*/