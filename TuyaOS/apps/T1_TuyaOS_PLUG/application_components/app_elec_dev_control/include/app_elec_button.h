/**
 * @file app_elec_button.h
 * @author www.tuya.com
 * @brief app_elec_button module is used to 
 * @version 0.1
 * @date 2023-03-21
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __APP_ELEC_BUTTON_H__
#define __APP_ELEC_BUTTON_H__

#include "tuya_cloud_types.h"
#include "tdl_button_manage.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
#define ELEC_BUTTON_NUM_MAX     9

typedef UINT_T ELEC_BUTTON_MODE_E;
#define ELEC_BUTTON_MODE_SHORT                  0x01
#define ELEC_BUTTON_MODE_DOUBLE_CLICK           0x02
#define ELEC_BUTTON_MODE_LONG                   0x04
#define ELEC_BUTTON_MODE_REPEAT                 0x08

typedef UINT8_T ELEC_BUTTON_FUNC_E;
#define ELEC_BUTTON_FUNC_EMPTY                  0 // 无功能
#define ELEC_BUTTON_CHANNEL_ALL_TOGGLE          1 // 所有通道反转。只要有一个通道打开，就会关闭所有通道；只有所有都关闭才会打开所有通道。
#define ELEC_BUTTON_CHANNEL_1_TOGGLE            2 // 通道 1 状态反转
#define ELEC_BUTTON_CHANNEL_2_TOGGLE            3
#define ELEC_BUTTON_CHANNEL_3_TOGGLE            4
#define ELEC_BUTTON_CHANNEL_4_TOGGLE            5
#define ELEC_BUTTON_CHANNEL_5_TOGGLE            6
#define ELEC_BUTTON_CHANNEL_6_TOGGLE            7
#define ELEC_BUTTON_CHANNEL_7_TOGGLE            8
#define ELEC_BUTTON_CHANNEL_8_TOGGLE            9
#define ELEC_BUTTON_LOCAL_RESET                 10 // 本地移除设备
#define ELEC_BUTTON_CHILD_LOCK_UNLOCK           11 // 解锁童锁
// #define ELEC_BUTTON_PROD_TEST                   12 // 产测状态

/***********************************************************
***********************typedef define***********************
***********************************************************/


typedef VOID_T (*APP_ELEC_BUTTON_FUN_CB)(ELEC_BUTTON_MODE_E mode, ELEC_BUTTON_FUNC_E fun);

/***********************************************************
********************function declaration********************
***********************************************************/

/**
* @brief        按键功能初始化 
*
* @param[in]    func_cb    按键功能回调
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET app_elec_button_init(APP_ELEC_BUTTON_FUN_CB func_cb);

/**
 * @brief 按键长按时间设置。如果需要进行设置，需要在 app_elec_button_init() 之前
 *
 * @param[in] index: 索引，从0开始
 * @param[in] long_time_ms: 长按时间，单位 ms
 *
 * @return none
 */
VOID_T app_elec_button_long_time_set(UINT8_T index, USHORT_T long_time_ms);

VOID_T app_elec_button_number_set(UINT8_T num);

// VOID_T app_elec_button_product_test(VOID_T);

#ifdef __cplusplus
}
#endif

#endif /* __APP_ELEC_BUTTON_H__ */
