/**
 * @file tdd_relay_mag.h
 * @author www.tuya.com
 * @brief tdd_relay_mag module is used to 
 * @version 0.1
 * @date 2023-03-23
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __TDD_RELAY_MAG_H__
#define __TDD_RELAY_MAG_H__

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
/* 磁保持继电器：
    继电器打开状态：on_pin：会设置为高电平，off_pin：会设置为低电平
    继电器关闭状态：on_pin：会设置为低电平，off_pin：会设置为高电平
 */
typedef struct {
    TUYA_GPIO_NUM_E on_pin;
    TUYA_GPIO_NUM_E off_pin;
    TUYA_GPIO_MODE_E mode;
    TUYA_GPIO_LEVEL_E level; /* 磁保持继电器动作切换后平常状态下的维持电平值（0：平常状态下 on_pin 和 off_pin 均为低电平; 1: 平常状态下 on_pin 和 off_pin 均为高电平）*/
    UINT_T hold_ms; // on_pin 产生高电平脉冲的保持时间,单位毫秒(ms)
}MAG_RELAY_DRIVER_CONFIG_T;


/***********************************************************
********************function declaration********************
***********************************************************/

OPERATE_RET tdd_relay_mag_register(IN CHAR_T *name, IN MAG_RELAY_DRIVER_CONFIG_T cfg);

#ifdef __cplusplus
}
#endif

#endif /* __TDD_RELAY_MAG_H__ */
