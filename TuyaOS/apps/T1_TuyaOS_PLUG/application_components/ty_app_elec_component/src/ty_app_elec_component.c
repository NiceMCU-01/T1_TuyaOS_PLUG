/**
 * @file ty_app_elec_component.c
 * @author www.tuya.com
 * @brief ty_app_elec_component module is used to 
 * @version 0.1
 * @date 2023-04-17
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_app_config.h"

#include "ty_sys.h"

#include "ty_app_elec_event_code.h"
#include "ty_app_elec_event.h"

#include "ty_app_elec_hardware.h"

// OEM
#if defined(ENABLE_TY_LOAD_OEM_PAR) && (ENABLE_TY_LOAD_OEM_PAR)
#include "ty_app_elec_oem_config.h"
#endif

// 按键
#include "ty_app_elec_trigger.h"

// 通道
#include "app_elec_channel.h"

// 配网/总控指示灯
#include "app_elec_led.h"

// 点动开关
#if (defined(ENABLE_ELEC_DELAY_OFF_TIMER) && (ENABLE_ELEC_DELAY_OFF_TIMER==1))
#include "app_elec_delay_off_timer.h"
#endif /* ENABLE_ELEC_DELAY_OFF_TIMER */

// 童锁
#include "app_elec_child_lock.h"

// UART AI
#if defined(ENABLE_UART_AI) && (ENABLE_UART_AI==1)
#include "app_uart_ai.h"
#endif

#if (defined(ENABLE_ELEC_RANDOM_TIMER) && (ENABLE_ELEC_RANDOM_TIMER==1))
#include "app_elec_timer_random.h"
#endif /* ENABLE_ELEC_RANDOM_TIMER */

#if (defined(ENABLE_ELEC_CYCLE_TIMER) && (ENABLE_ELEC_CYCLE_TIMER==1))
#include "app_elec_timer_cycle.h"
#endif /* ENABLE_ELEC_CYCLE_TIMER */

#if (defined(ENABLE_ELEC_COUNTDOWN_TIMER) && (ENABLE_ELEC_COUNTDOWN_TIMER==1))
#include "app_elec_timer_countdown.h"
#endif /* ENABLE_ELEC_COUNTDOWN_TIMER */

// 电量统计
#if (defined(ENERGY_MONITOR_ENABLE) && (ENERGY_MONITOR_ENABLE==1))
#include "app_elec_energy_monitor.h"
#endif /* ENERGY_MONITOR_ENABLE */

#if (defined(ENABLE_ENERGY_OVERCHARGE) && (ENABLE_ENERGY_OVERCHARGE==1))
#include "app_elec_overcharge.h"
#endif /* ENABLE_ENERGY_OVERCHARGE */

#include "ty_app_elec_factory.h"
#if (defined(ELEC_JSON_UPGRADE_EN) && (ELEC_JSON_UPGRADE_EN==1))
#include "ty_app_elec_json_upgrade.h"
#endif
#if defined(ENABLE_NVS_STORAGE) && (ENABLE_NVS_STORAGE)
#include "tbs_nvs.h"
#define NVS_APP_MAX_LENGTH 1024
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
STATIC TIMER_ID sg_show_heap_timer_id = NULL, sg_senior_tm_id = NULL;
STATIC BOOL_T sg_device_init_flag = FALSE;
/***********************************************************
***********************function define**********************
***********************************************************/

/**
 * @brief  local reset
 *
 * @param  none 
 * 
 * @return none
 */
STATIC VOID __ty_app_show_heap_size(TIMER_ID timerID, VOID_T *pTimerArg)
{
    TAL_PR_DEBUG("heapsize :%d byte", tal_system_get_free_heap_size());
}

