/**
 * @file tbs_nvs.h
 * @author www.tuya.com
 * @brief tbs_nvs module is used to 
 * @version 0.1
 * @date 2022-08-06
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#ifndef __TBS_NVS_H__
#define __TBS_NVS_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TUYA_NVS_DEBUG      1//nvs 测试使能

#define NVS_ONLY_READ_SECTOR_ENABLE  0//只读区使能   当前只读区固定位于NVS第一个block(4k)


/**
 * @brief nvs init api
 * 
 * @param[in] start_addr nvs start addr
 * @param[in] end_addr nvs end addr
 * 
 * @attention: start_addr and end_addr must be 4K aligned, and must reserve 4k in addition to available space
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tbs_nvs_init(UINT_T start_addr, UINT_T end_addr);

/**
 * @brief nvs set len max api
 * 
 * @param[in] len_max len max 
 * 
 * @attention: default len max is 512, suggest len_max <= 4096
 * @return VOID_T
 */
VOID_T tbs_nvs_set_len_max(UINT_T len_max);

/**
 * @brief tbs nvs write api
 * 
 * @param[in] key: name, max length is 14 bytes
 * @param[in] value: value 
 * @param[in] len: value len, max length is 512 bytes
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tbs_nvs_write(CHAR_T *key, UCHAR_T *value, UINT_T len);

/**
 * @brief tbs nvs read api
 * 
 * @param[in] key: name, max length is 14 bytes
 * @param[in] value: value 
 * @param[in] len: value len, max length is 512 bytes
 * 
 * @return >0: value len, <=0: read error
 */
INT_T tbs_nvs_read(CHAR_T *key, UCHAR_T *value, UINT_T len);

/**
 * @brief tbs nvs erase api
 * 
 * @param[in] key: name
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tbs_nvs_erase(CHAR_T *key);


/**
 * @brief tbs nvs erase all api
 * 
 * @param[in] none
 * 
 * @return none
 */
VOID_T tbs_nvs_erase_all(VOID_T);



//只读功能未调试,不能使用
#if(NVS_ONLY_READ_SECTOR_ENABLE == 1)
//只读区-读取接口
OPERATE_RET tuya_nvs_write_once(CHAR_T *key, UCHAR_T *value, USHORT_T len);

//只读区-读取接口
int tuya_nvs_read_once(CHAR_T *key, UCHAR_T *value, USHORT_T len);
#endif


//以下接口仅供调试使用
#if(TUYA_NVS_DEBUG == 1)
//打印所有block的信息
VOID_T tbs_nvs_debug_print_block_info(VOID_T);

//写入512字节数据
VOID_T tbs_nvs_debug_write_512_byte(VOID_T);

//写入修改特定key-512字节数据
VOID tbs_nvs_debug_write_fix_key(VOID_T);

//读取特定key-512字节数据
VOID_T tbs_nvs_debug_read_fix_key(VOID_T);

//打印单个block内容
VOID tbs_nvs_debug_print_block_content(UCHAR_T block_idx);

//碎片整理使能
VOID_T tbs_nvs_debug_defrag_enable(VOID_T);

//设备重启使能
VOID_T tbs_nvs_reset_enable(VOID_T);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TBS_NVS_H__ */