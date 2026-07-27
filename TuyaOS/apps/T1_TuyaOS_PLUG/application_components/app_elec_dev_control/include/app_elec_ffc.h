/*
 * @Descripttion: 
 * @version: 1.0
 * @Date: 2025-04-10 10:26:34
 */
#ifndef ___APP_ELEC_FFC_H__
#define ___APP_ELEC_FFC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_cloud_types.h"

/**
 * @function: app_wifi_ffc_init
 * @description: app_wifi_ffc_init
 * @param[in]: select ,pair_time 
 * @param[out]: none
 * @retval: none
 */
VOID app_wifi_ffc_init(UINT_T select , UINT_T time);
/**
 * @function: app_Wifi_Ffc_Register
 * @description: app_Wifi_Ffc_Register
 * @param[in]: VOID 
 * @param[out]: none
 * @retval: none
 */
VOID app_Wifi_Ffc_Register(VOID);
/**
 * @function: app_get_ffc_func_en
 * @description: app_get_ffc_func_en
 * @param[in]: pair_time 
 * @param[out]: none
 * @retval: none
 */
BOOL_T app_get_ffc_func_en(VOID);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
