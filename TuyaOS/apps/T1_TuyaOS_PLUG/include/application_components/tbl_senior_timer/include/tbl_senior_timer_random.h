/**
 * @file tbl_senior_timer_random.h
 * @author www.tuya.com
 * @version 0.1
 * @date 2023-04-18
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __TBL_SENIOR_TIMER_RANDOM_H__
#define __TBL_SENIOR_TIMER_RANDOM_H__

#include "tfm_basic_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
*********************** macro define ***********************
***********************************************************/


/***********************************************************
********************** typedef define **********************
***********************************************************/
typedef struct {
    BASIC_TM_CFG_BIT_T       cfg_bit;
    UCHAR_T                  week;           // 00-单次 / 01-周日 / 02-周一 / 04-周二 / 08-周三 / 10-周四 / 20-周五 / 40-周六
    USHORT_T                 start_time;     // 开始时间  minutes
    USHORT_T                 end_time;       // 结束时间  minutes
}TBL_TM_RANDOM_POINT_CFG_T;

typedef VOID_T (*TBL_SENIOR_RANDOM_TM_CB)(UCHAR_T point_id, BASIC_TM_STATE_E timer_stata);

/***********************************************************
******************* function declaration *******************
***********************************************************/
/**
 * @brief       随机定时初始化
 *
 * @param[in] : start_tp      定时启动类型
 * @param[in] : cb            定时到达后的回调
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tbl_random_timer_init(BASIC_TM_START_TP_E start_tp, TBL_SENIOR_RANDOM_TM_CB cb);

/**
 * @brief    随机定时节点
 *
 * @param[in] :p_cfg        定时节点控制参数
 * @param[in] :args         扩展参数
 * @param[in] :args_len     扩展参数长度
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tbl_random_timer_add_point(TBL_TM_RANDOM_POINT_CFG_T *p_cfg, UCHAR_T *args, UINT_T args_len);

/**
 * @brief    获取随机定时节点的控制参数
 *
 * @param[in] :point_id     定时节点序列
 * @param[out] :p_cfg       定时节点控制参数
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tbl_random_timer_get_point_cfg(UCHAR_T point_id, OUT TBL_TM_RANDOM_POINT_CFG_T *p_cfg);

/**
 * @brief    获取随机定时节点的扩展参数
 *
 * @param[in] :point_id     定时节点序列
 * @param[in] :p_args_len   扩展参数长度
 * 
 * @return args         扩展参数
 */
 UCHAR_T *tbl_random_timer_get_point_args(UCHAR_T point_id, UINT_T *p_args_len);

/**
 * @brief    获取随机定时节点的随机后的真实参数
 *
 * @param[in] :point_id     定时节点序列
 * @param[out] :p_cfg     定时节点实际控制参数
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tbl_random_timer_get_random_cfg(UCHAR_T point_id, OUT TBL_TM_RANDOM_POINT_CFG_T *p_cfg);

/**
 * @brief    开始随机启动定时
 *
 * @param  none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tbl_random_timer_start(VOID_T);

/**
 * @brief    停止随机定时
 *
 * @param  none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tbl_random_timer_stop(VOID_T);

/**
 * @brief    暂停随机定时
 *
 * @param  none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tbl_random_timer_pause(VOID_T);

/**
 * @brief    停止指定定时节点任务
 *
 * @param[in] :point_id     定时节点序列
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tbl_random_timer_stop_point(UCHAR_T point_id);

/**
 * @brief    获取随机定时节点数量
 *
 * @param  none
 *
 * @return UCHAR_T  定时节点数量
 */
UCHAR_T tbl_random_timer_get_point_num(VOID_T);

/**
 * @brief    定时节点是否是单次定时
 *
 * @param[in] :point_id     定时节点序列
 *
 * @return TRUE: once timer FALSE: cycle timer
 */
BOOL_T tbl_random_timer_is_point_once(UCHAR_T point_id);

/**
 * @brief    暂停运行中的定时节点任务 (单次任务则永远停止 循环任务等下一个周期)
 *
 * @param[in] :point_id     定时节点序列
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tbl_random_timer_pause_running_point(UCHAR_T point_id);

/**
 * @brief    删除所有定时节点
 *
 * @param  none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tbl_random_timer_delete_all_point(VOID_T);

/**
 * @brief    删除指定定时节点
 *
 * @param  none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tbl_random_timer_delete_point_cfg(TBL_TM_RANDOM_POINT_CFG_T *p_cfg);

#ifdef __cplusplus
}
#endif

#endif /* __TBL_SENIOR_TIMER_RANDOM_H__ */
