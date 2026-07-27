/**
* @file tfm_oem_cfg_hashmap.h
* @author www.tuya.com
* @brief tbs_oem_cfg_hashmap module is used to declare hashmap operate interface
* @version 0.1
* @date 2021-08-27
*
* @copyright Copyright (c) tuya.inc 2021
*
*/
 
#ifndef __TFM_OEM_CFG_HASHMAP_H__
#define __TFM_OEM_CFG_HASHMAP_H__
 
#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif
 
/***********************************************************
*************************micro define***********************
***********************************************************/
#define HASHMAP_TABLE_LEN   160 /* length of hashmap table, bigger length can improve find speed but need more memory */

/***********************************************************
***********************typedef define***********************
***********************************************************/
 
 
/***********************************************************
***********************variable define**********************
***********************************************************/
 
 
/***********************************************************
***********************function define**********************
***********************************************************/
 
/**
 * @brief: put data into hashmap, hashmap is saved 
 * 
 * @param[in]: key -> key of hash node 
 * @param[in]: p_value -> value of hash node, user need malloc memory external!!!!
 * @return: OPRT_OK->Success, other->Refer to tuya error code
 * @attention:this function won't overwrite data with same key
 */
OPERATE_RET tfm_oem_config_hashmap_put_value(IN CONST CHAR_T *p_key, IN VOID *p_value);

/**
 * @brief: put data into hashmap and if the key is exist, the value will be updated
 * 
 * @param[in]: key -> key of hash node 
 * @param[in]: p_value -> value of hash node, user need malloc memory external!!!!
 * @return: OPRT_OK->Success, other->Refer to tuya error code
 * @attention:this function will overwrite data with same key
 */
OPERATE_RET tfm_oem_config_hashmap_put_value_update(IN CONST CHAR_T *p_key, OUT VOID *p_value);


/**
 * @brief: put data into hashmap
 * 
 * @param[in]: key -> key of hash node 
 * @param[OUT]: p_value -> value of hash node
 * @return: OPRT_OK->Success, other->Refer to tuya error code
 */
OPERATE_RET tfm_oem_config_hashmap_get_value(IN CONST CHAR_T *p_key, OUT VOID **p_value);

/**
 * @brief: destroy hashmap
 * 
 * @param[in]: NONE
 * @return: NONE
 */
VOID_T tfm_oem_config_hashmap_destroy(VOID_T);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*__SVC_OEM_CONFIG_HASHMAP_H__*/

