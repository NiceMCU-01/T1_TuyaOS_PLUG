/**
 * @file app_elec_overcharge.h
 * @author www.tuya.com
 * @brief app_elec_overcharge module is used to 
 * @version 0.1
 * @date 2023-07-06
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __APP_ELEC_OVERCHARGE_H__
#define __APP_ELEC_OVERCHARGE_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef VOID_T (*OVERCHARGE_CB)(VOID_T);

/***********************************************************
********************function declaration********************
***********************************************************/

OPERATE_RET app_elec_overcharge_init(OVERCHARGE_CB cb);

OPERATE_RET app_elec_overcharge_status_set(BOOL_T is_enable);

OPERATE_RET app_elec_overcharge_start(VOID_T);

OPERATE_RET app_elec_overcharge_stop(VOID_T);

OPERATE_RET app_elec_overcharge_dp_upload(VOID_T);

OPERATE_RET app_elec_overcharge_date_easer(VOID_T);

OPERATE_RET app_elec_overcharge_detect(UINT32_T power_value);

OPERATE_RET app_elec_overcharge_channel_sync(UINT_T chan_status);
#ifdef __cplusplus
}
#endif

#endif /* __APP_ELEC_OVERCHARGE_H__ */
