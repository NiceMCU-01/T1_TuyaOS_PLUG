/**
 * @file app_elec_delay_off_timer.h
 * @author www.tuya.com
 * @brief app_elec_delay_off_timer module is used to 
 * @version 0.1
 * @date 2023-04-19
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __APP_ELEC_DELAY_OFF_TIMER_H__
#define __APP_ELEC_DELAY_OFF_TIMER_H__

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
#pragma pack(1)
typedef struct {
    UINT8_T enable:1;
    UINT8_T channel:7;
    UINT16_T time_s;
} APP_ELEC_DELAY_OFF_RAW_DATA_T;
#pragma pack()

typedef struct {
    UINT8_T num;
    APP_ELEC_DELAY_OFF_RAW_DATA_T raw_data[0];
} APP_ELEC_DELAY_OFF_DATA_T;


typedef VOID_T (*APP_ELEC_DELAY_OFF_CB)(UINT8_T channel_id);

/***********************************************************
********************function declaration********************
***********************************************************/
/**
 * @brief 点动开关初始化函数
 *
 * @param[in] cb: 点动开关状态变化后回调函数
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_delay_off_init(APP_ELEC_DELAY_OFF_CB cb);

/**
 * @brief 点动开关解析函数
 *
 * @param[in] dp_string: 平台下发点动开关的 DP 数据
 * @param[out] delay_off_data: DP 数据解析结果，开发者使用结束后需调用 tal_free() 函数释放 delay_off_data 的内存
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_delay_off_dp_data_parse(CHAR_T *dp_string, APP_ELEC_DELAY_OFF_DATA_T **delay_off_data);

/**
 * @brief 覆盖设置点动开关数据
 *
 * @param[in] data: 点动开关配置数据
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_delay_off_override_set(APP_ELEC_DELAY_OFF_DATA_T *data);

/**
 * @brief 开启点动开关
 *
 * @param[in] channel_id: 开启的通道 ID （0： 所有通道；1-8：通道1-通道8）
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_delay_off_start(UINT8_T channel_id);

/**
 * @brief 擦除点动开关本地存储数据
 *
 * @param[in] none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_delay_off_data_erase(VOID_T);

/**
 * @brief 获取点动开关数据，获取后 p_data 若不为 NULL 应调用 tal_free()
 *
 * @param[in] p_data: 点动开关 base64 后的数据
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_delay_off_str_get(CHAR_T **p_data);

/**
 * @brief 点动开关数据上报
 *
 * @param[in] none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_delay_off_dp_data_upload(VOID_T);

#ifdef __cplusplus
}
#endif

#endif /* __APP_ELEC_DELAY_OFF_TIMER_H__ */
