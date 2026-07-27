/**
 * @file tdd_relay_elec.h
 * @author www.tuya.com
 * @brief tdd_relay_elec module is used to 
 * @version 0.1
 * @date 2023-03-23
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __TDD_RELAY_ELEC_H__
#define __TDD_RELAY_ELEC_H__

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
typedef struct {
    TUYA_GPIO_NUM_E pin;
    TUYA_GPIO_MODE_E mode;
    TUYA_GPIO_LEVEL_E level; // 有效电平（0： 低电平导通继电器， 1： 高电平导通继电器）
}ELEC_RELAY_DRIVER_CONFIG_T;

/***********************************************************
********************function declaration********************
***********************************************************/

OPERATE_RET tdd_relay_elec_register(IN CHAR_T *name, IN ELEC_RELAY_DRIVER_CONFIG_T cfg);

#ifdef __cplusplus
}
#endif

#endif /* __TDD_RELAY_ELEC_H__ */