/**
 * @brief  local reset
 *
 * @param  none 
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
STATIC OPERATE_RET ty_app_elec_local_reset(VOID)
{
    return ty_app_event_post_synchronous(APP_EVT_GROUP_ELE, EVT_ELEC_DEV_LOCAL_REMOVE, NULL, 0);
}

STATIC OPERATE_RET ty_app_elec_start(VOID *data)
{
    OPERATE_RET rt = OPRT_OK;
    UINT_T chan_status = 0;

    tal_sw_timer_init(); //3.6.0 及以上框架中软件定时器初始化顺序提前了，适配 3.6.0 及以上框架时可删除。
#if defined(ENABLE_TY_LOAD_OEM_PAR) && (ENABLE_TY_LOAD_OEM_PAR)
    TUYA_CALL_ERR_RETURN(ty_app_elec_oem_config_init());
#endif

    /* 硬件注册 */
#if defined(ENABLE_TY_LOAD_OEM_PAR) && (ENABLE_TY_LOAD_OEM_PAR)
    TUYA_CALL_ERR_RETURN(ty_app_elec_oem_hardware_reg());
#else
    TUYA_CALL_ERR_RETURN(ty_app_elec_hardware_reg());
#endif

    /* 网络指示灯初始化 */
    TUYA_CALL_ERR_LOG(app_elec_net_led_init());

    /* 通道功能初始化 */
    TUYA_CALL_ERR_LOG(app_elec_channel_init());

    // 主控灯初始化
    TUYA_CALL_ERR_LOG(app_elec_power_led_init());
    TUYA_CALL_ERR_LOG(app_elec_get_all_channel_status(&chan_status));
    TUYA_CALL_ERR_LOG(app_elec_power_led_status_set(chan_status));
    // 更新通道指示灯状态
    app_elec_channel_led_refresh();

#if defined(ENABLE_TY_LOAD_OEM_PAR) && (ENABLE_TY_LOAD_OEM_PAR)
    ty_app_elec_oem_release();
#endif

#if (defined(ENERGY_MONITOR_ENABLE) && (ENERGY_MONITOR_ENABLE==1))
    // 关闭 WiFi 低功耗模式，避免计量采样错误，for ECR6600
    tal_wifi_lp_disable();
#endif

    return rt;
}

/**
 * @brief SDK 初始化前的预初始化 注册快速启动等 SDK 事件
 *
 * @param none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_pre_sdk_init(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    /* 应用事件注册 */
    TUYA_CALL_ERR_RETURN(ty_app_event_register(APP_EVT_GROUP_ELE, ty_app_event_callback));

    TAL_PR_DEBUG("event_register success");

    /*关闭产测超时检测标志，仅供研发调试使用*/
    // ty_sys_ignore_close_factory_test();
#if defined(ENABLE_NVS_STORAGE) && (ENABLE_NVS_STORAGE)
    TUYA_CALL_ERR_RETURN(tbs_nvs_init(NVS_STORAGE_START_ADDRESS, NVS_STORAGE_END_ADDRESS));
    tbs_nvs_set_len_max(NVS_APP_MAX_LENGTH);
#endif
#if defined(ENABLE_TY_LOAD_OEM_PAR) && (ENABLE_TY_LOAD_OEM_PAR)
    ty_subscribe_event(EVENT_SDK_EARLY_INIT_OK, "oem_start", ty_app_elec_oem_config_cache_init ,SUBSCRIBE_TYPE_EMERGENCY);
#endif
 
    ty_subscribe_event(EVENT_APP_MF_INIT_SUCC, "app_elec_start", ty_app_elec_start, SUBSCRIBE_TYPE_ONETIME);

    return rt;
}

