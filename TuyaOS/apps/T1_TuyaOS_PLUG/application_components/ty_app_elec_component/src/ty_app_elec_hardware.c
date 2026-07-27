/**
 * @file ty_app_elec_hardware.c
 * @author www.tuya.com
 * @brief ty_app_elec_hardware module is used to 
 * @version 0.1
 * @date 2023-03-20
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_app_config.h"
#include "ty_sys.h"

#include "ty_app_elec_hardware.h"

#include "app_elec_channel.h"

#include "tdd_button_gpio.h"
#include "tdd_led_gpio_driver.h"

#include "tdd_relay_elec.h"
#include "tdd_relay_mag.h"

#if (defined(ENERGY_MONITOR_CHIP_BL0937) && ENERGY_MONITOR_CHIP_BL0937 == 1)
#include "tdd_energy_monitor_bl0937_hlw8012.h"
#elif (defined(ENERGY_MONITOR_CHIP_BL0942) && ENERGY_MONITOR_CHIP_BL0942 == 1)
#include "tdd_energy_monitor_bl0942.h"
#elif (defined(ENERGY_MONITOR_CHIP_HLW8012) && ENERGY_MONITOR_CHIP_HLW8012 == 1)
#include "tdd_energy_monitor_bl0937_hlw8012.h"
#elif (defined(ENERGY_MONITOR_CHIP_HLW8032) && ENERGY_MONITOR_CHIP_HLW8032 == 1)
#include "tdd_energy_monitor_hlw8032.h"
#endif
#if (defined(ELEC_FFC_BEACON_REMOTE_EN) && (ELEC_FFC_BEACON_REMOTE_EN==1))
#include "app_beacon_remote.h"
#include "app_elec_ffc.h"
#endif
#if (defined(ELEC_JSON_UPGRADE_EN) && (ELEC_JSON_UPGRADE_EN==1))
#include "ty_app_elec_json_upgrade.h"
#endif

/***********************************************************
************************macro define************************
***********************************************************/
// 网络指示灯
#define NET_INDICATE_CFG_INIT(led_cfg)          \
    do {                                        \
        led_cfg.pin = ELEC_NET_LED_PIN;         \
        led_cfg.mode = ELEC_NET_LED_PIN_MODE;   \
        led_cfg.level = ELEC_NET_LED_LEVEL;     \
    } while(0)

// 总控指示灯

// 按键
#define BUTTON_NAME(seq)              ELEC_BUTTON_##seq##_NAME
#define BUTTON_GPIO_PIN(seq)          ELEC_BUTTON_##seq##_PIN
#define BUTTON_GPIO_LEVEL(seq)        ELEC_BUTTON_##seq##_ACTIVE_LEVEL

#define BUTTON_GPIO_CFG_INIT(gpio_cfg, seq)               \
    do {                                                  \
        memset(&gpio_cfg, 0, sizeof(BUTTON_GPIO_CFG_T));  \
        gpio_cfg.pin   = (BUTTON_GPIO_PIN(seq) > TUYA_GPIO_NUM_MAX)?(TUYA_GPIO_NUM_MAX):(BUTTON_GPIO_PIN(seq)); \
        gpio_cfg.level = BUTTON_GPIO_LEVEL(seq);          \
        gpio_cfg.mode  = BUTTON_TIMER_SCAN_MODE;          \
    } while (0)

/*                                   通道                                       */
// 继电器
#define RELAY_NAME(seq)             ELEC_CHANNEL_##seq##_RELAY_NAME
#define RELAY_TYPE(seq)             ELEC_CHANNEL_##seq##_RELAY_TYPE
#define RELAY_PIN_MODE(seq)         ELEC_CHANNEL_##seq##_RELAY_PIN_MODE
// 电保持继电器
#define ELEC_RELAY_PIN(seq)         ELEC_CHANNEL_##seq##_RELAY_PIN
#define ELEC_RELAY_LEVEL(seq)       ELEC_CHANNEL_##seq##_RELAY_LEVEL

