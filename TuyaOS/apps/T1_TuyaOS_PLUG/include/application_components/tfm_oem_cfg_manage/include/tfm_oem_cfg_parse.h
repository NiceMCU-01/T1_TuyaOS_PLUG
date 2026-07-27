/**
* @file tfm_oem_cfg_parse.h
* @author www.tuya.com
* @brief tbs_oem_cfg_analysis module is used to declare oem config analysis operate interface
* @version 0.1
* @date 2021-08-27
*
* @copyright Copyright (c) tuya.inc 2021
*
*/
 
#ifndef __TFM_OEM_CFG_PARSE_H__
#define __TFM_OEM_CFG_PARSE_H__

#include "tuya_cloud_types.h" 
 
#ifdef __cplusplus
extern "C" {
#endif
 
/***********************************************************
*************************micro define***********************
***********************************************************/
typedef UCHAR_T TFM_OEM_CFG_TP_E;
#define OEM_CFG_TP_HEX      0
#define OEM_CFG_TP_VALUE    1
#define OEM_CFG_TP_STRING   2 
#define OEM_CFG_TP_COMBINE  3

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    CHAR_T *p_string;
    INT_T value;
} TFM_OEM_CFG_STR_MAP_S;

typedef struct {
    CHAR_T *key;                           //需要转换的key值
    TFM_OEM_CFG_STR_MAP_S *table;    //key 对应string 映射的值
    USHORT_T nums;                      //表数量
} TFM_OEM_CFG_MAP_TABLE_S;

typedef struct {
    CHAR_T* p_key;
    TFM_OEM_CFG_TP_E type;
} TFM_OEM_CFG_KEY_TP_S;

typedef struct {
    CHAR_T* p_key;
    INT_T value;
} TFM_USER_STORE_TABLE_S;
 
/***********************************************************
***********************variable define**********************
***********************************************************/
 
 
/***********************************************************
***********************function define**********************
***********************************************************/

/**
* @brief  获取组合数据对应行、列的数据内容
*
* @param[in] p_key
* @param[in] row_index
* @param[in] colum_index
* @param[in] type
* @param[out]  p_out
* @param[in] out_size
* @return 
*/
OPERATE_RET tfm_oem_cfg_parse_combine_colum_value_get(CHAR_T* p_key, USHORT_T row_index, USHORT_T colum_index, TFM_OEM_CFG_TP_E type, VOID* p_out, USHORT_T out_size);

/**
* @brief 获取key 对应的组合数据函数和列数
*
* @param[in] p_key
* @param[out] row_nums
* @param[out] colum_nums
* @return 
*/
OPERATE_RET tfm_oem_cfg_parse_combine_info_get(CHAR_T* p_key, USHORT_T* row_nums, USHORT_T* colum_nums);

/**
* @brief 修改某个key 对应的数据/插入某个key
*
* @param[in] p_key
* @param[in] type
* @param[in] p_data
* @param[in] data_size
* @return 
*/
OPERATE_RET tfm_oem_cfg_parse_key_set(CHAR_T *p_key, TFM_OEM_CFG_TP_E type, VOID* p_data, USHORT_T data_size);

/**
* @brief key对应string值映射成value
*        type -> OEM_CFG_TP_VALUE
* @param[in] p_table
* @param[in] nums
* @return 
*/
OPERATE_RET tfm_oem_cfg_parse_str_to_value_appoint(TFM_OEM_CFG_MAP_TABLE_S *p_table, USHORT_T nums);

/**
* @brief  指定key 按照某种类型解析，若不指定，则默认解析string、十进制
*         
* @param[in] p_table
* @param[in] nums
* @return 
*/
OPERATE_RET tfm_oem_cfg_parse_key_type_appoint(TFM_OEM_CFG_KEY_TP_S *p_table, USHORT_T nums);

/**
* @brief 解析oem json字段
*        
* @param[in] data_buff
* @param[in] data_len
* @return 
*/
OPERATE_RET tfm_oem_cfg_parse(CHAR_T* data_buff, UINT_T data_len);

/**
* @brief  获取数值
*
* @param[in] p_key
* @param[out] p_value
* @param[in] value_len
* @return 
*/
OPERATE_RET tfm_oem_cfg_get_value(IN CONST CHAR_T *p_key, OUT VOID *p_value, IN INT_T value_len);

/**
* @brief  获取字符串值
*
* @param[in] p_key
* @param[out] p_buf
* @return 
*/
OPERATE_RET tfm_oem_cfg_get_string(IN CONST CHAR_T *p_key, OUT CHAR_T **p_buf);

/**
* @brief  设置用户存储表
*
* @param[in] p_table
* @param[in] nums
* @return 
*/
OPERATE_RET tfm_oem_cfg_set_user_store_table(TFM_USER_STORE_TABLE_S* p_table, USHORT_T nums);

/**
* @brief free all parse item
*
* @param[in] NONE
* @return NONE
*/
VOID_T tfm_oem_cfg_parse_free(VOID_T);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*__SVC_OEM_CONFIG_ANALYSIS_H__*/