STATIC VOID_T __ty_app_senior_timer_init_cb(TIMER_ID timerID, VOID_T *pTimerArg)
{
    OPERATE_RET rt = OPRT_OK;

    // 判断时间是否经过同步
    STATIC UINT32_T retry_cnt = 0;
    STATIC UINT32_T last_log_ms = 0;
    UINT32_T now_ms = 0;

    rt = tal_time_check_time_sync();
    if (OPRT_OK != rt) {
        retry_cnt++;
        now_ms = tal_system_get_millisecond();
        if (0 == last_log_ms || (now_ms - last_log_ms) >= (60 * 1000)) {
            last_log_ms = now_ms;
            TAL_PR_NOTICE("time not sync, senior timer init deferred, retry_cnt: %u", retry_cnt);
        }
        return;
    }
    retry_cnt = 0;
    last_log_ms = 0;
    TAL_PR_DEBUG("time synchronized, init senior timer");

    /* 随机定时 */
#if (defined(ENABLE_ELEC_RANDOM_TIMER) && (ENABLE_ELEC_RANDOM_TIMER==1))
    TUYA_CALL_ERR_LOG(app_elec_random_timer_init(ty_app_elec_random_timer_trigger));
#endif

    /* 循环定时 */
#if (defined(ENABLE_ELEC_CYCLE_TIMER) && (ENABLE_ELEC_CYCLE_TIMER==1))
    TUYA_CALL_ERR_LOG(app_elec_cycle_timer_init(ty_app_elec_cycle_timer_trigger));
#endif

    tal_sw_timer_stop(timerID);
    tal_sw_timer_delete(timerID);

    return;
}

/**
 * @brief SDK 初始化前的预初始化 注册快速启动等 SDK 事件
 *
 * @param none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_component_init(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    UINT_T chan_status = 0;
    UINT8_T chan_num = 0;

    // 如果进入产测，则不往下继续进行
    ELEC_PROD_TEST_STATUS_T prod_test_status = PROD_NOT_TESTED;
    prod_test_status = ty_app_elec_prod_test_status_get();
    while (PROD_TESTING == prod_test_status) {
        TAL_PR_DEBUG("produce testing...");
        tal_system_sleep(30 * 1000);
    }

#if defined(ENABLE_UART_AI) && (ENABLE_UART_AI==1)
    extern OPERATE_RET tuya_uart_ai_init(IN tuya_uart_ai_cfg_t *cfg);
    
    // UART AI配置参数
    tuya_uart_ai_cfg_t uart_ai_cfg = {
        .spkr_flow_pin = 6,           // 串口流控IO引脚，默认6
        .gx8006_power_pin = 24,       // 语音芯片供电控制引脚，默认24
        .gx8006_boot_pin = 8,         // 语音芯片BOOT引脚，默认8
        .enable_deep_sleep_mode = FALSE, // 深度睡眠模式，默认关闭
        .uart_ai_volume_dp = 101,     // 串口AI音量DP，默认101
        .enable_led_show = FALSE,     // LED显示功能，默认关闭
        .led_show_name = "led_show",  // LED显示名称
        .led_show_pin = 7,            // LED显示引脚，默认7
        .led_show_pin_level = 0       // LED显示引脚电平，默认低电平点亮
    };
    
    tuya_uart_ai_init(&uart_ai_cfg);
#endif

    TUYA_CALL_ERR_LOG(app_elec_get_all_channel_status(&chan_status));
    TUYA_CALL_ERR_LOG(app_elec_channel_config(ELEC_CHANNEL_NUM_GET, &chan_num));

    // 电量统计，过充，故障功能初始化
#if (defined(ENERGY_MONITOR_ENABLE) && (ENERGY_MONITOR_ENABLE==1))
    TUYA_CALL_ERR_LOG(app_elec_energy_monitor_init());
    if (OPRT_OK == rt) {
#if (defined(ENABLE_ENERGY_OVERCHARGE) && (ENABLE_ENERGY_OVERCHARGE==1))
        // 有计量功能，并且为 1 路插座才会有过充保护功能
        if (1 == chan_num) {
            TUYA_CALL_ERR_LOG(app_elec_overcharge_init(ty_app_elec_overcharge_trigger));
            app_elec_overcharge_channel_sync(chan_status);
            if (0 != chan_status) {
                TUYA_CALL_ERR_LOG(app_elec_overcharge_start());
            }
        }
#endif /* ENABLE_ENERGY_OVERCHARGE */
#if (defined(ENABLE_ENERGY_FAULT) && (ENABLE_ENERGY_FAULT==1))
        TUYA_CALL_ERR_LOG(app_elec_fault_init(ty_app_elec_fault_trigger));