#define ELEC_RELAY_GPIO_CFG_INIT(elec_relay, seq)                       \
    do {                                                                \
        memset(&elec_relay, 0, sizeof(ELEC_RELAY_DRIVER_CONFIG_T)); \
        elec_relay.pin = (ELEC_RELAY_PIN(seq) > TUYA_GPIO_NUM_MAX)?(TUYA_GPIO_NUM_MAX):(ELEC_RELAY_PIN(seq)); \
        elec_relay.mode = RELAY_PIN_MODE(seq);                          \
        elec_relay.level = ELEC_RELAY_LEVEL(seq);                       \
    } while (0)

// 磁保持继电器
#define MAG_RELAY_ON_PIN(seq)       ELEC_CHANNEL_##seq##_RELAY_ON_PIN
#define MAG_RELAY_OFF_PIN(seq)      ELEC_CHANNEL_##seq##_RELAY_OFF_PIN
#define MAG_RELAY_NORMAL_LEVEL(seq) ELEC_CHANNEL_##seq##_RELAY_NORMAL_LEVEL
#define MAG_RELAY_HOLD_MS(seq)      ELEC_CHANNEL_##seq##_RELAY_HOLD_MS

#define MAG_RELAY_GPIO_CFG_INIT(mag_relay, seq)                       \
    do {                                                              \
        memset(&mag_relay_cfg, 0, sizeof(MAG_RELAY_DRIVER_CONFIG_T)); \
        mag_relay.on_pin = MAG_RELAY_ON_PIN(seq);                     \
        mag_relay.off_pin = MAG_RELAY_OFF_PIN(seq);                   \
        mag_relay.mode = RELAY_PIN_MODE(seq);                         \
        mag_relay.level = MAG_RELAY_NORMAL_LEVEL(seq);                \
        mag_relay.hold_ms = MAG_RELAY_HOLD_MS(seq);                   \
    } while (0)

// 通道指示灯
#define CHANNEL_LED_NAME(seq)      ELEC_CHANNEL_##seq##_LED_NAME
#define CHANNEL_LED_PIN(seq)       ELEC_CHANNEL_##seq##_LED_PIN
#define CHANNEL_LED_MODE(seq)      ELEC_CHANNEL_##seq##_LED_MODE
#define CHANNEL_LED_LEVEL(seq)     ELEC_CHANNEL_##seq##_LED_LEVEL

#if defined(T1_PLUG_WAY_NUM)
#define APP_ELEC_ACTIVE_CHANNEL_NUM T1_PLUG_WAY_NUM
#else
#define APP_ELEC_ACTIVE_CHANNEL_NUM DEFAULT_ELEC_CHANNEL_NUM
#endif

