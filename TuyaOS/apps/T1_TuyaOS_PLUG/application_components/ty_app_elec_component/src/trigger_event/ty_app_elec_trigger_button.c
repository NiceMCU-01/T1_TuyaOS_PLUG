/**
 * @file ty_app_elec_trigger_button.c
 * @author www.tuya.com
 * @brief ty_app_elec_trigger_button module is used to 
 * @version 0.1
 * @date 2023-04-24
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_app_config.h"
#include "ty_sys.h"
#include "ty_app_elec_event_code.h"

#include "ty_app_elec_trigger.h"
#include "app_elec_energy_monitor.h"
#include "app_elec_button.h"
#include "app_elec_channel.h"
#include "app_elec_child_lock.h"
#include "app_elec_led.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define CHILD_UNLOCK_CNT    4

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    ELEC_BUTTON_FUNC_E  fun;
    ELEC_BUTTON_MODE_E  mode;

    UINT_T              evt_id;
    void                *evt_data;
    UINT32_T            evt_data_len;
} BUTTON_FUN_EVT_T;

/***********************************************************
********************function declaration********************
***********************************************************/
#if (defined(ENABLE_ELEC_CHILD_LOCK) && (ENABLE_ELEC_CHILD_LOCK==1))
TIMER_ID __child_lock_tm_id = NULL;
STATIC volatile UINT8_T __child_unlock_cnt = 0;
#endif

/***********************************************************
***********************variable define**********************
***********************************************************/
// 反转所有通道状态
APP_ELEC_CHANNEL_CFG_T all_channel_cfg = {.chan_id = 0, .status = STATE_TOGGLE};
// 指定通道状态反转
APP_ELEC_CHANNEL_CFG_T channel_1_cfg = {.chan_id = 1, .status = STATE_TOGGLE};
APP_ELEC_CHANNEL_CFG_T channel_2_cfg = {.chan_id = 2, .status = STATE_TOGGLE};
APP_ELEC_CHANNEL_CFG_T channel_3_cfg = {.chan_id = 3, .status = STATE_TOGGLE};
APP_ELEC_CHANNEL_CFG_T channel_4_cfg = {.chan_id = 4, .status = STATE_TOGGLE};
APP_ELEC_CHANNEL_CFG_T channel_5_cfg = {.chan_id = 5, .status = STATE_TOGGLE};
APP_ELEC_CHANNEL_CFG_T channel_6_cfg = {.chan_id = 6, .status = STATE_TOGGLE};
APP_ELEC_CHANNEL_CFG_T channel_7_cfg = {.chan_id = 7, .status = STATE_TOGGLE};
APP_ELEC_CHANNEL_CFG_T channel_8_cfg = {.chan_id = 8, .status = STATE_TOGGLE};

STATIC CONST BUTTON_FUN_EVT_T c_BUTTON_EVT_LIST[] = {
    {ELEC_BUTTON_CHANNEL_ALL_TOGGLE, ELEC_BUTTON_MODE_SHORT, EVT_ELEC_CHANNEL_STATUS_SET, &all_channel_cfg, sizeof(APP_ELEC_CHANNEL_CFG_T)},
    {ELEC_BUTTON_CHANNEL_1_TOGGLE, ELEC_BUTTON_MODE_SHORT, EVT_ELEC_CHANNEL_STATUS_SET, &channel_1_cfg, sizeof(APP_ELEC_CHANNEL_CFG_T)},
    {ELEC_BUTTON_CHANNEL_2_TOGGLE, ELEC_BUTTON_MODE_SHORT, EVT_ELEC_CHANNEL_STATUS_SET, &channel_2_cfg, sizeof(APP_ELEC_CHANNEL_CFG_T)},
    {ELEC_BUTTON_CHANNEL_3_TOGGLE, ELEC_BUTTON_MODE_SHORT, EVT_ELEC_CHANNEL_STATUS_SET, &channel_3_cfg, sizeof(APP_ELEC_CHANNEL_CFG_T)},
    {ELEC_BUTTON_CHANNEL_4_TOGGLE, ELEC_BUTTON_MODE_SHORT, EVT_ELEC_CHANNEL_STATUS_SET, &channel_4_cfg, sizeof(APP_ELEC_CHANNEL_CFG_T)},
    {ELEC_BUTTON_CHANNEL_5_TOGGLE, ELEC_BUTTON_MODE_SHORT, EVT_ELEC_CHANNEL_STATUS_SET, &channel_5_cfg, sizeof(APP_ELEC_CHANNEL_CFG_T)},
    {ELEC_BUTTON_CHANNEL_6_TOGGLE, ELEC_BUTTON_MODE_SHORT, EVT_ELEC_CHANNEL_STATUS_SET, &channel_6_cfg, sizeof(APP_ELEC_CHANNEL_CFG_T)},
    {ELEC_BUTTON_CHANNEL_7_TOGGLE, ELEC_BUTTON_MODE_SHORT, EVT_ELEC_CHANNEL_STATUS_SET, &channel_7_cfg, sizeof(APP_ELEC_CHANNEL_CFG_T)},
    {ELEC_BUTTON_CHANNEL_8_TOGGLE, ELEC_BUTTON_MODE_SHORT, EVT_ELEC_CHANNEL_STATUS_SET, &channel_8_cfg, sizeof(APP_ELEC_CHANNEL_CFG_T)},
    {ELEC_BUTTON_LOCAL_RESET, ELEC_BUTTON_MODE_LONG,  EVT_ELEC_DEV_LOCAL_REMOVE, NULL, 0},
};

