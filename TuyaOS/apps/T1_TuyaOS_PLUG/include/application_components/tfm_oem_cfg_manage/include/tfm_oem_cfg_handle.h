/**
* @file tfm_oem_cfg_handle.h
* @author www.tuya.com
* @brief tfm_oem_cfg_handle module is used to 
* @version 0.1
* @date 2022-08-18
*
* @copyright Copyright (c) tuya.inc 2022
*
*/
#ifndef __TFM_OEM_CFG_HANDLE_H__
#define __TFM_OEM_CFG_HANDLE_H__

#include "tuya_cloud_types.h" 

/***********************************************************
*************************micro define***********************
***********************************************************/
 
 
/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef UCHAR_T OEM_HANDLE_CMD;
#define  OEM_KV_READ      0  //从 kv 区获取config 解析
#define  OEM_UF_READ      1  //从 uf 区获取config 解析
#define  OEM_KV_UF_READ   2  //从 kv 区获取config 解析并将 kv 区数据写入 uf 区
 
/***********************************************************
***********************variable define**********************
***********************************************************/
 
/***********************************************************
***********************function define**********************
***********************************************************/
/**
* @brief  oem 解析
*
* @param[in] cmd OEM_KV_READ:kv 获取config 并解析 OEM_UF_READ：uf 获取config 并解析 OEM_KV_UF_READ：从KV读取 写入UF并解析
* @return 
*/
OPERATE_RET tfm_oem_cfg_handle(OEM_HANDLE_CMD cmd, VOID* arg);


/**
* @brief tfm_oem_cfg_handle_uf_write
*
* @param[in] p_file_name
* @param[in] p_data
* @param[in] len
* @return 
*/
OPERATE_RET tfm_oem_cfg_handle_uf_write(CHAR_T *uf_name, CHAR_T *p_data, USHORT_T len);
#endif