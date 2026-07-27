/**
 * @file app_elec_led.h
 * @author www.tuya.com
 * @brief app_elec_led module is used to 
 * @version 0.1
 * @date 2023-07-24
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __APP_ELEC_LED_H__
#define __APP_ELEC_LED_H__

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
/* 网络指示灯指示模式 */
typedef UINT8_T ELEC_NET_INDICATE_MODE_E;
#define INDICATE_MODE_NOT_CONNECT               (0)
#define INDICATE_MODE_CONNECTED                 (1)
#define INDICATE_MODE_AP                        (2)
#define INDICATE_MODE_EZ                        (3)
#define INDICATE_MODE_WEAK_SIGNAL               (4) // 产测路由信号弱
#define INDICATE_MODE_ENERGY_MONITOR            (5) // 计量产测中
#define INDICATE_MODE_PROD_TEST_SUCCESS         (6)
#define INDICATE_MODE_PROD_TEST_FAIL            (7)
#define INDICATE_MODE_PT_NOT_ENERGY_LOW_POWER   (8) // 非计量产测开始，长按配网
#define INDICATE_MODE_PT_NOT_ENERGY_BUTTON      (9) // 非计量产测中，按键触发状态
#define INDICATE_MODE_ALWAYS_ON                 (10)
#define INDICATE_MODE_ALWAYS_OFF                (11)
#define INDICATE_MODE_PT_NOT_ENERGY_AUTO_CFG    (12) // 非计量产测开始，上电配网

// 总控指示灯模式
typedef UINT8_T ELEC_LIGHT_MODE_E;
#define LIGHT_MODE_RELAY        0
#define LIGHT_MODE_POSITION     1
#define LIGHT_MODE_ALWAYS_OFF   2
#define LIGHT_MODE_ALWAYS_ON    3
#define LIGHT_MODE_MAX          4

// 配网指示灯不复用时，未联网，联网后状态
#define LED_ALWAYS_OFF          0
#define LED_ALWAYS_ON           1
#define LED_RELAY_STATE         2

/***********************************************************
********************function declaration********************
***********************************************************/
OPERATE_RET app_elec_net_led_ffc_beacon_set(UINT8_T led_stat);

OPERATE_RET app_elec_net_led_refresh(VOID_T);

VOID_T app_net_led_ffc_beacon_use_set(UINT8_T is_use);

VOID_T app_elec_net_led_mux_set(UINT8_T is_mux);

UINT8_T app_elec_net_led_mux_get(VOID_T);

VOID_T app_elec_net_led_status_set(UINT8_T nety_status, UINT8_T netn_status);

OPERATE_RET app_elec_net_led_init(VOID_T);

OPERATE_RET app_elec_net_led_mode_set(ELEC_NET_INDICATE_MODE_E indicate_mode);

OPERATE_RET app_elec_power_led_init(VOID_T);

OPERATE_RET app_elec_power_led_status_set(UINT8_T chan_status);

OPERATE_RET app_elec_power_led_mode_set(ELEC_LIGHT_MODE_E light_mode);

OPERATE_RET app_elec_power_led_mode_upload(VOID_T);

OPERATE_RET app_elec_power_led_mode_data_erase(VOID_T);

ELEC_LIGHT_MODE_E app_elec_power_led_mode_get(VOID_T);

OPERATE_RET app_elec_no_energy_prod_led(VOID_T);

#ifdef __cplusplus
}
#endif

#endif /* __APP_ELEC_LED_H__ */