#define CHANNEL_LED_GPIO_CFG_INIT(led_cfg, seq)         \
    do {                                                \
        memset(&led_cfg, 0, sizeof(LED_GPIO_CFG_T));    \
        led_cfg.pin = (CHANNEL_LED_PIN(seq) > TUYA_GPIO_NUM_MAX)?(TUYA_GPIO_NUM_MAX):(CHANNEL_LED_PIN(seq)); \
        led_cfg.mode = (CHANNEL_LED_MODE(seq));           \
        led_cfg.level = (CHANNEL_LED_LEVEL(seq));         \
    }while(0)

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
STATIC OPERATE_RET __ty_app_elec_button_register(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

    BUTTON_GPIO_CFG_T gpio_cfg = {0};

    (void)gpio_cfg;
#if defined(ENABLE_DEFAULT_ELEC_BUTTON_0) && (ENABLE_DEFAULT_ELEC_BUTTON_0==1)
    BUTTON_GPIO_CFG_INIT(gpio_cfg, 0);
    TUYA_CALL_ERR_RETURN(tdd_gpio_button_register(ELEC_BUTTON_0_NAME, &gpio_cfg));
#endif

#if defined(ENABLE_DEFAULT_ELEC_BUTTON_1) && (ENABLE_DEFAULT_ELEC_BUTTON_1==1)
    BUTTON_GPIO_CFG_INIT(gpio_cfg, 1);
    TUYA_CALL_ERR_RETURN(tdd_gpio_button_register(ELEC_BUTTON_1_NAME, &gpio_cfg));
#endif

#if defined(ENABLE_DEFAULT_ELEC_BUTTON_2) && (ENABLE_DEFAULT_ELEC_BUTTON_2==1)
    BUTTON_GPIO_CFG_INIT(gpio_cfg, 2);
    TUYA_CALL_ERR_RETURN(tdd_gpio_button_register(ELEC_BUTTON_2_NAME, &gpio_cfg));
#endif

#if defined(ENABLE_DEFAULT_ELEC_BUTTON_3) && (ENABLE_DEFAULT_ELEC_BUTTON_3==1)
    BUTTON_GPIO_CFG_INIT(gpio_cfg, 3);
    TUYA_CALL_ERR_RETURN(tdd_gpio_button_register(ELEC_BUTTON_3_NAME, &gpio_cfg));
#endif

#if defined(ENABLE_DEFAULT_ELEC_BUTTON_4) && (ENABLE_DEFAULT_ELEC_BUTTON_4==1)
    BUTTON_GPIO_CFG_INIT(gpio_cfg, 4);
    TUYA_CALL_ERR_RETURN(tdd_gpio_button_register(ELEC_BUTTON_4_NAME, &gpio_cfg));
#endif

#if defined(ENABLE_DEFAULT_ELEC_BUTTON_5) && (ENABLE_DEFAULT_ELEC_BUTTON_5==1)
    BUTTON_GPIO_CFG_INIT(gpio_cfg, 5);
    TUYA_CALL_ERR_RETURN(tdd_gpio_button_register(ELEC_BUTTON_5_NAME, &gpio_cfg));
#endif

#if defined(ENABLE_DEFAULT_ELEC_BUTTON_6) && (ENABLE_DEFAULT_ELEC_BUTTON_6==1)
    BUTTON_GPIO_CFG_INIT(gpio_cfg, 6);
    TUYA_CALL_ERR_RETURN(tdd_gpio_button_register(ELEC_BUTTON_6_NAME, &gpio_cfg));
#endif

#if defined(ENABLE_DEFAULT_ELEC_BUTTON_7) && (ENABLE_DEFAULT_ELEC_BUTTON_7==1)
    BUTTON_GPIO_CFG_INIT(gpio_cfg, 7);
    TUYA_CALL_ERR_RETURN(tdd_gpio_button_register(ELEC_BUTTON_7_NAME, &gpio_cfg));
#endif

#if defined(ENABLE_DEFAULT_ELEC_BUTTON_8) && (ENABLE_DEFAULT_ELEC_BUTTON_8==1)
    BUTTON_GPIO_CFG_INIT(gpio_cfg, 8);
    TUYA_CALL_ERR_RETURN(tdd_gpio_button_register(ELEC_BUTTON_8_NAME, &gpio_cfg));
#endif

    return rt;
}
STATIC OPERATE_RET __ty_app_elec_power_led_register(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

#ifdef ELEC_POWER_LED_PIN
        LED_GPIO_CFG_T led_cfg = {
            .pin = ELEC_POWER_LED_PIN,
            .mode = ELEC_POWER_LED_PIN_MODE,
            .level = ELEC_POWER_LED_LEVEL,
        };
        TUYA_CALL_ERR_RETURN(tdd_led_gpio_init(ELEC_POWER_LED_NAME, led_cfg));
#endif

    return rt;
}

