/**
 * @file tdl_relay_driver.h
 * @author www.tuya.com
 * @brief tdl_relay_driver module is used to 
 * @version 0.1
 * @date 2023-03-23
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __TDL_RELAY_DRIVER_H__
#define __TDL_RELAY_DRIVER_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
typedef PVOID_T RELAY_DRV_HANDLE;

/***********************************************************
***********************typedef define***********************
***********************************************************/

typedef struct {
    OPERATE_RET (*open)(RELAY_DRV_HANDLE handle);
    OPERATE_RET (*close)(RELAY_DRV_HANDLE handle);
    OPERATE_RET (*write)(RELAY_DRV_HANDLE handle, UINT8_T status);
}RELAY_DRV_INTFS_T;

/***********************************************************
********************function declaration********************
***********************************************************/

OPERATE_RET tdl_relay_driver_register(IN CHAR_T *name, IN RELAY_DRV_INTFS_T *intfs, IN RELAY_DRV_HANDLE drv_hdl);

#ifdef __cplusplus
}
#endif

#endif /* __TDL_RELAY_DRIVER_H__ */
