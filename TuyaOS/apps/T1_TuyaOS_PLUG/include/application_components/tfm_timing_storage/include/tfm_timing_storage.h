/**
 * @file tfm_timing_storage.h
 * @author www.tuya.com
 * @brief light_data_save module is used to start timer and create list to storage data
 * @version 0.1
 * @date 2023-02-08
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __TFM_TIMING_STORAGE_H__
#define __TFM_TIMING_STORAGE_H__

#include "tuya_cloud_types.h" 

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
*********************** macro define ***********************
***********************************************************/

/***********************************************************
********************** typedef define **********************
***********************************************************/

typedef VOID_T (*TFM_STORAGE_DATA_CB)(CHAR_T *p_key, VOID_T *arg);

/***********************************************************
******************* function declaration *******************
***********************************************************/
/**
 * @brief         kv-擦除数据
 *
 * @param[in]     key          存储索引
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tfm_kv_uf_storage_erase_data(CHAR_T *key);
/**
 * @brief         kv-写入数据
 *
 * @param[in]     key          存储索引
 * @param[in]     p_value      数据指针
 * @param[in]     len          数据长度 
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tfm_kv_uf_storage_write_data(CHAR_T *key, IN UCHAR_T *p_value, UINT_T len);
/**
 * @brief        KV-读取数据
 *
 * @param[in]     key          存储索引
 * @param[out]    p_buf        数据缓存地址    
 * @param[in]     buf_len      缓存大小     
 * 
 * @return read_len  读取数据长度
 */
UINT_T tfm_kv_uf_storage_read_data(CHAR_T *key, OUT UCHAR_T *p_buf, UINT_T buf_len);
/**
 * @brief        读取数据
 *
 * @param[in]     key          存储索引
 * @param[out]    p_buf        数据缓存地址    
 * @param[in]     buf_len      缓存大小     
 * 
 * @return read_len  读取数据长度
 */
UINT_T tfm_storage_read_data(CHAR_T *key, OUT UCHAR_T *p_buf, UINT_T buf_len);

/**
 * @brief         读取数据
 *
 * @param[in]     key          存储索引
 * @param[in]     p_value      数据指针
 * @param[in]     len          数据长度 
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tfm_storage_write_data(CHAR_T *key, IN UCHAR_T *p_value, UINT_T len);

/**
 * @brief         擦除数据
 *
 * @param[in]     key          存储索引
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tfm_storage_erase_data(CHAR_T *key);

/**
 * @brief          存储定时器初始化
 *
 * @param[in] :  time_ms         定时时间
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tfm_timing_storage_init(USHORT_T time_ms);

/**
 * @brief           注册定时存储函数
 *
 * @param[in] :   p_key        存储索引地址
 * @param[in] :   cb           定时时间到后，调用的存储回调
 * @param[in] :   arg          回调返回时参数
 *  
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tfm_timing_storage_register(CHAR_T *p_key, TFM_STORAGE_DATA_CB cb, VOID_T *arg);

/**
 * @brief           启动存储定时器
 *
 * @param[in] :   p_key        存储索引地址
 *  
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tfm_storage_start_timer(CHAR_T *p_key);

/**
 * @brief           取消定时存储
 *
 * @param[in] :   p_key        存储索引地址
 *  
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tfm_storage_cancel_timer(CHAR_T *p_key);

/**
 * @brief           停止存储定时器
 *
 * @param  none 
 *  
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tfm_storage_stop_timer(VOID_T);


#ifdef __cplusplus
}
#endif

#endif /* __TFM_TIMING_STORAGE_H__ */
