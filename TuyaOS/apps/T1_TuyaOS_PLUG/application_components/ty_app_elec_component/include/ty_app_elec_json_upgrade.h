#ifndef TY_APP_ELEC_JSON_UPGRADE_H
#define TY_APP_ELEC_JSON_UPGRADE_H

#include "tuya_cloud_com_defs.h"
#if (defined(ELEC_JSON_UPGRADE_EN) && (ELEC_JSON_UPGRADE_EN==1))
#include "tfm_json_cfg_ug.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief JSON版本号
*
* @param[in] json_version JSON版本号
* @return none
*/
VOID app_json_version(CONST CHAR_T *json_version);


/**
 * @brief 设备更新通知回调
 *
 * @param[in] fw
 * @return 更新结果
 */
int app_json_upgrade_pre_cb(IN CONST FW_UG_S *fw);

/***********************************************************
*  Function:app_json_upgrade_init
*  Input: 
*  Output: 
*  Return: none
***********************************************************/
OPERATE_RET app_json_upgrade_init(VOID_T);


#ifdef __cplusplus
}
#endif 
#endif
#endif