/***********************************************************
***********************function define**********************
***********************************************************/
#if (defined(ENABLE_ELEC_CHILD_LOCK) && (ENABLE_ELEC_CHILD_LOCK==1))
STATIC VOID_T __child_lock_button_cb(TIMER_ID timer_id, VOID_T *arg)
{
    __child_unlock_cnt = 0;
    TAL_PR_DEBUG("child lock timer cnt clear");
    return;
}
#endif

/**
 * @brief       检测到有效按键后，触发相应事件
 *
 * @param[in] :    mode     按键模式
 * @param[in] :    fun      按键功能
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T ty_app_elec_button_trigger(ELEC_BUTTON_MODE_E mode, ELEC_BUTTON_FUNC_E fun)
{
    OPERATE_RET rt = OPRT_OK;
    UINT_T i = 0;

    for (i=0; i<CNTSOF(c_BUTTON_EVT_LIST); i++) {
        if ((mode & c_BUTTON_EVT_LIST[i].mode) && fun == c_BUTTON_EVT_LIST[i].fun) {
            TAL_PR_NOTICE("button trigger mode:%u fun:%u evt:0x%x", mode, fun, c_BUTTON_EVT_LIST[i].evt_id);
#if (defined(ENABLE_ELEC_CHILD_LOCK) && (ENABLE_ELEC_CHILD_LOCK==1))
            CHILD_LOCK_STATUS_E child_lock_status = STATUS_UNLOCK;
            child_lock_status = app_elec_child_lock_status_get();
            if (child_lock_status == STATUS_LOCK && EVT_ELEC_DEV_LOCAL_REMOVE != c_BUTTON_EVT_LIST[i].evt_id) {
                TAL_PR_DEBUG("child lock");
                if (NULL == __child_lock_tm_id) {
                    __child_unlock_cnt = 0;
                    tal_sw_timer_create(__child_lock_button_cb, NULL, &__child_lock_tm_id);
                }
                if (FALSE == tal_sw_timer_is_running(__child_lock_tm_id)) {
                    tal_sw_timer_start(__child_lock_tm_id, 5000, TAL_TIMER_ONCE);
                }
                __child_unlock_cnt++;
                if (__child_unlock_cnt >= CHILD_UNLOCK_CNT) {
                    __child_unlock_cnt = 0;
                    tal_sw_timer_stop(__child_lock_tm_id);
                    TUYA_CALL_ERR_LOG(ty_app_event_post_synchronous(APP_EVT_GROUP_ELE, EVT_ELEC_CHILD_LOCK_DISABLE, NULL, 0));
                }
                return;
            }
#endif
            TUYA_CALL_ERR_LOG(ty_app_event_post(APP_EVT_GROUP_ELE, c_BUTTON_EVT_LIST[i].evt_id, c_BUTTON_EVT_LIST[i].evt_data, c_BUTTON_EVT_LIST[i].evt_data_len));
            return;
        }
    }

    return;
}

/**
 * @brief       产测模式下的按键回调
 *
 * @param[in] :    mode     按键模式
 * @param[in] :    fun      按键功能
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T ty_app_elec_button_factory_trigger(ELEC_BUTTON_MODE_E mode, ELEC_BUTTON_FUNC_E fun)
{
    OPERATE_RET rt = OPRT_OK;
    UINT8_T i = 0;
    ELEC_NET_INDICATE_MODE_E indicate_mode = 0;
    UINT8_T led_num  = 0;
    UINT8_T chan_num = 0;

    //获取 LED，通道个数
    TUYA_CALL_ERR_LOG(app_elec_channel_config(ELEC_CHANNEL_NUM_GET, &chan_num));

    led_num = app_elec_channel_led_num_get();
    if (app_elec_net_led_mux_get()) {
        led_num += 1;
    } else {
        led_num += 2;
    }

    TAL_PR_DEBUG("channel number: %d, led number: %d", chan_num, led_num);

    if (1 < chan_num && 1 == led_num) {
        // 插排，只有一个 LED，WIFI 指示灯常量
        indicate_mode = INDICATE_MODE_ALWAYS_ON;
        TUYA_CALL_ERR_LOG(ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_PT_NET_LED_SET, &indicate_mode, SIZEOF(ELEC_NET_INDICATE_MODE_E)));
    } else {
        indicate_mode = INDICATE_MODE_PT_NOT_ENERGY_BUTTON;
        TUYA_CALL_ERR_LOG(ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_PT_NET_LED_SET, &indicate_mode, SIZEOF(ELEC_NET_INDICATE_MODE_E)));
    }

    for (i=0; i<CNTSOF(c_BUTTON_EVT_LIST); i++) {
        if ((mode & c_BUTTON_EVT_LIST[i].mode) && fun == c_BUTTON_EVT_LIST[i].fun) {
            TUYA_CALL_ERR_LOG(ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_PT_NOT_CHAN_REVERSE, c_BUTTON_EVT_LIST[i].evt_data, c_BUTTON_EVT_LIST[i].evt_data_len));
            return;
        }
    }

    return;
}
