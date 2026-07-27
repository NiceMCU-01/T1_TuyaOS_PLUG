/**
 * @file tdd_relay_elec.c
 * @author www.tuya.com
 * @brief tdd_relay_elec module is used to 
 * @version 0.1
 * @date 2023-03-23
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tkl_gpio.h"

#include "tal_log.h"
#include "tal_memory.h"

#include "tdd_relay_elec.h"
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

STATIC OPERATE_RET __tdd_relay_elec_open(RELAY_DRV_HANDLE handle)
{
    OPERATE_RET rt = OPRT_OK;

    ELEC_RELAY_DRIVER_CONFIG_T *relay_cfg = (ELEC_RELAY_DRIVER_CONFIG_T *)handle;

    TUYA_GPIO_BASE_CFG_T gpio_cfg ={
        .direct = TUYA_GPIO_OUTPUT,
        .mode = relay_cfg->mode,
        .level = !(relay_cfg->level), // 继电器初始化时默认关闭状态
    };
    rt = tkl_gpio_init(relay_cfg->pin, &gpio_cfg);

    return rt;
}

STATIC OPERATE_RET __tdd_relay_elec_close(RELAY_DRV_HANDLE handle)
{
    OPERATE_RET rt = OPRT_OK;

    ELEC_RELAY_DRIVER_CONFIG_T *relay_cfg = (ELEC_RELAY_DRIVER_CONFIG_T *)handle;

    rt = tkl_gpio_deinit(relay_cfg->pin);

    return rt;
}

STATIC OPERATE_RET __tdd_relay_elec_write(RELAY_DRV_HANDLE handle, UINT8_T status)
{
    TUYA_GPIO_LEVEL_E write_level = TUYA_GPIO_LEVEL_LOW;

    ELEC_RELAY_DRIVER_CONFIG_T *relay_cfg = (ELEC_RELAY_DRIVER_CONFIG_T *)handle;

    write_level = (status) ? (relay_cfg->level) : !(relay_cfg->level);

    return tkl_gpio_write(relay_cfg->pin, write_level);
}

OPERATE_RET tdd_relay_elec_register(IN CHAR_T *name, IN ELEC_RELAY_DRIVER_CONFIG_T cfg)
{
    OPERATE_RET rt = OPRT_OK;
    RELAY_DRV_INTFS_T elec_relay_intfs = {0};
    RELAY_DRV_HANDLE relay_hdl = NULL;

    TUYA_CHECK_NULL_RETURN(name, OPRT_INVALID_PARM);

    elec_relay_intfs.open = __tdd_relay_elec_open;
    elec_relay_intfs.close = __tdd_relay_elec_close;
    elec_relay_intfs.write = __tdd_relay_elec_write;

    relay_hdl = tal_malloc(sizeof(ELEC_RELAY_DRIVER_CONFIG_T));
    TUYA_CHECK_NULL_RETURN(relay_hdl, OPRT_MALLOC_FAILED);
    memset(relay_hdl, 0, sizeof(ELEC_RELAY_DRIVER_CONFIG_T));
    memcpy(relay_hdl, &cfg, sizeof(ELEC_RELAY_DRIVER_CONFIG_T));

    TUYA_CALL_ERR_GOTO(tdl_relay_driver_register(name, &elec_relay_intfs, (RELAY_DRV_HANDLE)relay_hdl), __ERR);

    return rt;

__ERR:
    if (NULL != relay_hdl) {
        tal_free(relay_hdl);
        relay_hdl = NULL;
    }

    return rt;
}