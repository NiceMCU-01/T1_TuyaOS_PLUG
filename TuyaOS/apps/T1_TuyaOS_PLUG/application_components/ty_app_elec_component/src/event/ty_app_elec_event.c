/**
 * @file ty_app_elec_event.c
 * @author www.tuya.com
 * @brief ty_app_elec_event module is used to 
 * @version 0.1
 * @date 2023-04-17
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_app_config.h"

#include "ty_sys.h"
#include "ty_app_starts_up_intf.h"
#include "ty_app_elec_event.h"
#include "ty_app_elec_factory.h"

#include "tal_log.h"

#include "app_elec_channel.h"
#include "app_elec_led.h"
#include "app_elec_delay_off_timer.h"
#include "app_elec_child_lock.h"
#include "app_elec_timer_random.h"

#if (defined(ENABLE_ELEC_COUNTDOWN_TIMER) && (ENABLE_ELEC_COUNTDOWN_TIMER==1))
#include "app_elec_timer_countdown.h"
#endif

#include "app_elec_timer_cycle.h"

#include "app_elec_energy_monitor.h"

#include "app_elec_overcharge.h"
#include "app_elec_fault.h"

#if defined(ENABLE_UART_AI) && (ENABLE_UART_AI==1)
#include "tuya_player.h"
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
STATIC VOID_T __ty_app_channel_set(APP_ELEC_CHANNEL_CFG_T *chan_cfg)
{
    OPERATE_RET rt = OPRT_OK;
    UINT8_T chan_num = 0;
    UINT8_T i = 0;
    UINT_T cur_chan_status = 0, last_chan_status = 0;

    TUYA_CHECK_NULL_GOTO(chan_cfg, __EXIT);
    TAL_PR_NOTICE("channel set req chan:%u status:%u", chan_cfg->chan_id, chan_cfg->status);

#if (defined(ENABLE_ENERGY_FAULT) && (ENABLE_ENERGY_FAULT==1))
    APP_ELEC_FAULT_T fault_value = 0;
    // 判断是否触发错误
    fault_value = app_elec_fault_value_get();
    if (0 != fault_value && STATE_OFF != chan_cfg->status) {
        TAL_PR_NOTICE("fault: 0x%04x, return", fault_value);
        return;
    }
#endif

    app_elec_get_all_channel_status(&last_chan_status);

    // 设置通道状态
    TUYA_CALL_ERR_LOG(app_elec_channel_status_set(chan_cfg));

    app_elec_get_all_channel_status(&cur_chan_status);
    TUYA_CALL_ERR_GOTO(app_elec_channel_config(ELEC_CHANNEL_NUM_GET, &chan_num), __EXIT);

// 倒计时
#if (defined(ENABLE_ELEC_COUNTDOWN_TIMER) && (ENABLE_ELEC_COUNTDOWN_TIMER==1))
    UINT8_T cur_s = 0, last_s = 0;
    UINT32_T remain_s = 0;
    APP_ELEC_COUNTDOWN_CFG_T cd_cfg = {
        .id = chan_cfg->chan_id,
        .time_s = 0,
    };
    if (chan_cfg->chan_id == 0) {
        for (i=0; i<chan_num; i++) {
            cur_s = (cur_chan_status & (0x00000001<<i));
            last_s = (last_chan_status & (0x00000001<<i));
            if (cur_s != last_s) {
                cd_cfg.id = i+1;
                app_elec_countdown_time_set(cd_cfg);
            }
        }
    } else {
        cur_s = (cur_chan_status & (0x00000001<<(chan_cfg->chan_id-1)));
        last_s = (last_chan_status & (0x00000001<<(chan_cfg->chan_id-1)));
        app_elec_countdown_time_remain_sec_get(chan_cfg->chan_id, &remain_s);
        if (cur_s != last_s && remain_s > 0) {
            app_elec_countdown_time_set(cd_cfg);
        }
    }
#endif /* ENABLE_ELEC_COUNTDOWN_TIMER */

    // 点动开关设置
