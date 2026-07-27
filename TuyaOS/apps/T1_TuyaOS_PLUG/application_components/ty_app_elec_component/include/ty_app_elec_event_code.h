/**
 * @file ty_app_elec_event_code.h
 * @author www.tuya.com
 * @brief ty_app_elec_event_code module is used to 
 * @version 0.1
 * @date 2023-03-20
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __TY_APP_ELEC_EVENT_CODE_H__
#define __TY_APP_ELEC_EVENT_CODE_H__

#include "ty_sys.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/

// device control
#define ELEC_DEV_CTRL_EVT_START             0x00000001
// channel 
#define EVT_ELEC_CHANNEL_STATUS_SET         (ELEC_DEV_CTRL_EVT_START)
#define EVT_ELEC_POWER_ON_MODE_SET          (ELEC_DEV_CTRL_EVT_START+1)
// child lock
#define EVT_ELEC_CHILD_LOCK_ENABLE          (ELEC_DEV_CTRL_EVT_START+2)
#define EVT_ELEC_CHILD_LOCK_DISABLE         (ELEC_DEV_CTRL_EVT_START+3)
#define EVT_ELEC_CHILD_AUTO_LOCK_ENABLE     (ELEC_DEV_CTRL_EVT_START+4)
#define EVT_ELEC_CHILD_AUTO_LOCK_DISABLE    (ELEC_DEV_CTRL_EVT_START+5)
// delay off
#define EVT_ELEC_DELAY_OFF_SET              (ELEC_DEV_CTRL_EVT_START+6)
// countdown time
#define EVT_ELEC_COUNTDOWN_TIME_SET         (ELEC_DEV_CTRL_EVT_START+7)
// cycle timer
#define EVT_ELEC_CYCLE_TIMER_STR_SET        (ELEC_DEV_CTRL_EVT_START+8)
// random timer
#define EVT_ELEC_RANDOM_TIMER_STR_SET       (ELEC_DEV_CTRL_EVT_START+9)
// dp
#define EVT_ELEC_REPORT_DP_ALL              (ELEC_DEV_CTRL_EVT_START+10)
#define EVT_ELEC_REPORT_DP                  (ELEC_DEV_CTRL_EVT_START+11)
// data erase
#define EVT_ELEC_DEV_DATA_ERASE             (ELEC_DEV_CTRL_EVT_START+12)
// light mode
#define EVT_ELEC_LIGHT_MODE_SET             (ELEC_DEV_CTRL_EVT_START+13)
// overcharge
#define EVT_ELEC_OVERCHARGE_SET             (ELEC_DEV_CTRL_EVT_START+14)
// runtime sitch
#define EVT_ELEC_RUNTIME_SWITCH_SET         (ELEC_DEV_CTRL_EVT_START+15)
// network
#define ELEC_NETWORK_EVT_START              0x00000030
#define EVT_ELEC_DEV_LOCAL_REMOVE           (ELEC_NETWORK_EVT_START+1)
#define EVT_ELEC_NET_INDICATE_NOT_CONNECT   (ELEC_NETWORK_EVT_START+2)
#define EVT_ELEC_NET_INDICATE_CONNECTED     (ELEC_NETWORK_EVT_START+3)
#define EVT_ELEC_EZ_INDICATE                (ELEC_NETWORK_EVT_START+4)
#define EVT_ELEC_AP_INDICATE                (ELEC_NETWORK_EVT_START+5)
#define ELEC_NETWORK_EVT_END                0x0000003F

// 产测事件
#define ELEC_PROD_TEST_EVT_START            0x00000040
#define EVT_ELEC_PROD_UNAUTHOR              (ELEC_PROD_TEST_EVT_START)
#define EVT_ELEC_PROD_WEAK_SIGNAL           (ELEC_PROD_TEST_EVT_START+1)
#define EVT_ELEC_PROD_FIND_SSID             (ELEC_PROD_TEST_EVT_START+2)
#define EVT_ELEC_PROD_TEST_1                (ELEC_PROD_TEST_EVT_START+3) // 进入产测 1
#define EVT_ELEC_PROD_TEST_2                (ELEC_PROD_TEST_EVT_START+4)
#define EVT_ELEC_PT_NET_LED_SET             (ELEC_PROD_TEST_EVT_START+5) // 产测时配网指示灯设置
#define EVT_ELEC_PT_NOT_CHAN_REVERSE        (ELEC_PROD_TEST_EVT_START+6) // 通道反转三次（非计量产测）
#define EVT_ELEC_PT_DATA_ERASE              (ELEC_PROD_TEST_EVT_START+7)
#define ELEC_PROD_TEST_EVT_END              0x0000004F

#ifdef __cplusplus
}
#endif

#endif /* __TY_APP_ELEC_EVENT_CODE_H__ */
