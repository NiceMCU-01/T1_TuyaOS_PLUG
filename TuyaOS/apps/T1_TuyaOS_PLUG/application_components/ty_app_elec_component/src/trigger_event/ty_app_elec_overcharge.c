/**
 * @file ty_app_elec_overcharge.c
 * @author www.tuya.com
 * @brief ty_app_elec_overcharge module is used to 
 * @version 0.1
 * @date 2023-07-06
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_app_config.h"
#include "ty_sys.h"

#if (defined(ENABLE_ENERGY_OVERCHARGE) && (ENABLE_ENERGY_OVERCHARGE==1))
#include "ty_app_elec_event_code.h"
#include "app_elec_channel.h"

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

VOID_T ty_app_elec_overcharge_trigger(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    APP_ELEC_CHANNEL_CFG_T channel_cfg = {0};
    UINT_T chan_status = 0;
    TAL_PR_DEBUG("---> overcharge trigger");

    rt = app_elec_get_all_channel_status(&chan_status);
    if (OPRT_OK == rt && 0 == chan_status) {
        TAL_PR_DEBUG("channel is close, return");
        return;
    }

    // close device
    channel_cfg.chan_id = 0;
    channel_cfg.status = STATE_OFF;
    ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_CHANNEL_STATUS_SET, &channel_cfg, SIZEOF(APP_ELEC_CHANNEL_CFG_T));

    return;
}

#endif /* ENABLE_ENERGY_OVERCHARGE */
