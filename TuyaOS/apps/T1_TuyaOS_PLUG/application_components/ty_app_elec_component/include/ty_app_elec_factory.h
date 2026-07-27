/**
 * @file ty_app_elec_factory.h
 * @author www.tuya.com
 * @brief ty_app_elec_factory module is used to 
 * @version 0.1
 * @date 2023-08-01
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __TY_APP_ELEC_FACTORY_H__
#define __TY_APP_ELEC_FACTORY_H__

#include "tuya_cloud_types.h"

#include "tdl_energy_monitor_manage.h"

#include "app_elec_channel.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
typedef UINT8_T ELEC_PROD_TEST_STATUS_T;
#define PROD_TEST_UNKNOW    0
#define PROD_TEST_SUCCESS   1 // 成品产测成功
#define PROD_TEST_FAILED    2 // 成品产测失败
#define PROD_TESTING        3 // 成品产测中
#define PROD_NOT_TESTED     4 // 未进行过成品产测

/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
********************function declaration********************
***********************************************************/

/**
 * @brief 计量成品产测
 *
 * @param[in] none: 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_energy_monitor_prod_test(VOID_T);

/**
 * @brief 计量校准环境设置
 *
 * @param[in] none: 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T ty_app_elec_energy_monitor_cal_data_set(ENERGY_MONITOR_CAL_DATA_T set_data);

/**
 * @brief 获取计量设备产测结果
 *
 * @param[in] none: 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
ELEC_PROD_TEST_STATUS_T ty_app_elec_prod_test_status_get(VOID_T);

/**
 * @brief 擦除计量产测数据
 *
 * @param[in] none:
 *
 * @return none
 */
VOID_T ty_app_elec_energy_monitor_cal_data_erase(VOID_T);

/**
 * @brief 产测状态设置
 *
 * @param[in] none: 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID_T app_elec_prod_test_status_set(ELEC_PROD_TEST_STATUS_T test_status);


/**
 * @brief 非计量产测
 *
 * @param[in] none: 
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_elec_not_energy_prod_test(APP_ELEC_CHANNEL_CFG_T *chan_cfg);

#ifdef __cplusplus
}
#endif

#endif /* __TY_APP_ELEC_FACTORY_H__ */
