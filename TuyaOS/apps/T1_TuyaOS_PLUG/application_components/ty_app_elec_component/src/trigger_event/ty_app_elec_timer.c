/**
 * @file ty_app_elec_timer.c
 * @author www.tuya.com
 * @brief ty_app_elec_timer module is used to 
 * @version 0.1
 * @date 2023-05-05
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_app_config.h"
#include "ty_sys.h"

#include "ty_app_elec_event_code.h"

#include "app_elec_channel.h"

#if (defined(ENABLE_ELEC_COUNTDOWN_TIMER) && (ENABLE_ELEC_COUNTDOWN_TIMER==1))
#include "app_elec_timer_countdown.h"
#endif

#if (defined(ENABLE_ELEC_CYCLE_TIMER) && (ENABLE_ELEC_CYCLE_TIMER==1))
#include "app_elec_timer_cycle.h"
#endif

#if (defined(ENABLE_ELEC_RANDOM_TIMER) && (ENABLE_ELEC_RANDOM_TIMER==1))
#include "app_elec_timer_random.h"
#endif

#if (defined(ENABLE_ELEC_DELAY_OFF_TIMER) && (ENABLE_ELEC_DELAY_OFF_TIMER==1))
#include "app_elec_delay_off_timer.h"
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


/***********************************************************
***********************variable define**********************
***********************************************************/


/***********************************************************
***********************function define**********************
***********************************************************/

#if (defined(ENABLE_ELEC_COUNTDOWN_TIMER) && (ENABLE_ELEC_COUNTDOWN_TIMER==1))
/**
 * @brief 倒计时出发回调
 *
 * @param[in] countdown_id: 倒计时 ID
 *
 * @return none
 */
VOID_T ty_app_elec_time_countdown_trigger(UINT8_T countdown_id)
{
    TAL_PR_DEBUG("countdown: %d", countdown_id);

    APP_ELEC_CHANNEL_CFG_T chan_cfg = {0};
    chan_cfg.chan_id = countdown_id;
    chan_cfg.status = STATE_TOGGLE;
    ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_CHANNEL_STATUS_SET, &chan_cfg, SIZEOF(APP_ELEC_CHANNEL_CFG_T));

    return;
}
#endif

#if (defined(ENABLE_ELEC_CYCLE_TIMER) && (ENABLE_ELEC_CYCLE_TIMER==1))
/**
 * @brief 循环定时触发回调
 *
 * @param[in] channel_id: 通道 ID
 * @param[in] state: 通道状态
 *
 * @return none
 */
VOID_T ty_app_elec_cycle_timer_trigger(UINT8_T channel_id, BOOL_T state)
{
    APP_ELEC_CHANNEL_CFG_T chan_cfg = {0};

    TAL_PR_NOTICE("ct, chan_id: %d, sta: %d", channel_id, state);

    chan_cfg.chan_id = channel_id;
    chan_cfg.status = (TRUE == state) ? (STATE_ON) : (STATE_OFF);
    ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_CHANNEL_STATUS_SET, &chan_cfg, SIZEOF(APP_ELEC_CHANNEL_CFG_T));

    return;
}
#endif

#if (defined(ENABLE_ELEC_RANDOM_TIMER) && (ENABLE_ELEC_RANDOM_TIMER==1))
/**
 * @brief 随机定时触发回调
 *
 * @param[in] channel_id: 通道 ID
 * @param[in] state: 通道状态
 *
 * @return none
 */
VOID_T ty_app_elec_random_timer_trigger(UINT8_T channel_id, RANDOM_STATE_E state)
{
    APP_ELEC_CHANNEL_CFG_T chan_cfg = {0};

    TAL_PR_DEBUG("random timer, channel id: %d, status: %d", channel_id, state);

    chan_cfg.chan_id = channel_id;
    chan_cfg.status = (RANDOM_STATE_START == state) ? (STATE_ON) : (STATE_OFF);
    ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_CHANNEL_STATUS_SET, &chan_cfg, SIZEOF(APP_ELEC_CHANNEL_CFG_T));

    return;
}
#endif

#if (defined(ENABLE_ELEC_DELAY_OFF_TIMER) && (ENABLE_ELEC_DELAY_OFF_TIMER==1))
VOID_T ty_app_elec_delay_off_trigger(UINT8_T channel_id)
{
    TAL_PR_DEBUG("delay off cb, id: %d", channel_id);

    APP_ELEC_CHANNEL_CFG_T chan_cfg = {0};
    chan_cfg.chan_id = channel_id;
    chan_cfg.status = STATE_OFF;
    ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_CHANNEL_STATUS_SET, &chan_cfg, SIZEOF(APP_ELEC_CHANNEL_CFG_T));

    return;
}
#endif