STATIC OPERATE_RET __ty_app_elec_energy_meter_register(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

#if (defined(ENERGY_MONITOR_CHIP_BL0937) && ENERGY_MONITOR_CHIP_BL0937 == 1)
    BL0937_DRIVER_CONFIG_T bl0937_cfg = {
        .timer_id = ENERGY_MONITOR_TIMER_ID,
        .sel_pin = ENERGY_MONITOR_SEL_PIN,
        .sel_level = ENERGY_MONITOR_SEL_LEVEL,
        .cf1_pin = ENERGY_MONITOR_CF1_PIN,
        .cf_pin = ENERGY_MONITOR_CF_PIN,
    };
    rt = tdd_energy_driver_bl0937_register(ENERGY_MONITOR_NAME, bl0937_cfg);
#elif (defined(ENERGY_MONITOR_CHIP_BL0942) && ENERGY_MONITOR_CHIP_BL0942 == 1)
    BL0942_DRIVER_CONFIG_T bl0942_cfg = {
        .mode = BL0942_DRV_UART,
        .driver.uart = {
            .id = ENERGY_MONITOR_UART_ID,
            .addr = ENERGY_MONITOR_CHIP_ADDR,
        },
    };
    rt = tdd_energy_driver_bl0942_register(ENERGY_MONITOR_NAME, bl0942_cfg);
#elif (defined(ENERGY_MONITOR_CHIP_HLW8012) && ENERGY_MONITOR_CHIP_HLW8012 == 1)
    HLW8012_DRIVER_CONFIG_T hlw8012_cfg = {
        .timer_id = ENERGY_MONITOR_TIMER_ID,
        .sel_pin = ENERGY_MONITOR_SEL_PIN,
        .sel_level = ENERGY_MONITOR_SEL_LEVEL,
        .cf1_pin = ENERGY_MONITOR_CF1_PIN,
        .cf_pin = ENERGY_MONITOR_CF_PIN,
    };
    rt = tdd_energy_driver_hlw8012_register(ENERGY_MONITOR_NAME, hlw8012_cfg);
#elif (defined(ENERGY_MONITOR_CHIP_HLW8032) && ENERGY_MONITOR_CHIP_HLW8032 == 1)
    HLW8032_DRIVER_CONFIG_T hlw8032_cfg = {
        .uart_id = ENERGY_MONITOR_UART_ID,
    };
    rt = tdd_energy_driver_hlw8032_register(ENERGY_MONITOR_NAME, hlw8032_cfg);
#endif

    return rt;
}

STATIC OPERATE_RET __ty_app_elec_net_indicate_register(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

    LED_GPIO_CFG_T led_cfg = {0};
    NET_INDICATE_CFG_INIT(led_cfg);

    TUYA_CALL_ERR_RETURN(tdd_led_gpio_init(ELEC_NET_LED_NAME, led_cfg));

    return rt;
}

STATIC OPERATE_RET __ty_app_elec_channel_register(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    #if (defined(ENABLE_TY_LOAD_OEM_PAR) && (ENABLE_TY_LOAD_OEM_PAR == 0))
    // #if (!defined(ENABLE_TY_LOAD_OEM_PAR) || (defined(ENABLE_TY_LOAD_OEM_PAR) && (ENABLE_TY_LOAD_OEM_PAR == 0)))

#if ((defined(ELEC_CHANNEL_1_RELAY_TYPE) && (0 == ELEC_CHANNEL_1_RELAY_TYPE)) || \
        (defined(ELEC_CHANNEL_2_RELAY_TYPE) && (0 == ELEC_CHANNEL_2_RELAY_TYPE)) || \
        (defined(ELEC_CHANNEL_3_RELAY_TYPE) && (0 == ELEC_CHANNEL_3_RELAY_TYPE)) || \
        (defined(ELEC_CHANNEL_4_RELAY_TYPE) && (0 == ELEC_CHANNEL_4_RELAY_TYPE)) || \
        (defined(ELEC_CHANNEL_5_RELAY_TYPE) && (0 == ELEC_CHANNEL_5_RELAY_TYPE)) || \
        (defined(ELEC_CHANNEL_6_RELAY_TYPE) && (0 == ELEC_CHANNEL_6_RELAY_TYPE)) || \
        (defined(ELEC_CHANNEL_7_RELAY_TYPE) && (0 == ELEC_CHANNEL_7_RELAY_TYPE)) || \
        (defined(ELEC_CHANNEL_8_RELAY_TYPE) && (0 == ELEC_CHANNEL_8_RELAY_TYPE)))
    ELEC_RELAY_DRIVER_CONFIG_T elec_relay_cfg = {0};
