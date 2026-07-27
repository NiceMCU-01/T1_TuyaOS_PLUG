/**
 * @file app_elec_fault.h
 * @author www.tuya.com
 * @brief app_elec_fault module is used to 
 * @version 0.1
 * @date 2023-07-07
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __APP_ELEC_FAULT_H__
#define __APP_ELEC_FAULT_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
/*故障类型最多15种*/
typedef UINT_T APP_ELEC_FAULT_T;
#define APP_ELEC_FAULT_OVER_CURRENT (1<<0)
#define APP_ELEC_FAULT_OVER_VOLTAGE (1<<1)
#define APP_ELEC_FAULT_OVER_POWER   (1<<2)
#define APP_ELEC_FAULT_LESS_CURRENT (1<<3)
#define APP_ELEC_FAULT_LESS_VOLTAGE (1<<4)
#define APP_ELEC_FAULT_LESS_POWER   (1<<5)

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef VOID_T (*FAULT_CALLBACK)(APP_ELEC_FAULT_T fault);

/***********************************************************
********************function declaration********************
***********************************************************/

VOID_T app_elec_fault_params_set(UINT32_T over_voltage, UINT32_T less_voltage, UINT32_T over_current,UINT32_T over_power);

OPERATE_RET app_elec_fault_init(FAULT_CALLBACK cb);

APP_ELEC_FAULT_T app_elec_fault_value_get(VOID_T);

OPERATE_RET app_elec_fault_upload(VOID_T);

#ifdef __cplusplus
}
#endif

#endif /* __APP_ELEC_FAULT_H__ */
