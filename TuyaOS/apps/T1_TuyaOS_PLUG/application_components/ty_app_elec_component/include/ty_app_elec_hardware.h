/**
 * @file ty_app_elec_hardware.h
 * @author www.tuya.com
 * @brief ty_app_elec_hardware module is used to 
 * @version 0.1
 * @date 2023-03-20
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __TY_APP_ELEC_HARDWARE_H__
#define __TY_APP_ELEC_HARDWARE_H__

#include "tuya_app_config.h"

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
typedef UINT8_T TY_ENERGY_MONITOR_CHIP_T;
#define TY_ENERGY_MONITOR_BL0937  0
#define TY_ENERGY_MONITOR_HLW8012 1
#define TY_ENERGY_MONITOR_HLW8032 2
#define TY_ENERGY_MONITOR_BL0942  3

/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
********************function declaration********************
***********************************************************/

#if defined(ENABLE_TY_LOAD_OEM_PAR) && (ENABLE_TY_LOAD_OEM_PAR)
/**
 * @brief    OEM 硬件注册
 *
 * @param     none 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_elec_oem_hardware_reg(VOID_T);
#else
/**
 * @brief    硬件注册
 *
 * @param     none 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_elec_hardware_reg(VOID_T);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __TY_APP_ELEC_HARDWARE_H__ */