#endif

#if ((defined(ELEC_CHANNEL_1_RELAY_TYPE) && (1 == ELEC_CHANNEL_1_RELAY_TYPE)) || \
        (defined(ELEC_CHANNEL_2_RELAY_TYPE) && (1 == ELEC_CHANNEL_2_RELAY_TYPE)) || \
        (defined(ELEC_CHANNEL_3_RELAY_TYPE) && (1 == ELEC_CHANNEL_3_RELAY_TYPE)) || \
        (defined(ELEC_CHANNEL_4_RELAY_TYPE) && (1 == ELEC_CHANNEL_4_RELAY_TYPE)) || \
        (defined(ELEC_CHANNEL_5_RELAY_TYPE) && (1 == ELEC_CHANNEL_5_RELAY_TYPE)) || \
        (defined(ELEC_CHANNEL_6_RELAY_TYPE) && (1 == ELEC_CHANNEL_6_RELAY_TYPE)) || \
        (defined(ELEC_CHANNEL_7_RELAY_TYPE) && (1 == ELEC_CHANNEL_7_RELAY_TYPE)) || \
        (defined(ELEC_CHANNEL_8_RELAY_TYPE) && (1 == ELEC_CHANNEL_8_RELAY_TYPE)))
    MAG_RELAY_DRIVER_CONFIG_T mag_relay_cfg = {0};
#endif
    LED_GPIO_CFG_T led_cfg = {0};

#if (defined(APP_ELEC_ACTIVE_CHANNEL_NUM) && (APP_ELEC_ACTIVE_CHANNEL_NUM >= 1))
    #if (defined(ELEC_CHANNEL_1_RELAY_TYPE) && (0 == ELEC_CHANNEL_1_RELAY_TYPE))
    // 电保持继电器硬件注册
    ELEC_RELAY_GPIO_CFG_INIT(elec_relay_cfg, 1);
    TUYA_CALL_ERR_RETURN(tdd_relay_elec_register(ELEC_CHANNEL_1_RELAY_NAME, elec_relay_cfg));
    #elif (defined(ELEC_CHANNEL_1_RELAY_TYPE) && (1 == ELEC_CHANNEL_1_RELAY_TYPE))
    // 磁保持继电器硬件注册
    MAG_RELAY_GPIO_CFG_INIT(mag_relay_cfg, 1);
    TUYA_CALL_ERR_RETURN(tdd_relay_mag_register(ELEC_CHANNEL_1_RELAY_NAME, mag_relay_cfg));
    #endif
    #if (defined(ENABLE_ELEC_CHANNEL_1_LED) && (1 == ENABLE_ELEC_CHANNEL_1_LED))
    // 通道指示灯硬件注册
    CHANNEL_LED_GPIO_CFG_INIT(led_cfg, 1);
    TAL_PR_DEBUG("===> led active level: %d", led_cfg.level);
    TUYA_CALL_ERR_RETURN(tdd_led_gpio_init(ELEC_CHANNEL_1_LED_NAME, led_cfg));
    #endif
#endif

