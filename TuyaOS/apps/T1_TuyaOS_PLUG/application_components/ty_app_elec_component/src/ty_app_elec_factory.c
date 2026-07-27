/**
 * @file ty_app_elec_factory.c
 * @author www.tuya.com
 * @brief ty_app_elec_factory module is used to 
 * @version 0.1
 * @date 2023-08-01
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_app_config.h"
#include "ty_sys.h"

#include "ty_app_elec_factory.h"

#if (defined(ENERGY_MONITOR_ENABLE) && (ENERGY_MONITOR_ENABLE==1))
#include "app_elec_button.h"
#include "ty_app_elec_trigger.h"
#include "tfm_timing_storage.h"
#endif

#include "app_elec_led.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define ENERGY_CAL_RESULT   "measure_rslt"
#define ENERGY_CAL_VALUE    "measure_coe"

// 产测结果
#define ENERGY_CAL_SUCCESS  1
#define ENERGY_CAL_FAIL     2

/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC ELEC_PROD_TEST_STATUS_T sg_test_status = PROD_TEST_UNKNOW;

#if (defined(ENERGY_MONITOR_ENABLE) && (ENERGY_MONITOR_ENABLE==1))
STATIC ENERGY_MONITOR_HANDLE_T sg_energy_monitor_hdl = NULL;

    // 产测数据
STATIC UINT8_T sg_err_threshold = ENERGY_MONITOR_CAL_ERR_PERCENT;

// 产测校准环境数据
STATIC ENERGY_MONITOR_CAL_DATA_T sg_cal_data = {
    .voltage  = ENERGY_MONITOR_CAL_VOLTAGE,
    .current  = ENERGY_MONITOR_CAL_CURRENT,
    .power    = ENERGY_MONITOR_CAL_POWER,
    .resval   = ENERGY_MONITOR_SAMPLE_RESISTOR,
};
#endif

/***********************************************************
***********************function define**********************
***********************************************************/

/************************************************************
 *                       产测状态
 ************************************************************/
ELEC_PROD_TEST_STATUS_T ty_app_elec_prod_test_status_get(VOID_T)
{
    UINT_T ret_len = 0;
    UINT8_T result = 0;

    if (sg_test_status == PROD_TEST_UNKNOW) {
        ret_len = tfm_kv_uf_storage_read_data(ENERGY_CAL_RESULT, &result, SIZEOF(UINT8_T));
        if (0 == ret_len) {
            TAL_PR_DEBUG("energy monitor not tested");
            sg_test_status = PROD_NOT_TESTED;
        } else {
            TAL_PR_DEBUG("energy monitor test result: %d", result);
            sg_test_status = (ENERGY_CAL_SUCCESS==result) ? (PROD_TEST_SUCCESS) : (PROD_TEST_FAILED);
        }
    }

    return sg_test_status;
}

VOID_T app_elec_prod_test_status_set(ELEC_PROD_TEST_STATUS_T test_status)
{
    sg_test_status = test_status;
    return;
}

/************************************************************
 *                       计量产测
 ************************************************************/
#if (defined(ENERGY_MONITOR_ENABLE) && (ENERGY_MONITOR_ENABLE==1))
OPERATE_RET app_elec_energy_monitor_prod_test(VOID_T)
{
    OPERATE_RET rt = OPRT_OK, cal_rt = OPRT_COM_ERROR;
    UINT8_T cal_result = ENERGY_CAL_FAIL, old_result = ENERGY_CAL_FAIL;
    UINT_T ret_len = 0;
    UINT32_T err_percent = 0;
    ENERGY_MONITOR_CAL_PARAMS_T save_data;

    sg_test_status = PROD_TESTING;

    if (NULL == sg_energy_monitor_hdl) {
        TUYA_CALL_ERR_GOTO(tdl_energy_monitor_find(ENERGY_MONITOR_NAME, &sg_energy_monitor_hdl), __EXIT);
        tdl_energy_monitor_config(sg_energy_monitor_hdl, TDL_EM_CMD_CAL_DATA_SET, &sg_cal_data);
        TUYA_CALL_ERR_GOTO(tdl_energy_monitor_open(sg_energy_monitor_hdl), __EXIT);
    }

    TAL_PR_NOTICE("sg_cal_data: %d, %d, %d, %d", sg_cal_data.voltage, sg_cal_data.current, sg_cal_data.power, sg_cal_data.resval);

    err_percent = tdl_energy_monitor_calibration(sg_energy_monitor_hdl, sg_cal_data, &save_data);

    if (err_percent > sg_err_threshold || sg_err_threshold < 0) {
        cal_result = ENERGY_CAL_FAIL;
        cal_rt = OPRT_COM_ERROR;
    } else {
       // TUYA_CALL_ERR_GOTO(tfm_storage_write_data(ENERGY_CAL_VALUE, (UCHAR_T *)&save_data, SIZEOF(ENERGY_MONITOR_CAL_PARAMS_T)), __EXIT);
        cal_result = ENERGY_CAL_SUCCESS;
        cal_rt = OPRT_OK;
    }

__EXIT:
    TAL_PR_NOTICE("energy meter monitor result: %d", cal_result);
    ret_len = tfm_kv_uf_storage_read_data(ENERGY_CAL_RESULT, &old_result, 1);
    if (0 >= ret_len || ENERGY_CAL_FAIL == old_result) {
        tfm_kv_uf_storage_write_data(ENERGY_CAL_RESULT, &cal_result, 1);
    }

    if (ENERGY_CAL_SUCCESS == old_result && (ENERGY_CAL_FAIL == cal_result)) {
        /*之前是成功的，但这次失败，不做保存*/
    } else {
        TUYA_CALL_ERR_LOG(tfm_kv_uf_storage_write_data(ENERGY_CAL_VALUE, (UCHAR_T *)&save_data, SIZEOF(ENERGY_MONITOR_CAL_PARAMS_T)));
    }

    if (ENERGY_CAL_SUCCESS == cal_result) {
        sg_test_status = PROD_TEST_SUCCESS;
    } else {
        sg_test_status = PROD_TEST_FAILED;
    }

    // 打开按键功能
    TUYA_CALL_ERR_LOG(app_elec_button_init(ty_app_elec_button_trigger));

    return cal_rt;
}