#if (defined(ENABLE_ELEC_DELAY_OFF_TIMER) && (ENABLE_ELEC_DELAY_OFF_TIMER==1))
    if (chan_cfg->chan_id == 0) {
        
        for (i=0; i<chan_num; i++) {
            if (STATE_ON == app_elec_channel_status_get(i+1)) {
                TUYA_CALL_ERR_LOG(app_elec_delay_off_start(i+1));
            }
        }
        TUYA_CALL_ERR_LOG(app_elec_all_channel_status_upload());
    } else {
        TAL_PR_DEBUG("single chan_cfg->chan_id : %d", chan_cfg->chan_id);
        if (STATE_ON == app_elec_channel_status_get(chan_cfg->chan_id)) {
            TUYA_CALL_ERR_LOG(app_elec_delay_off_start(chan_cfg->chan_id));
        }
    }
#endif /* ENABLE_ELEC_DELAY_OFF_TIMER */

    // 总控指示灯状态设置
    TAL_PR_DEBUG("---> chan status: 0x%x", cur_chan_status);
    app_elec_power_led_status_set(cur_chan_status);

__EXIT:
    app_elec_channel_status_upload(chan_cfg->chan_id);
#if defined (ELEC_RUNTIME_SWITCH_EN) && (ELEC_RUNTIME_SWITCH_EN == 1)
    // === 新增：运行时长开关DP同步上报 ===
    app_run_time_switch_upload(chan_cfg->chan_id);
#endif
#if (defined(ENABLE_ENERGY_OVERCHARGE) && (ENABLE_ENERGY_OVERCHARGE==1))
    app_elec_overcharge_channel_sync(cur_chan_status);
    if(chan_cfg->status){
        app_elec_overcharge_power_get();
    }
#endif 
    return;
}

STATIC VOID_T __ty_app_all_dp_upload(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

    TUYA_CALL_ERR_LOG(app_elec_all_channel_status_upload());
    TUYA_CALL_ERR_LOG(app_elec_channel_mode_upload());
    TUYA_CALL_ERR_LOG(app_elec_power_led_mode_upload());

#if (defined(ENABLE_ELEC_CHILD_LOCK) && (ENABLE_ELEC_CHILD_LOCK==1))
    TUYA_CALL_ERR_LOG(app_elec_child_lock_dp_data_upload());
#endif 
#if (defined(ENABLE_ELEC_DELAY_OFF_TIMER) && (ENABLE_ELEC_DELAY_OFF_TIMER==1))
    TUYA_CALL_ERR_LOG(app_elec_delay_off_dp_data_upload());
#endif
#if (defined(ENABLE_ELEC_RANDOM_TIMER) && (ENABLE_ELEC_RANDOM_TIMER==1))
    TUYA_CALL_ERR_LOG(app_elec_random_timer_upload());
#endif
#if (defined(ENABLE_ELEC_CYCLE_TIMER) && (ENABLE_ELEC_CYCLE_TIMER==1))
    TUYA_CALL_ERR_LOG(app_elec_cycle_timer_upload());
#endif
#if (defined(ENABLE_ELEC_COUNTDOWN_TIMER) && (ENABLE_ELEC_COUNTDOWN_TIMER==1))
    TUYA_CALL_ERR_LOG(app_elec_countdown_time_all_upload());
#endif
#if (defined(ENERGY_MONITOR_ENABLE) && (ENERGY_MONITOR_ENABLE==1))
    TUYA_CALL_ERR_LOG(app_elec_energy_monitor_coe_upload());
    TUYA_CALL_ERR_LOG(app_elec_energy_monitor_pvi_upload());
    app_elec_energy_monitor_add_energy_upload();
#endif
#if (defined(ENABLE_ENERGY_OVERCHARGE) && (ENABLE_ENERGY_OVERCHARGE==1))
    TUYA_CALL_ERR_LOG(app_elec_overcharge_dp_upload());
#endif
#if (defined(ENABLE_ENERGY_FAULT) && (ENABLE_ENERGY_FAULT==1))
    TUYA_CALL_ERR_LOG(app_elec_fault_upload());
#endif

    return;
}