#if (defined(APP_ELEC_ACTIVE_CHANNEL_NUM) && (APP_ELEC_ACTIVE_CHANNEL_NUM >= 2))
    #if (defined(ELEC_CHANNEL_2_RELAY_TYPE) && (0 == ELEC_CHANNEL_2_RELAY_TYPE))
    ELEC_RELAY_GPIO_CFG_INIT(elec_relay_cfg, 2);
    TUYA_CALL_ERR_RETURN(tdd_relay_elec_register(ELEC_CHANNEL_2_RELAY_NAME, elec_relay_cfg));
    #elif (defined(ELEC_CHANNEL_2_RELAY_TYPE) && (1 == ELEC_CHANNEL_2_RELAY_TYPE))
    MAG_RELAY_GPIO_CFG_INIT(mag_relay_cfg, 2);
    TUYA_CALL_ERR_RETURN(tdd_relay_mag_register(ELEC_CHANNEL_2_RELAY_NAME, mag_relay_cfg));
    #endif
    #if (defined(ENABLE_ELEC_CHANNEL_2_LED) && (1 == ENABLE_ELEC_CHANNEL_2_LED))
    CHANNEL_LED_GPIO_CFG_INIT(led_cfg, 2);
    TUYA_CALL_ERR_RETURN(tdd_led_gpio_init(ELEC_CHANNEL_2_LED_NAME, led_cfg));
    #endif
#endif

#if (defined(APP_ELEC_ACTIVE_CHANNEL_NUM) && (APP_ELEC_ACTIVE_CHANNEL_NUM >= 3))
    #if (defined(ELEC_CHANNEL_3_RELAY_TYPE) && (0 == ELEC_CHANNEL_3_RELAY_TYPE))
    ELEC_RELAY_GPIO_CFG_INIT(elec_relay_cfg, 3);
    TUYA_CALL_ERR_RETURN(tdd_relay_elec_register(ELEC_CHANNEL_3_RELAY_NAME, elec_relay_cfg));
    #elif (defined(ELEC_CHANNEL_3_RELAY_TYPE) && (1 == ELEC_CHANNEL_3_RELAY_TYPE))
    MAG_RELAY_GPIO_CFG_INIT(mag_relay_cfg, 3);
    TUYA_CALL_ERR_RETURN(tdd_relay_mag_register(ELEC_CHANNEL_3_RELAY_NAME, mag_relay_cfg));
    #endif
    #if (defined(ENABLE_ELEC_CHANNEL_3_LED) && (1 == ENABLE_ELEC_CHANNEL_3_LED))
    CHANNEL_LED_GPIO_CFG_INIT(led_cfg, 3);
    TUYA_CALL_ERR_RETURN(tdd_led_gpio_init(ELEC_CHANNEL_3_LED_NAME, led_cfg));
    #endif
#endif

#if (defined(APP_ELEC_ACTIVE_CHANNEL_NUM) && (APP_ELEC_ACTIVE_CHANNEL_NUM >= 4))
    #if (defined(ELEC_CHANNEL_4_RELAY_TYPE) && (0 == ELEC_CHANNEL_4_RELAY_TYPE))
    ELEC_RELAY_GPIO_CFG_INIT(elec_relay_cfg, 4);
    TUYA_CALL_ERR_RETURN(tdd_relay_elec_register(ELEC_CHANNEL_4_RELAY_NAME, elec_relay_cfg));
    #elif (defined(ELEC_CHANNEL_4_RELAY_TYPE) && (1 == ELEC_CHANNEL_4_RELAY_TYPE))
    MAG_RELAY_GPIO_CFG_INIT(mag_relay_cfg, 4);
    TUYA_CALL_ERR_RETURN(tdd_relay_mag_register(ELEC_CHANNEL_4_RELAY_NAME, mag_relay_cfg));
    #endif
    #if (defined(ENABLE_ELEC_CHANNEL_4_LED) && (1 == ENABLE_ELEC_CHANNEL_4_LED))
    CHANNEL_LED_GPIO_CFG_INIT(led_cfg, 4);
    TUYA_CALL_ERR_RETURN(tdd_led_gpio_init(ELEC_CHANNEL_4_LED_NAME, led_cfg));
    #endif
#endif

