/**
 * @file tdl_relay_manage.h
 * @author www.tuya.com
 * @brief tdl_relay_manage module is used to 
 * @version 0.1
 * @date 2023-03-23
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __TDL_RELAY_MANAGE_H__
#define __TDL_RELAY_MANAGE_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
#define RELAY_DEV_NAME_LEN_MAX      (16u)

typedef UINT8_T RELAY_STATUS_E;
#define RELAY_STATUS_OFF    0
#define RELAY_STATUS_ON     1


/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef PVOID_T RELAY_HANDLE_T;

/***********************************************************
********************function declaration********************
***********************************************************/

OPERATE_RET tdl_relay_dev_find(CHAR_T *name, RELAY_HANDLE_T *handle);

OPERATE_RET tdl_relay_dev_open(RELAY_HANDLE_T handle);

OPERATE_RET tdl_relay_dev_write(RELAY_HANDLE_T handle, RELAY_STATUS_E status);

OPERATE_RET tdl_relay_dev_write_without_lock(RELAY_HANDLE_T handle, RELAY_STATUS_E status);

OPERATE_RET tdl_relay_dev_read(RELAY_HANDLE_T handle, RELAY_STATUS_E *status);

OPERATE_RET tdl_relay_dev_close(RELAY_HANDLE_T handle);

#ifdef __cplusplus
}
#endif

#endif /* __TDL_RELAY_MANAGE_H__ */