VOID_T ty_app_elec_energy_monitor_cal_data_set(ENERGY_MONITOR_CAL_DATA_T set_data)
{
    memcpy(&sg_cal_data, &set_data, SIZEOF(ENERGY_MONITOR_CAL_DATA_T));

    return;
}

VOID_T ty_app_elec_energy_monitor_cal_data_erase(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    TAL_PR_DEBUG("erase energy monitor cal data");
    TUYA_CALL_ERR_LOG(tfm_kv_uf_storage_erase_data(ENERGY_CAL_RESULT));
    TUYA_CALL_ERR_LOG(tfm_kv_uf_storage_erase_data(ENERGY_CAL_VALUE));
    return;
}
#endif

/************************************************************
 *                       非计量产测
 ************************************************************/
STATIC VOID_T __ty_app_prod_test_chan_set(APP_ELEC_CHANNEL_CFG_T *chan_cfg)
{
    OPERATE_RET rt = OPRT_OK;
    UINT_T chan_status = 0;

    // 设置通道状态
    TUYA_CALL_ERR_LOG(app_elec_channel_status_set(chan_cfg));

    // 总控指示灯状态设置
    app_elec_get_all_channel_status(&chan_status);
    TAL_PR_DEBUG("---> chan status: 0x%x", chan_status);
    app_elec_power_led_status_set(chan_status);
    return;
}

OPERATE_RET ty_app_elec_not_energy_prod_test(APP_ELEC_CHANNEL_CFG_T *chan_cfg)
{
    OPERATE_RET rt = OPRT_OK;

    TUYA_CHECK_NULL_RETURN(chan_cfg, OPRT_INVALID_PARM);

    UINT8_T i = 0;
    UINT8_T chan_num = 0;
    
    TAL_PR_DEBUG("chan_cfg->chan_id: %d", chan_cfg->chan_id);
    TUYA_CALL_ERR_LOG(app_elec_channel_config(ELEC_CHANNEL_NUM_GET, &chan_num));
   //单插： 无继电器灯和总控灯，单路对应继电器和指示灯动作3次，网络指示灯跟随继电器动作
    if((1 == chan_num) && (OPRT_OK == app_elec_no_energy_prod_led())){
        __ty_app_prod_test_chan_set(chan_cfg);
        tal_system_sleep(500);
        __ty_app_prod_test_chan_set(chan_cfg);
        tal_system_sleep(500);
        __ty_app_prod_test_chan_set(chan_cfg);
        tal_system_sleep(500);
    }else{
        if (chan_cfg->chan_id == 0) {
            TUYA_CALL_ERR_LOG(app_elec_channel_config(ELEC_CHANNEL_NUM_GET, &chan_num));
            for (i=0; i<chan_num; i++) {
                chan_cfg->chan_id = i+1;
                chan_cfg->status = STATE_TOGGLE;
                __ty_app_prod_test_chan_set(chan_cfg);
                tal_system_sleep(500);
                __ty_app_prod_test_chan_set(chan_cfg);
                tal_system_sleep(500);
                __ty_app_prod_test_chan_set(chan_cfg);
                tal_system_sleep(500);
            }
        } else {
            __ty_app_prod_test_chan_set(chan_cfg);
            tal_system_sleep(500);
            __ty_app_prod_test_chan_set(chan_cfg);
            tal_system_sleep(500);
            __ty_app_prod_test_chan_set(chan_cfg);
            tal_system_sleep(500);
        }
   }
    // 产测结束后应该删除存储数据
    app_elec_channel_data_erase();

    return rt;
}