#if (defined(APP_ELEC_ACTIVE_CHANNEL_NUM) && (APP_ELEC_ACTIVE_CHANNEL_NUM >= 5))
    #if (defined(ELEC_CHANNEL_5_RELAY_TYPE) && (0 == ELEC_CHANNEL_5_RELAY_TYPE))
    ELEC_RELAY_GPIO_CFG_INIT(elec_relay_cfg, 5);
    TUYA_CALL_ERR_RETURN(tdd_relay_elec_register(ELEC_CHANNEL_5_RELAY_NAME, elec_relay_cfg));
    #elif (defined(ELEC_CHANNEL_5_RELAY_TYPE) && (1 == ELEC_CHANNEL_5_RELAY_TYPE))
    MAG_RELAY_GPIO_CFG_INIT(mag_relay_cfg, 5);
    TUYA_CALL_ERR_RETURN(tdd_relay_mag_register(ELEC_CHANNEL_5_RELAY_NAME, mag_relay_cfg));
    #endif
    #if (defined(ENABLE_ELEC_CHANNEL_5_LED) && (1 == ENABLE_ELEC_CHANNEL_5_LED))
    CHANNEL_LED_GPIO_CFG_INIT(led_cfg, 5);
    TUYA_CALL_ERR_RETURN(tdd_led_gpio_init(ELEC_CHANNEL_5_LED_NAME, led_cfg));
    #endif
#endif

#if (defined(APP_ELEC_ACTIVE_CHANNEL_NUM) && (APP_ELEC_ACTIVE_CHANNEL_NUM >= 6))
    #if (defined(ELEC_CHANNEL_6_RELAY_TYPE) && (0 == ELEC_CHANNEL_6_RELAY_TYPE))
    ELEC_RELAY_GPIO_CFG_INIT(elec_relay_cfg, 6);
    TUYA_CALL_ERR_RETURN(tdd_relay_elec_register(ELEC_CHANNEL_6_RELAY_NAME, elec_relay_cfg));
    #elif (defined(ELEC_CHANNEL_6_RELAY_TYPE) && (1 == ELEC_CHANNEL_6_RELAY_TYPE))
    MAG_RELAY_GPIO_CFG_INIT(mag_relay_cfg, 6);
    TUYA_CALL_ERR_RETURN(tdd_relay_mag_register(ELEC_CHANNEL_6_RELAY_NAME, mag_relay_cfg));
    #endif
    #if (defined(ENABLE_ELEC_CHANNEL_6_LED) && (1 == ENABLE_ELEC_CHANNEL_6_LED))
    CHANNEL_LED_GPIO_CFG_INIT(led_cfg, 6);
    TUYA_CALL_ERR_RETURN(tdd_led_gpio_init(ELEC_CHANNEL_6_LED_NAME, led_cfg));
    #endif
#endif

#if (defined(APP_ELEC_ACTIVE_CHANNEL_NUM) && (APP_ELEC_ACTIVE_CHANNEL_NUM >= 7))
    #if (defined(ELEC_CHANNEL_7_RELAY_TYPE) && (0 == ELEC_CHANNEL_7_RELAY_TYPE))
    ELEC_RELAY_GPIO_CFG_INIT(elec_relay_cfg, 7);
    TUYA_CALL_ERR_RETURN(tdd_relay_elec_register(ELEC_CHANNEL_7_RELAY_NAME, elec_relay_cfg));
    #elif (defined(ELEC_CHANNEL_7_RELAY_TYPE) && (1 == ELEC_CHANNEL_7_RELAY_TYPE))
    MAG_RELAY_GPIO_CFG_INIT(mag_relay_cfg, 7);
    TUYA_CALL_ERR_RETURN(tdd_relay_mag_register(ELEC_CHANNEL_7_RELAY_NAME, mag_relay_cfg));
    #endif
    #if (defined(ENABLE_ELEC_CHANNEL_7_LED) && (1 == ENABLE_ELEC_CHANNEL_7_LED))
    CHANNEL_LED_GPIO_CFG_INIT(led_cfg, 7);
    TUYA_CALL_ERR_RETURN(tdd_led_gpio_init(ELEC_CHANNEL_7_LED_NAME, led_cfg));
    #endif
#endif

