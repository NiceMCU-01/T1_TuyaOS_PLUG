/**
 * @file app_elec_energy_monitor.h
 * @author www.tuya.com
 * @brief app_elec_energy_monitor module is used to 
 * @version 0.1
 * @date 2023-03-24
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __APP_ELEC_ENERGY_MONITOR_H__
#define __APP_ELEC_ENERGY_MONITOR_H__

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


/***********************************************************
********************function declaration********************
***********************************************************/

VOID_T app_elec_overcharge_power_get(VOID_T);

VOID_T app_elec_energy_monitor_enable_set(UINT8_T is_enable);

UINT8_T app_elec_energy_monitor_enable_get(VOID_T);

/**
 * @brief 电量统计功能初始化
 *
 * @param[in] none: 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_energy_monitor_init(VOID_T);

/**
 * @brief 采样电阻设置，如果需要设置应在 app_elec_energy_monitor_init() 前进行设置
 *
 * @param[in] sample_res: 采样电阻大小，单位 mR
 *
 * @return none
 */
VOID_T app_elec_energy_monitor_cal_data_set(ENERGY_MONITOR_CAL_DATA_T cal_data);

/**
 * @brief 上报实时电压，电流，功率
 *
 * @param[in] none: 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_energy_monitor_pvi_upload(VOID_T);

/**
 * @brief 增加电量上报
 *
 * @param[in] none: 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T app_elec_energy_monitor_add_energy_upload(VOID_T);

/**
 * @brief 上报计量校准参数
 *
 * @param[in] none: 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_energy_monitor_coe_upload(VOID_T);

/**
 * @brief 获取计量校准是否成功
 *
 * @param[in] none: 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
BOOL_T app_elec_energy_monitor_coe_result_get(VOID_T);

/**
 * @brief 擦除存储的所有数据
 *
 * @param[in] none: 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_energy_monitor_data_easer(VOID_T);

/**
 * @brief 0942计量芯片过零信号初始化
 *
 * @param[in] none: 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T app_elec_energy_monitor_set_zero(VOID_T);

#ifdef __cplusplus
}
#endif

#endif /* __APP_ELEC_ENERGY_MONITOR_H__ */