STATIC VOID_T __ty_app_dp_upload(UINT8_T dpid)
{
    OPERATE_RET rt = OPRT_OK;

    switch (dpid) {
#if (defined(ENERGY_MONITOR_ENABLE) && (ENERGY_MONITOR_ENABLE==1))
        case ENERGY_MONITOR_CUR_VOLTAGE_DPID :
        case ENERGY_MONITOR_CUR_CURRENT_DPID :
        case ENERGY_MONITOR_CUR_POWER_DPID : {
            TUYA_CALL_ERR_LOG(app_elec_energy_monitor_pvi_upload());
        } break;
        case ENERGY_MONITOR_TEST_RESULT_DPID:
        case ENERGY_MONITOR_COE_VOLTAGE_DPID:
        case ENERGY_MONITOR_COE_CURRENT_DPID:
        case ENERGY_MONITOR_COE_POWER_DPID:
        case ENERGY_MONITOR_COE_ENERGY_DPID: {
            TUYA_CALL_ERR_LOG(app_elec_energy_monitor_coe_upload());
        } break;
#endif /* ENERGY_MONITOR_ENABLE */
        case ELEC_CHANNEL_POWER_ON_DPID: {
            TUYA_CALL_ERR_LOG(app_elec_channel_mode_upload());
        } break;
        case ELEC_LIGHT_MODE_DPID: {
            TUYA_CALL_ERR_LOG(app_elec_power_led_mode_upload());
        } break;
#if (defined(ENABLE_ELEC_CHILD_LOCK) && (ENABLE_ELEC_CHILD_LOCK==1))
        case ELEC_CHILD_LOCK_DPID: {
            TUYA_CALL_ERR_LOG(app_elec_child_lock_dp_data_upload());
        } break;
#endif 
#if (defined(ENABLE_ELEC_CYCLE_TIMER) && (ENABLE_ELEC_CYCLE_TIMER==1))
        case ELEC_CYCLE_TIMER_DPID: {
            TUYA_CALL_ERR_LOG(app_elec_cycle_timer_upload());
        } break;
#endif /* ENABLE_ELEC_CYCLE_TIMER */
#if (defined(ENABLE_ELEC_RANDOM_TIMER) && (ENABLE_ELEC_RANDOM_TIMER==1))
        case ELEC_RANDOM_TIMER_DPID: {
            TUYA_CALL_ERR_LOG(app_elec_random_timer_upload());
        } break;
#endif /* ENABLE_ELEC_RANDOM_TIMER */
#if (defined(ENABLE_ELEC_DELAY_OFF_TIMER) && (ENABLE_ELEC_DELAY_OFF_TIMER==1))
        case ELEC_DELAY_OFF_DPID: {
            TUYA_CALL_ERR_LOG(app_elec_delay_off_dp_data_upload());
        } break;
#endif /* ENABLE_ELEC_DELAY_OFF_TIMER */
#if (defined(ENABLE_ENERGY_OVERCHARGE) && (ENABLE_ENERGY_OVERCHARGE==1))
        case ENERGY_METER_OVERCHARGE_DPID: {
            TUYA_CALL_ERR_LOG(app_elec_overcharge_dp_upload());
        }
#endif /* ENABLE_ENERGY_OVERCHARGE */
#if (defined(ENABLE_ENERGY_FAULT) && (ENABLE_ENERGY_FAULT==1))
        case ENERGY_METER_FAULT_DPID: {
            TUYA_CALL_ERR_LOG(app_elec_fault_upload());
        }
#endif
        default : 
        if (dpid <= 8) {
            // 通道 DP 数据上报，通道 DP 一般为 1-8
            TUYA_CALL_ERR_LOG(app_elec_channel_status_upload_by_dpid(dpid));
        } else if (dpid <= 16) {
#if (defined(ENABLE_ELEC_COUNTDOWN_TIMER) && (ENABLE_ELEC_COUNTDOWN_TIMER==1))
            // 倒计时通道 DP 一般为 9-16
            TUYA_CALL_ERR_LOG(app_elec_countdown_time_upload_by_dpid(dpid));
#endif
        } else {
            TAL_PR_ERR("dpid: %d not upload", dpid);
        }
        break;
    }

    return;
}