#if (defined(APP_ELEC_ACTIVE_CHANNEL_NUM) && (APP_ELEC_ACTIVE_CHANNEL_NUM >= 8))
    #if (defined(ELEC_CHANNEL_8_RELAY_TYPE) && (0 == ELEC_CHANNEL_8_RELAY_TYPE))
    ELEC_RELAY_GPIO_CFG_INIT(elec_relay_cfg, 8);
    TUYA_CALL_ERR_RETURN(tdd_relay_elec_register(ELEC_CHANNEL_8_RELAY_NAME, elec_relay_cfg));
    #elif (defined(ELEC_CHANNEL_8_RELAY_TYPE) && (1 == ELEC_CHANNEL_8_RELAY_TYPE))
    MAG_RELAY_GPIO_CFG_INIT(mag_relay_cfg, 8);
    TUYA_CALL_ERR_RETURN(tdd_relay_mag_register(ELEC_CHANNEL_8_RELAY_NAME, mag_relay_cfg));
    #endif
    #if (defined(ENABLE_ELEC_CHANNEL_8_LED) && (1 == ENABLE_ELEC_CHANNEL_8_LED))
    CHANNEL_LED_GPIO_CFG_INIT(led_cfg, 8);
    TUYA_CALL_ERR_RETURN(tdd_led_gpio_init(ELEC_CHANNEL_8_LED_NAME, led_cfg));
    #endif
#endif
#endif

    return rt;
}

#if (defined(ELEC_FFC_BEACON_REMOTE_EN) && (ELEC_FFC_BEACON_REMOTE_EN==1))
STATIC OPERATE_RET __ty_app_elec_ffc_beacon_register(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
   
    UINT_T ffc_select = ELEC_FFC_SELECT;
    UINT_T pair_time = ELEC_BLE_PAIR_TIME;

    TAL_PR_NOTICE("ffc_select = %d, pair_time = %d", ffc_select, pair_time);
    //1-wifi ffc;2-beacon
    if(1 == ffc_select){
        //WIFI 遥控器
        app_wifi_ffc_init(ffc_select,pair_time);
    }else if(2 == ffc_select){
        app_beacon_remote_init(pair_time);
    }else{
        TAL_PR_NOTICE("ffc beacon control is not supported !");
    }
    return rt;
}
#endif

#if (defined(ELEC_JSON_UPGRADE_EN) && (ELEC_JSON_UPGRADE_EN==1))
STATIC OPERATE_RET __ty_app_elec_json_upgrade_register(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

    app_json_version(ELEC_JSON_VERSION);
    return rt;
}
#endif
/**
 * @brief    硬件注册
 *
 * @param     none 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_elec_hardware_reg(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

    /* 网络指示灯注册 */
    TUYA_CALL_ERR_RETURN(__ty_app_elec_net_indicate_register());

    /* 按键注册 */
    TUYA_CALL_ERR_RETURN(__ty_app_elec_button_register());

    /* 总控指示灯注册 */
    TUYA_CALL_ERR_RETURN(__ty_app_elec_power_led_register());

    /* 通道注册（继电器，通道指示灯） */
    TUYA_CALL_ERR_RETURN(__ty_app_elec_channel_register());

    /* 计量注册 */
#if (defined(ENERGY_MONITOR_ENABLE) && (ENERGY_MONITOR_ENABLE==1))
    TUYA_CALL_ERR_RETURN(__ty_app_elec_energy_meter_register());
#endif

    /* 遥控器注册 */
#if (defined(ELEC_FFC_BEACON_REMOTE_EN) && (ELEC_FFC_BEACON_REMOTE_EN==1))
    TUYA_CALL_ERR_RETURN(__ty_app_elec_ffc_beacon_register());
#endif
    /* JSON版本号注册 */
#if (defined(ELEC_JSON_UPGRADE_EN) && (ELEC_JSON_UPGRADE_EN==1))
    TUYA_CALL_ERR_RETURN(__ty_app_elec_json_upgrade_register());
#endif

    return rt;
}