#endif /* ENABLE_ENERGY_FAULT */
    }
#endif

    /* 按键功能初始化 */
    TUYA_CALL_ERR_LOG(app_elec_button_init(ty_app_elec_button_trigger));

    /* 点动开关服务初始化 */
#if (defined(ENABLE_ELEC_DELAY_OFF_TIMER) && (ENABLE_ELEC_DELAY_OFF_TIMER==1))
    TUYA_CALL_ERR_LOG(app_elec_delay_off_init(ty_app_elec_delay_off_trigger));
#endif

    /* 童锁服务初始化 */
#if (defined(ENABLE_ELEC_CHILD_LOCK) && (ENABLE_ELEC_CHILD_LOCK==1))
    TUYA_CALL_ERR_LOG(app_elec_child_lock_init(NULL, ty_app_elec_child_lock_trigger));
#endif

    /* 倒计时 */
#if (defined(ENABLE_ELEC_COUNTDOWN_TIMER) && (ENABLE_ELEC_COUNTDOWN_TIMER==1))
    TUYA_CALL_ERR_LOG(app_elec_countdown_time_init(ty_app_elec_time_countdown_trigger));
#endif
    /* json升级初始化 */
#if (defined(ELEC_JSON_UPGRADE_EN) && (ELEC_JSON_UPGRADE_EN==1))
    TUYA_CALL_ERR_LOG(app_json_upgrade_init());
#endif
    // 高级定时需要时间同步后在进行初始化
    TUYA_CALL_ERR_RETURN(tal_sw_timer_create(__ty_app_senior_timer_init_cb, NULL, &sg_senior_tm_id));
    tal_sw_timer_start(sg_senior_tm_id, 5 * 1000, TAL_TIMER_CYCLE);

    TUYA_CALL_ERR_RETURN(tal_sw_timer_create(__ty_app_show_heap_size, NULL, &sg_show_heap_timer_id));
    tal_sw_timer_start(sg_show_heap_timer_id, 10 * 1000, TAL_TIMER_CYCLE);

    sg_device_init_flag = TRUE;

    return rt;
}

/**
 * @brief 获取设备初始化标志        
 *
 * @param[in] : VOID_T
 *
 * @return BOOL_T
 */
BOOL_T ty_app_elec_device_init_flag_get(VOID_T)
{
    return sg_device_init_flag;
}

/**
 * @brief    获取 SOC 产品配置参数
 *
 * @param[in] : prod    产品配置
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_get_soc_product_cfg(OUT TY_WIFI_SOC_PROD_CFG_S *prod)
{
    if(NULL == prod) {
        return OPRT_COM_ERROR;
    }

    prod->start_mode   = TY_WIFI_START_MODE;
    prod->firmware_key = TY_FIRMWARE_KEY;

#if defined(ENABLE_TY_LOAD_OEM_PAR) && (ENABLE_TY_LOAD_OEM_PAR)
    prod->wf_cfg        = ty_app_elec_oem_get_wf_cfg();
    prod->wf_cfg_tm_s   = ty_app_elec_oem_get_wf_cfg_time();
#else  
    prod->wf_cfg        = TY_WIFI_CFG_MTHD;
    prod->wf_cfg_tm_s   = TY_NET_CFG_TIME;
#endif

#if defined(ENABLE_TY_FIRMWARE_OEM) && (ENABLE_TY_FIRMWARE_OEM)
    prod->product_id    = TY_FIRMWARE_KEY;
#else
    /* Public source does not contain a product PID. Configure an authorized
     * TY_PRODUCT_ID in the consumer's private TuyaOS build configuration. */
    prod->product_id    = TY_PRODUCT_ID;
#endif

    return OPRT_OK;
}