STATIC VOID_T __ty_app_elec_data_erase(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

    TAL_PR_DEBUG("erase all app data");

    app_elec_channel_data_erase();
    TUYA_CALL_ERR_LOG(app_elec_power_led_mode_data_erase());
#if (defined(ENERGY_MONITOR_ENABLE) && (ENERGY_MONITOR_ENABLE==1))
    TUYA_CALL_ERR_LOG(app_elec_energy_monitor_data_easer());
#endif
#if (defined(ENABLE_ELEC_DELAY_OFF_TIMER) && (ENABLE_ELEC_DELAY_OFF_TIMER==1))
    TUYA_CALL_ERR_LOG(app_elec_delay_off_data_erase());
#endif
#if (defined(ENABLE_ELEC_CYCLE_TIMER) && (ENABLE_ELEC_CYCLE_TIMER==1))
    TUYA_CALL_ERR_LOG(app_elec_cycle_timer_memory_easer());
#endif
#if (defined(ENABLE_ELEC_RANDOM_TIMER) && (ENABLE_ELEC_RANDOM_TIMER==1))
    TUYA_CALL_ERR_LOG(app_elec_random_timer_memory_easer());
#endif
#if (defined(ENABLE_ENERGY_OVERCHARGE) && (ENABLE_ENERGY_OVERCHARGE==1))
    TUYA_CALL_ERR_LOG(app_elec_overcharge_date_easer());
#endif
#if defined (ELEC_RUNTIME_SWITCH_EN) && (ELEC_RUNTIME_SWITCH_EN == 1)
    TUYA_CALL_ERR_LOG(app_run_time_switch_data_erase());
#endif
    return;
}

/**
 * @brief        事件回调函数
 *
 * @param[in] :  event         事件信息
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T ty_app_event_callback(APP_EVENT_MSG_S *event)
{
    OPERATE_RET rt = OPRT_OK;

    if(NULL == event) {
        TAL_PR_ERR("elec event callback input is null");
        return;
    }

    TAL_PR_DEBUG("event id: 0x%X ->>>", event->event_id);

    switch (event->event_id) {
/************************ channel **************************************/
        case EVT_ELEC_CHANNEL_STATUS_SET:{
            TUYA_CHECK_NULL_GOTO(event->data, __EXIT);
            APP_ELEC_CHANNEL_CFG_T *chan_cfg = (APP_ELEC_CHANNEL_CFG_T *)event->data;
            __ty_app_channel_set(chan_cfg);
        } break;
/************************ power led ************************************/
        case EVT_ELEC_POWER_ON_MODE_SET: {
            TUYA_CHECK_NULL_GOTO(event->data, __EXIT);
            APP_CHANNEL_MODE_E chan_mode = MODE_TURN_OFF;
            memcpy(&chan_mode, event->data, sizeof(APP_CHANNEL_MODE_E));
            TUYA_CALL_ERR_LOG(app_elec_channel_mode_set(chan_mode));
            TUYA_CALL_ERR_LOG(app_elec_channel_mode_upload());
        } break;
/************************ child lock ***********************************/
#if (defined(ENABLE_ELEC_CHILD_LOCK) && (ENABLE_ELEC_CHILD_LOCK==1))
        case EVT_ELEC_CHILD_LOCK_ENABLE: {
            TUYA_CALL_ERR_LOG(app_elec_child_lock_status_set(STATUS_LOCK));
        } break;
        case EVT_ELEC_CHILD_LOCK_DISABLE: {
            TUYA_CALL_ERR_LOG(app_elec_child_lock_status_set(STATUS_UNLOCK));
        } break;
        case EVT_ELEC_CHILD_AUTO_LOCK_ENABLE : {
            TUYA_CALL_ERR_LOG(app_elec_child_auto_lock_status_set(1));
        } break;
        case EVT_ELEC_CHILD_AUTO_LOCK_DISABLE : {
            TUYA_CALL_ERR_LOG(app_elec_child_auto_lock_status_set(0));
        } break;
#endif
/************************ delay off *************************************/
#if (defined(ENABLE_ELEC_DELAY_OFF_TIMER) && (ENABLE_ELEC_DELAY_OFF_TIMER==1))
        case EVT_ELEC_DELAY_OFF_SET: {
            TUYA_CHECK_NULL_GOTO(event->data, __EXIT);
            APP_ELEC_DELAY_OFF_DATA_T *delay_off_data = (APP_ELEC_DELAY_OFF_DATA_T *)event->data;
            TUYA_CALL_ERR_LOG(app_elec_delay_off_override_set(delay_off_data));
            app_elec_delay_off_dp_data_upload();
        } break;
#endif
/************************ countdown time *******************************/
#if (defined(ENABLE_ELEC_COUNTDOWN_TIMER) && (ENABLE_ELEC_COUNTDOWN_TIMER==1))
        case EVT_ELEC_COUNTDOWN_TIME_SET: {
            TUYA_CHECK_NULL_GOTO(event->data, __EXIT);
            APP_ELEC_COUNTDOWN_CFG_T *countdown_cfg = (APP_ELEC_COUNTDOWN_CFG_T *)event->data;
            TUYA_CALL_ERR_LOG(app_elec_countdown_time_set(*countdown_cfg));
        } break;
