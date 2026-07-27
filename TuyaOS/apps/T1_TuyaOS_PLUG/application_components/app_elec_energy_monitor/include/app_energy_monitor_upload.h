/**
 * @file app_energy_monitor_upload.h
 * @author www.tuya.com
 * @brief app_energy_monitor_upload module is used to 
 * @version 0.1
 * @date 2023-09-15
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __APP_ENERGY_MONITOR_UPLOAD_H__
#define __APP_ENERGY_MONITOR_UPLOAD_H__

#include "tuya_cloud_types.h"
#include "tdl_energy_monitor_manage.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef PVOID_T TFM_ENERGY_HANDLE;

typedef struct {
    OPERATE_RET (*pvi_upload_cb)(UINT_T voltage, UINT_T current, UINT_T power); // pvi上报回调
    OPERATE_RET (*add_energy_upload_cb)(UINT_T electric_quantity, UINT_T timestamp);  // 电能数据上报
} TFM_ENERGY_CB_T;

/***********************************************************
********************function declaration********************
***********************************************************/

OPERATE_RET app_energy_monitor_upload_init(ENERGY_MONITOR_HANDLE_T tdl_hdl, TFM_ENERGY_CB_T energy_cb, TFM_ENERGY_HANDLE *handle);

OPERATE_RET app_energy_monitor_pvi_sample_upload_now(TFM_ENERGY_HANDLE handle);

VOID_T app_electric_quantity_upload(TFM_ENERGY_HANDLE handle);

OPERATE_RET app_energy_monitor_data_erase(TFM_ENERGY_HANDLE handle);

OPERATE_RET app_energy_monitor_pvi_get(TFM_ENERGY_HANDLE handle,ENERGY_MONITOR_VCP_T *realtime_data);

#ifdef __cplusplus
}
#endif

#endif /* __APP_ENERGY_MONITOR_UPLOAD_H__ */
