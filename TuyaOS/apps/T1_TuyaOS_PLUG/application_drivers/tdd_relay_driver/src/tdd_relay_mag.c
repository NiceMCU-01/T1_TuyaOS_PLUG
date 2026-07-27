/**
 * @file tdd_relay_mag.c
 * @author www.tuya.com
 * @brief tdd_relay_mag module is used to 
 * @version 0.1
 * @date 2023-03-23
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tkl_gpio.h"

#include "tal_log.h"
#include "tal_memory.h"
#include "tal_system.h"

#include "tdd_relay_mag.h"
#include "tdl_relay_driver.h"

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

STATIC OPERATE_RET __tdd_relay_mag_open(RELAY_DRV_HANDLE handle)
{
    OPERATE_RET rt = OPRT_OK;

    MAG_RELAY_DRIVER_CONFIG_T *relay_cfg = (MAG_RELAY_DRIVER_CONFIG_T *)handle;

    TUYA_GPIO_BASE_CFG_T gpio_cfg ={
        .direct = TUYA_GPIO_OUTPUT,
        .mode = relay_cfg->mode,
        .level = relay_cfg->level,
    };
    TUYA_CALL_ERR_RETURN(tkl_gpio_init(relay_cfg->on_pin, &gpio_cfg));
    TUYA_CALL_ERR_RETURN(tkl_gpio_init(relay_cfg->off_pin, &gpio_cfg));

    return rt;
}

STATIC OPERATE_RET __tdd_relay_mag_close(RELAY_DRV_HANDLE handle)
{
    OPERATE_RET rt = OPRT_OK;

    MAG_RELAY_DRIVER_CONFIG_T *relay_cfg = (MAG_RELAY_DRIVER_CONFIG_T *)handle;

    TUYA_CALL_ERR_RETURN(tkl_gpio_deinit(relay_cfg->on_pin));
    TUYA_CALL_ERR_RETURN(tkl_gpio_deinit(relay_cfg->off_pin));

    return rt;
}

STATIC OPERATE_RET __tdd_relay_mag_write(RELAY_DRV_HANDLE handle, UINT8_T status)
{
    OPERATE_RET rt = OPRT_OK;

    MAG_RELAY_DRIVER_CONFIG_T *relay_cfg = (MAG_RELAY_DRIVER_CONFIG_T *)handle;

    if (status) {
        TUYA_CALL_ERR_RETURN(tkl_gpio_write(relay_cfg->on_pin, TUYA_GPIO_LEVEL_HIGH));
        TUYA_CALL_ERR_RETURN(tkl_gpio_write(relay_cfg->off_pin, TUYA_GPIO_LEVEL_LOW));
    } else {
        TUYA_CALL_ERR_RETURN(tkl_gpio_write(relay_cfg->on_pin, TUYA_GPIO_LEVEL_LOW));
        TUYA_CALL_ERR_RETURN(tkl_gpio_write(relay_cfg->off_pin, TUYA_GPIO_LEVEL_HIGH));
    }

    tal_system_sleep(relay_cfg->hold_ms);

    TUYA_CALL_ERR_RETURN(tkl_gpio_write(relay_cfg->on_pin, relay_cfg->level));
    TUYA_CALL_ERR_RETURN(tkl_gpio_write(relay_cfg->off_pin, relay_cfg->level));

    return rt;
}

OPERATE_RET tdd_relay_mag_register(IN CHAR_T *name, IN MAG_RELAY_DRIVER_CONFIG_T cfg)
{
    OPERATE_RET rt = OPRT_OK;
    RELAY_DRV_INTFS_T mag_relay_intfs = {0};
    RELAY_DRV_HANDLE relay_hdl = NULL;

    TUYA_CHECK_NULL_RETURN(name, OPRT_INVALID_PARM);

    mag_relay_intfs.open = __tdd_relay_mag_open;
    mag_relay_intfs.close = __tdd_relay_mag_close;
    mag_relay_intfs.write = __tdd_relay_mag_write;

    relay_hdl = tal_malloc(sizeof(MAG_RELAY_DRIVER_CONFIG_T));
    TUYA_CHECK_NULL_RETURN(relay_hdl, OPRT_MALLOC_FAILED);
    memset(relay_hdl, 0, sizeof(MAG_RELAY_DRIVER_CONFIG_T));
    memcpy(relay_hdl, &cfg, sizeof(MAG_RELAY_DRIVER_CONFIG_T));

    TUYA_CALL_ERR_GOTO(tdl_relay_driver_register(name, &mag_relay_intfs, (RELAY_DRV_HANDLE)relay_hdl), __ERR);

    return rt;

__ERR:
    if (NULL != relay_hdl) {
        tal_free(relay_hdl);
        relay_hdl = NULL;
    }

    return rt;
}