#endif /* ENABLE_ELEC_COUNTDOWN_TIMER */
/************************ cycle time ***********************************/
#if (defined(ENABLE_ELEC_CYCLE_TIMER) && (ENABLE_ELEC_CYCLE_TIMER==1))
        case EVT_ELEC_CYCLE_TIMER_STR_SET: {
            TUYA_CHECK_NULL_GOTO(event->data, __EXIT);
            CHAR_T *cycle_tm_data = (CHAR_T *)event->data;
            TUYA_CALL_ERR_LOG(app_elec_cycle_timer_dp_data_set(cycle_tm_data));
        } break;
#endif /* ENABLE_ELEC_CYCLE_TIMER */
/************************ random time **********************************/
#if (defined(ENABLE_ELEC_RANDOM_TIMER) && (ENABLE_ELEC_RANDOM_TIMER==1))
        case EVT_ELEC_RANDOM_TIMER_STR_SET: {
            TUYA_CHECK_NULL_GOTO(event->data, __EXIT);
            CHAR_T *random_tm_data = (CHAR_T *)event->data;
            TUYA_CALL_ERR_LOG(app_elec_random_timer_dp_data_set(random_tm_data));
        } break;
#endif /* ENABLE_ELEC_RANDOM_TIMER */
/************************ data point upload *****************************/
        case EVT_ELEC_REPORT_DP_ALL: {
            if (FALSE == ty_app_elec_device_init_flag_get()) {
                TAL_PR_DEBUG("device not init");
                tal_system_sleep(20);
                ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_REPORT_DP_ALL, NULL, 0);
                break;
            }
            __ty_app_all_dp_upload();
        } break;
        case EVT_ELEC_REPORT_DP: {
            TUYA_CHECK_NULL_GOTO(event->data, __EXIT);
            UINT8_T dpid = *((UINT8_T *)event->data);
            __ty_app_dp_upload(dpid);
        } break;
/************************ data erase ************************************/
        case EVT_ELEC_DEV_DATA_ERASE: {
            __ty_app_elec_data_erase();
        } break;
/************************ light mode ************************************/
        case EVT_ELEC_LIGHT_MODE_SET: {
            TUYA_CHECK_NULL_GOTO(event->data, __EXIT);
            ELEC_LIGHT_MODE_E light_mode = *((ELEC_LIGHT_MODE_E *)event->data);
            TUYA_CALL_ERR_LOG(app_elec_power_led_mode_set(light_mode));
            app_elec_channel_led_refresh();
        } break;
/************************ overcharge ************************************/
#if (defined(ENABLE_ENERGY_OVERCHARGE) && (ENABLE_ENERGY_OVERCHARGE==1))
        case EVT_ELEC_OVERCHARGE_SET: {
            TUYA_CHECK_NULL_GOTO(event->data, __EXIT);
            BOOL_T overcharge_status = FALSE;
            memcpy(&overcharge_status, event->data, sizeof(BOOL_T));
            TUYA_CALL_ERR_LOG(app_elec_overcharge_status_set(overcharge_status));
        } break;
#endif /* ENABLE_ENERGY_OVERCHARGE */
/************************ device remove *********************************/
        case EVT_ELEC_DEV_LOCAL_REMOVE:{
#if (defined(ENABLE_ELEC_DELAY_OFF_TIMER) && (ENABLE_ELEC_DELAY_OFF_TIMER==1))
            TUYA_CALL_ERR_LOG(app_elec_delay_off_data_erase());
#endif
            tuya_iot_wf_gw_unactive();
        } break;
/************************ wifi led indicate *****************************/
        case EVT_ELEC_NET_INDICATE_NOT_CONNECT:{
            app_elec_net_led_mode_set(INDICATE_MODE_NOT_CONNECT);
        } break;
        case EVT_ELEC_NET_INDICATE_CONNECTED:{
            app_elec_net_led_mode_set(INDICATE_MODE_CONNECTED);
        } break;
        case EVT_ELEC_EZ_INDICATE:{
            app_elec_net_led_mode_set(INDICATE_MODE_EZ);
#if (defined(ENABLE_UART_AI) && (ENABLE_UART_AI==1))
            TAL_PR_DEBUG("play netconfig alert");
            ai_audio_player_alert(ALART_TYPE_NETWORK_CFG);
#endif
        } break;
        case EVT_ELEC_AP_INDICATE:{
            app_elec_net_led_mode_set(INDICATE_MODE_AP);
#if (defined(ENABLE_UART_AI) && (ENABLE_UART_AI==1))
            TAL_PR_DEBUG("play netconfig alert");
            ai_audio_player_alert(ALART_TYPE_NETWORK_CFG);
#endif
        } break;
