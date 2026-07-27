/**
 * @file ty_app_elec_event.h
 * @author www.tuya.com
 * @brief ty_app_elec_event module is used to 
 * @version 0.1
 * @date 2023-04-17
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __TY_APP_ELEC_EVENT_H__
#define __TY_APP_ELEC_EVENT_H__

#include "tuya_cloud_types.h"

#include "ty_sys.h"

#include "ty_app_elec_event_code.h"

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
 * @brief        事件回调函数
 *
 * @param[in] :  event         事件信息
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T ty_app_event_callback(APP_EVENT_MSG_S *event);

/**
 * @brief 获取设备初始化标志
 *
 * @param[in] : VOID_T
 *
 * @return BOOL_T
 */
BOOL_T ty_app_elec_device_init_flag_get(VOID_T);

#ifdef __cplusplus
}
#endif

#endif /* __TY_APP_ELEC_EVENT_H__ */
