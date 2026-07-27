/**
 * @file ty_app_elec_fault.c
 * @author www.tuya.com
 * @brief ty_app_elec_fault module is used to 
 * @version 0.1
 * @date 2023-07-10
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_app_config.h"
#include "ty_sys.h"

#if (defined(ENABLE_ENERGY_FAULT) && (ENABLE_ENERGY_FAULT==1))
#include "ty_app_elec_event_code.h"
#include "app_elec_channel.h"
#include "app_elec_fault.h"

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

VOID_T ty_app_elec_fault_trigger(APP_ELEC_FAULT_T fault)
{
    APP_ELEC_CHANNEL_CFG_T channel_cfg = {0};

    TAL_PR_DEBUG("fault value: %x", fault);

    app_elec_fault_upload();

    if (0 == fault) {
        return;
    }

    // close all relay
    channel_cfg.chan_id = 0;
    channel_cfg.status = STATE_OFF;
    ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_CHANNEL_STATUS_SET, &channel_cfg, SIZEOF(APP_ELEC_CHANNEL_CFG_T));

    return;
}

#endif /* ENABLE_ENERGY_FAULT */