/************************ product test **********************************/
        case EVT_ELEC_PROD_UNAUTHOR:
        case EVT_ELEC_PROD_WEAK_SIGNAL: {
            app_elec_net_led_mode_set(INDICATE_MODE_WEAK_SIGNAL);
        } break;
        case EVT_ELEC_PROD_TEST_1:
        case EVT_ELEC_PROD_TEST_2:{
#if (defined(ENERGY_MONITOR_ENABLE) && (ENERGY_MONITOR_ENABLE==1))
        if (app_elec_energy_monitor_enable_get()) {
            // 计量产测
            APP_ELEC_CHANNEL_CFG_T chan_cfg = {.chan_id = 0, .status = STATE_ON};
            __ty_app_channel_set(&chan_cfg);
            app_elec_net_led_mode_set(INDICATE_MODE_ENERGY_MONITOR);
            tal_system_sleep(1500);
            rt = app_elec_energy_monitor_prod_test();
            if (OPRT_OK == rt) {
                TAL_PR_NOTICE("---> product test success <---");
                app_elec_net_led_mode_set(INDICATE_MODE_PROD_TEST_SUCCESS);
                __ty_app_elec_data_erase();
            } else {
                TAL_PR_NOTICE("---> product test fail <---");
                app_elec_net_led_mode_set(INDICATE_MODE_PROD_TEST_FAIL);
            }
            chan_cfg.status = STATE_OFF;
            __ty_app_channel_set(&chan_cfg);
        } else {
#endif
            // 非计量产测下，网络指示灯指示模式
#if defined(ENABLE_PRODUCT_TEST_SCAN_WIFI) && (ENABLE_PRODUCT_TEST_SCAN_WIFI)
            TY_WIFI_TEST_SCAN_INFO_T wifi_info = {0};
            ty_app_get_product_test_scan_wifi(&wifi_info);
            if (wifi_info.wf_cfg_mthd == GWCM_LOW_POWER || wifi_info.wf_cfg_mthd == GWCM_SPCL_MODE) {
                // 长按配网，快速闪烁，250ms-250ms
                app_elec_net_led_mode_set(INDICATE_MODE_PT_NOT_ENERGY_LOW_POWER);
            } else {
                // 上电配网，慢闪，3s-3s
                app_elec_net_led_mode_set(INDICATE_MODE_PT_NOT_ENERGY_AUTO_CFG);
            }
#endif
#if (defined(ENERGY_MONITOR_ENABLE) && (ENERGY_MONITOR_ENABLE==1))
        }
#endif
        } break;
        case EVT_ELEC_PT_NET_LED_SET: {
            TUYA_CHECK_NULL_GOTO(event->data, __EXIT);
            ELEC_NET_INDICATE_MODE_E *indicate_mode = (ELEC_NET_INDICATE_MODE_E *)event->data;
            app_elec_net_led_mode_set(*indicate_mode);
        } break;
        case EVT_ELEC_PT_NOT_CHAN_REVERSE : {
            TUYA_CHECK_NULL_GOTO(event->data, __EXIT);
            APP_ELEC_CHANNEL_CFG_T *chan_cfg = (APP_ELEC_CHANNEL_CFG_T *)event->data;
            ty_app_elec_not_energy_prod_test(chan_cfg);
        } break;
        case EVT_ELEC_PT_DATA_ERASE : {
#if (defined(ENERGY_MONITOR_ENABLE) && (ENERGY_MONITOR_ENABLE==1))
            // 产测数据擦除
            ty_app_elec_energy_monitor_cal_data_erase();
#endif
            // 应用数据擦除
            __ty_app_elec_data_erase();
        } break;
#if defined (ELEC_RUNTIME_SWITCH_EN) && (ELEC_RUNTIME_SWITCH_EN == 1)
        case EVT_ELEC_RUNTIME_SWITCH_SET : {
            TAL_PR_NOTICE("---> device online check cache switch <---");
            app_run_time_switch_local_upload();
        }break;
#endif
        default:
            break;
    }

__EXIT:
    return;
}
