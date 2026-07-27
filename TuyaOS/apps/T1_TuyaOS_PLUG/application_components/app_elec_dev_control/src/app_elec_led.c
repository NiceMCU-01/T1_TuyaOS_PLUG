/**
 * @file app_elec_led.c
 * @author www.tuya.com
 * @brief app_elec_led module is used to 
 * @version 0.1
 * @date 2023-07-24
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tal_log.h"
#include "tuya_app_config.h"
#include "tuya_cloud_types.h"
#include "tuya_ws_db.h"
#include "ty_sys.h"

#include "app_elec_led.h"
#include "app_elec_channel.h"

#include "tfm_timing_storage.h"

#include "tdl_led_manage.h"
#include <stdbool.h>

/***********************************************************
************************macro define************************
***********************************************************/
#define LIGHT_MODE                  "LIGHT_MODE"

#define NOT_USE                     (-1)

// 产测路由信号弱，LED 闪烁时间
#define TEST_WEAK_SIGNAL_FLASH_MS   (100)

// 计量芯片产测时，LED 闪烁时间
#define ENERGY_METER_FLASH_MS       (1500)

// 非计量插座，长按配网，产测中指示状态
#define NOT_ENERGY_LOW_POWER_ON_MS          (250)
#define NOT_ENERGY_LOW_POWER_OFF_MS         (250)

// 非计量插座，上电配网，产测中指示状态
#define NOT_ENERGY_AUTO_CFG_ON_MS           (3*1000)
#define NOT_ENERGY_AUTO_CFG_OFF_MS          (3*1000)

#if (defined(ENABLE_TY_LOAD_OEM_PAR) && (ENABLE_TY_LOAD_OEM_PAR == 1))
#define ELEC_CHANNEL_NUM_USED   ELEC_CHANNEL_NUM
#else
#if defined(T1_PLUG_WAY_NUM)
#define ELEC_CHANNEL_NUM_USED   T1_PLUG_WAY_NUM
#else
#define ELEC_CHANNEL_NUM_USED   DEFAULT_ELEC_CHANNEL_NUM
#endif
#endif
/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    INT32_T chan_led_idx;
    UINT8_T is_use; // net led 是否正在使用中
    PVOID_T hdl; // led ctrl handle
    UINT8_T is_mux; // 是否复用
    UINT8_T nety_status; // 联网后 LED 状态
    UINT8_T netn_status; // 未联网时 LED 状态
    ELEC_NET_INDICATE_MODE_E mode;
} ELEC_NET_LED_T;

typedef struct {
    PVOID_T hdl; // led ctrl handle
    UINT8_T light_mode_dpid;
    ELEC_LIGHT_MODE_E light_mode;
    UINT8_T chan_status;
} ELEC_POWER_LED_T;

/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC ELEC_NET_LED_T sg_net_led = {
    .hdl = NULL,
    .is_use = 0,
    .chan_led_idx = NOT_USE,
    .mode = INDICATE_MODE_NOT_CONNECT,
#if (defined(ENABLE_ELEC_NET_INDICATOR_MUX_MODE))
    .is_mux = ENABLE_ELEC_NET_INDICATOR_MUX_MODE,
#else
    .is_mux = 0,
#endif
#if !(defined(ENABLE_ELEC_NET_INDICATOR_MUX_MODE) && (ENABLE_ELEC_NET_INDICATOR_MUX_MODE==1))
    .netn_status = ELEC_NET_LED_NOT_CONNECT,
    .nety_status = ELEC_NET_LED_CONNECTED,
#endif
};

STATIC ELEC_POWER_LED_T sg_power_led = {
    .hdl = NULL,
    .chan_status = 0,
#if (defined(ELEC_LIGHT_MODE_DPID))
    .light_mode_dpid = ELEC_LIGHT_MODE_DPID,
#endif
#if (defined(ELEC_LIGHT_MODE))
    .light_mode = ELEC_LIGHT_MODE,
#endif
};


// 通道 LED 名称
STATIC CHAR_T *chan_led_name[ELEC_CHANNEL_NUM_USED] ={
#ifdef ELEC_CHANNEL_1_LED_NAME
    ELEC_CHANNEL_1_LED_NAME,
#else
    NULL,
#endif
#ifdef ELEC_CHANNEL_2_LED_NAME
    ELEC_CHANNEL_2_LED_NAME,
#else
    // NULL,
#endif
#ifdef ELEC_CHANNEL_3_LED_NAME
    ELEC_CHANNEL_3_LED_NAME,
#else
    // NULL,
#endif
#ifdef ELEC_CHANNEL_4_LED_NAME
    ELEC_CHANNEL_4_LED_NAME,
#else
    // NULL,
#endif
#ifdef ELEC_CHANNEL_5_LED_NAME
    ELEC_CHANNEL_5_LED_NAME,
#else
    // NULL,
#endif
#ifdef ELEC_CHANNEL_6_LED_NAME
    ELEC_CHANNEL_6_LED_NAME,
#else
    // NULL,
#endif
#ifdef ELEC_CHANNEL_7_LED_NAME
    ELEC_CHANNEL_7_LED_NAME,
#else
    // NULL,
#endif
#ifdef ELEC_CHANNEL_8_LED_NAME
    ELEC_CHANNEL_8_LED_NAME,
#else
    // NULL,
#endif
};

/***********************************************************
***********************function define**********************
***********************************************************/
#if defined (ENABLE_ELEC_COMPATIBLE_DATA_FORMAT) && (ENABLE_ELEC_COMPATIBLE_DATA_FORMAT == 1)
#define LIGHT_MODE_SAVE "light_mode_save"
#define DEF_LIGHT_MODE  "def_light_mode"

/**
 * @brief 读取兼容旧数据格式的网络指示灯数据
 *
 * @param[in] : ELEC_LIGHT_MODE_E *mode
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_net_led_data_compatible(ELEC_LIGHT_MODE_E *mode)
{
    OPERATE_RET rt = OPRT_OK;
    BYTE_T *buff = NULL;
    UINT_T buff_len = 0;
    OPERATE_RET op_ret = OPRT_OK;
    BOOL_T exist = FALSE;
    
    //检查是否存在老的key
    rt = wd_common_exist(LIGHT_MODE_SAVE, &exist);
    if (OPRT_OK != rt || exist == FALSE) {
        return OPRT_OK;
    }

    op_ret = wd_common_read(LIGHT_MODE_SAVE, &buff, &buff_len);
    if (OPRT_OK != op_ret) {
        TAL_PR_NOTICE("old light mode is null!!");
        return OPRT_OK;
    }
    TAL_PR_DEBUG("wd_common_read: %s", buff);

    ty_cJSON *root = NULL;
    root = ty_cJSON_Parse((CHAR_T *)buff);
    Free(buff);
    buff = NULL;
    if (NULL == root) {
        TAL_PR_ERR("ty_cJSON parse err");
        return OPRT_CJSON_PARSE_ERR;
    }

    ty_cJSON *js_json = ty_cJSON_GetObjectItem(root, DEF_LIGHT_MODE);
    if (NULL == js_json) {
        ty_cJSON_Delete(root);
        root = NULL;
        return OPRT_CJSON_GET_ERR;
    }

    *mode = js_json->valueint;
    //重新写成新的key
    rt = tfm_kv_uf_storage_write_data(LIGHT_MODE, mode, SIZEOF(ELEC_LIGHT_MODE_E));
    //写入成功删除老key
    if (OPRT_OK == rt) {
        TUYA_CALL_ERR_LOG(wd_common_delete(LIGHT_MODE_SAVE));
    }

    ty_cJSON_Delete(root);
    root = NULL;
    return rt;
}
#endif
/**
 * @brief  LED 初始化始化
 *
 * @param  none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_net_led_init(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    UINT8_T i = 0;
    TDL_LED_CONFIG_T led_ctrl = {
        .start_stat = TDL_LED_OFF,
    };

    // 网络指示灯初始化
    rt = (tdl_led_dev_find(ELEC_NET_LED_NAME, &sg_net_led.hdl));
    if (OPRT_OK != rt) {
#ifdef ELEC_POWER_LED_NAME
        // 没有网络指示灯，查找总控指示灯
        rt = (tdl_led_dev_find(ELEC_POWER_LED_NAME, &sg_net_led.hdl));
        if (OPRT_OK != rt) {
#endif
            // 如果总控指示灯和网络指示灯都没有，从通道指示灯 1-8 开始查找
            for (i=0; i<ELEC_CHANNEL_NUM_USED; i++) {
                if (NULL != chan_led_name[i]) {
                    rt = (tdl_led_dev_find(chan_led_name[i], &sg_net_led.hdl));
                    if (OPRT_OK == rt) {
                        sg_net_led.chan_led_idx = i;
                        app_elec_net_led_mux_set(1);
                        break;
                    }
                }
            }
#ifdef ELEC_POWER_LED_NAME
        }
#endif
    }

    if (NULL != sg_net_led.hdl) {
        TUYA_CALL_ERR_RETURN(tdl_led_ctrl(sg_net_led.hdl, &led_ctrl));
    } else {
        TAL_PR_ERR("not net led");
        rt = OPRT_COM_ERROR;
    }

    return rt;
}

STATIC VOID_T __app_net_led_use_status_set(UINT8_T is_use)
{

    sg_net_led.is_use = is_use;

    if (is_use == 0) {
        if (sg_net_led.chan_led_idx == NOT_USE) {
            // 复用总控指示灯
            app_elec_power_led_status_set(sg_power_led.chan_status);
        } else {
            // 复用通道继电器指示灯
            app_elec_chan_led_handle_set(sg_net_led.chan_led_idx+1, sg_net_led.hdl);
        }
    } else {
        if (sg_net_led.chan_led_idx != NOT_USE) {
            app_elec_chan_led_handle_set(sg_net_led.chan_led_idx+1, NULL);
        }
    }

    return;
}

/**
 * @brief             设置指示模式
 *
 * @param[in] :    indicate_mode            灯光变化模式
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_net_led_mode_set(ELEC_NET_INDICATE_MODE_E indicate_mode)
{
    OPERATE_RET rt = OPRT_OK;

    TDL_LED_CONFIG_T led_ctrl = {0};

    switch(indicate_mode) {
        case (INDICATE_MODE_NOT_CONNECT):
            if (sg_net_led.is_mux) {
                led_ctrl.stat = TDL_LED_OFF;
                TUYA_CALL_ERR_RETURN(tdl_led_ctrl(sg_net_led.hdl, &led_ctrl));
                __app_net_led_use_status_set(1);
            } else {
                __app_net_led_use_status_set(1);
                if (sg_net_led.netn_status == LED_ALWAYS_ON) {
                    led_ctrl.stat = TDL_LED_ON;
                } else if (sg_net_led.netn_status == LED_RELAY_STATE) {
                    led_ctrl.stat = (sg_power_led.chan_status==0) ? (TDL_LED_OFF) : (TDL_LED_ON);
                } else {
                    led_ctrl.stat = TDL_LED_OFF;
                }
                TUYA_CALL_ERR_RETURN(tdl_led_ctrl(sg_net_led.hdl, &led_ctrl));
            }
        break;
        case (INDICATE_MODE_CONNECTED):
            if (sg_net_led.is_mux) {
                led_ctrl.stat = TDL_LED_ON;
                TUYA_CALL_ERR_RETURN(tdl_led_ctrl(sg_net_led.hdl, &led_ctrl));
                __app_net_led_use_status_set(1);
            } else {
                __app_net_led_use_status_set(1);
                if (sg_net_led.nety_status == LED_ALWAYS_ON) {
                    led_ctrl.stat = TDL_LED_ON;
                } else if (sg_net_led.nety_status == LED_RELAY_STATE) {
                    led_ctrl.stat = (sg_power_led.chan_status==0) ? (TDL_LED_OFF) : (TDL_LED_ON);
                } else {
                    led_ctrl.stat = TDL_LED_OFF;
                }
                TUYA_CALL_ERR_RETURN(tdl_led_ctrl(sg_net_led.hdl, &led_ctrl));
            }
        break;
        case (INDICATE_MODE_AP):
            led_ctrl.stat = TDL_LED_FLASH;
            led_ctrl.start_stat = TRUE;
            led_ctrl.end_stat = FALSE;
            led_ctrl.flash_cnt = TDL_FLASH_FOREVER;
            led_ctrl.flash_first_time = ELEC_NET_LED_AP_FLASH_MS;
            led_ctrl.flash_second_time = ELEC_NET_LED_AP_FLASH_MS;
            TUYA_CALL_ERR_RETURN(tdl_led_ctrl(sg_net_led.hdl, &led_ctrl));
            __app_net_led_use_status_set(1);
        break;
        case (INDICATE_MODE_EZ):
            led_ctrl.stat = TDL_LED_FLASH;
            led_ctrl.start_stat = TRUE;
            led_ctrl.end_stat = FALSE;
            led_ctrl.flash_cnt = TDL_FLASH_FOREVER;
            led_ctrl.flash_first_time = ELEC_NET_LED_EZ_FLASH_MS;
            led_ctrl.flash_second_time = ELEC_NET_LED_EZ_FLASH_MS;
            TUYA_CALL_ERR_RETURN(tdl_led_ctrl(sg_net_led.hdl, &led_ctrl));
            __app_net_led_use_status_set(1);
        break;
        case (INDICATE_MODE_WEAK_SIGNAL):
            led_ctrl.stat = TDL_LED_FLASH;
            led_ctrl.start_stat = TRUE;
            led_ctrl.end_stat = FALSE;
            led_ctrl.flash_cnt = TDL_FLASH_FOREVER;
            led_ctrl.flash_first_time = TEST_WEAK_SIGNAL_FLASH_MS;
            led_ctrl.flash_second_time = TEST_WEAK_SIGNAL_FLASH_MS;
            TUYA_CALL_ERR_RETURN(tdl_led_ctrl(sg_net_led.hdl, &led_ctrl));
            __app_net_led_use_status_set(1);
        break;
        case (INDICATE_MODE_ENERGY_MONITOR):
            led_ctrl.stat = TDL_LED_FLASH;
            led_ctrl.start_stat = TRUE;
            led_ctrl.end_stat = FALSE;
            led_ctrl.flash_cnt = TDL_FLASH_FOREVER;
            led_ctrl.flash_first_time = ENERGY_METER_FLASH_MS;
            led_ctrl.flash_second_time = ENERGY_METER_FLASH_MS;
            TUYA_CALL_ERR_RETURN(tdl_led_ctrl(sg_net_led.hdl, &led_ctrl));
            __app_net_led_use_status_set(1);
        break;
        case (INDICATE_MODE_PROD_TEST_SUCCESS):
            led_ctrl.stat = TDL_LED_ON;
            TUYA_CALL_ERR_RETURN(tdl_led_ctrl(sg_net_led.hdl, &led_ctrl));
            __app_net_led_use_status_set(1);
        break;
        case (INDICATE_MODE_PROD_TEST_FAIL):
            led_ctrl.stat = TDL_LED_OFF;
            TUYA_CALL_ERR_RETURN(tdl_led_ctrl(sg_net_led.hdl, &led_ctrl));
            __app_net_led_use_status_set(1);
        break;
        case (INDICATE_MODE_PT_NOT_ENERGY_LOW_POWER) :{
            led_ctrl.stat = TDL_LED_FLASH;
            led_ctrl.start_stat = TRUE;
            led_ctrl.end_stat = FALSE;
            led_ctrl.flash_cnt = TDL_FLASH_FOREVER;
            led_ctrl.flash_first_time = NOT_ENERGY_LOW_POWER_ON_MS;
            led_ctrl.flash_second_time = NOT_ENERGY_LOW_POWER_OFF_MS;
            TUYA_CALL_ERR_RETURN(tdl_led_ctrl(sg_net_led.hdl, &led_ctrl));
            __app_net_led_use_status_set(1);
        } break;
        case (INDICATE_MODE_PT_NOT_ENERGY_AUTO_CFG) :{
            led_ctrl.stat = TDL_LED_FLASH;
            led_ctrl.start_stat = TRUE;
            led_ctrl.end_stat = FALSE;
            led_ctrl.flash_cnt = TDL_FLASH_FOREVER;
            led_ctrl.flash_first_time = NOT_ENERGY_AUTO_CFG_ON_MS;
            led_ctrl.flash_second_time = NOT_ENERGY_AUTO_CFG_OFF_MS;
            TUYA_CALL_ERR_RETURN(tdl_led_ctrl(sg_net_led.hdl, &led_ctrl));
            __app_net_led_use_status_set(1);
        } break;
        case (INDICATE_MODE_PT_NOT_ENERGY_BUTTON) : {
            if (sg_net_led.is_mux) {
                __app_net_led_use_status_set(0);
            }
            // 非复用则无需对配网指示灯做任何操作
        } break;
        case (INDICATE_MODE_ALWAYS_ON) : {
            led_ctrl.stat = TDL_LED_ON;
            TUYA_CALL_ERR_RETURN(tdl_led_ctrl(sg_net_led.hdl, &led_ctrl));
            __app_net_led_use_status_set(1);
        } break;
        case (INDICATE_MODE_ALWAYS_OFF) : {
            led_ctrl.stat = TDL_LED_OFF;
            TUYA_CALL_ERR_RETURN(tdl_led_ctrl(sg_net_led.hdl, &led_ctrl));
            __app_net_led_use_status_set(1);
        } break;
        default : return OPRT_INVALID_PARM;
    }

    sg_net_led.mode = indicate_mode;

    return rt;
}
OPERATE_RET app_elec_net_led_ffc_beacon_set(UINT8_T led_stat)
{
    OPERATE_RET rt = OPRT_OK;

    TDL_LED_CONFIG_T led_ctrl = {0};
    led_ctrl.stat = led_stat;
    TUYA_CALL_ERR_RETURN(tdl_led_ctrl(sg_net_led.hdl, &led_ctrl));

    return rt;
}

OPERATE_RET app_elec_net_led_refresh(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    
    TAL_PR_DEBUG("sg_net_led.mode~~~~~~~~~ = %d",sg_net_led.mode);
    TUYA_CALL_ERR_RETURN(app_elec_net_led_mode_set(sg_net_led.mode));

    return rt;
}

VOID_T app_net_led_ffc_beacon_use_set(UINT8_T is_use)
{
    __app_net_led_use_status_set(is_use);
}

VOID_T app_elec_net_led_mux_set(UINT8_T is_mux)
{
    sg_net_led.is_mux = is_mux;
    return;
}

UINT8_T app_elec_net_led_mux_get(VOID_T)
{
    return sg_net_led.is_mux;
}

VOID_T app_elec_net_led_status_set(UINT8_T nety_status, UINT8_T netn_status)
{
    sg_net_led.nety_status = nety_status;
    sg_net_led.netn_status = netn_status;
    return;
}

OPERATE_RET app_elec_power_led_init(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    TDL_LED_CONFIG_T led_ctrl = {
        .start_stat = TDL_LED_OFF,
    };

#if defined (ENABLE_ELEC_COMPATIBLE_DATA_FORMAT) && (ENABLE_ELEC_COMPATIBLE_DATA_FORMAT == 1)
    app_elec_net_led_data_compatible(&sg_power_led.light_mode);
#endif

    // 获取指示灯状态
    TUYA_CALL_ERR_LOG(tfm_kv_uf_storage_read_data(LIGHT_MODE, &sg_power_led.light_mode, SIZEOF(ELEC_LIGHT_MODE_E)));


    // 总控指示灯初始化
    if (1 == sg_net_led.is_mux) {
        sg_power_led.hdl = sg_net_led.hdl;
    } else {
#ifdef ELEC_POWER_LED_NAME
        rt = tdl_led_dev_find(ELEC_POWER_LED_NAME, &sg_power_led.hdl);
        if (OPRT_OK != rt) {
            // 插排才需要总控指示灯
            TAL_PR_DEBUG("not find total led");
            return OPRT_OK;
        }
        led_ctrl.start_stat = TDL_LED_OFF;
        TUYA_CALL_ERR_RETURN(tdl_led_ctrl(sg_power_led.hdl, &led_ctrl));
#else
        // 插排才需要总控指示灯
        TAL_PR_DEBUG("not find total led");
#endif
    }

    return rt;
}

OPERATE_RET app_elec_power_led_status_set(UINT8_T chan_status)
{
    OPERATE_RET rt = OPRT_OK;
    TDL_LED_CONFIG_T led_ctrl = {0};

    sg_power_led.chan_status = chan_status;

    if (sg_net_led.is_mux) {
        // 本项目中复用灯只作为配网指示灯使用，非配网状态不再显示总控状态
        return OPRT_OK;
    }

    if (sg_power_led.hdl == NULL) {
        // 单孔插座没有总控指示灯
        return OPRT_OK;
    }

    switch (sg_power_led.light_mode) {
        case (LIGHT_MODE_RELAY):
            led_ctrl.stat = (chan_status == 0) ? (TDL_LED_OFF) : (TDL_LED_ON);
        break;
        case LIGHT_MODE_POSITION:
            led_ctrl.stat = (chan_status == 0) ? (TDL_LED_ON) : (TDL_LED_OFF);
        break;
        case LIGHT_MODE_ALWAYS_OFF:
            led_ctrl.stat = TDL_LED_OFF;
        break;
        case LIGHT_MODE_ALWAYS_ON:
            led_ctrl.stat = TDL_LED_ON;
        break;
        default: 
        TAL_PR_ERR("Unknow light mode: %d", sg_power_led.light_mode);
        return OPRT_COM_ERROR;
    }

    TUYA_CALL_ERR_RETURN(tdl_led_ctrl(sg_power_led.hdl, &led_ctrl));

    // 当配网指示灯独立指示，并且指示继电器状态时，继电器状态改变，这里应该同步改变
    if (0 == sg_net_led.is_mux && \
        (INDICATE_MODE_NOT_CONNECT == sg_net_led.mode || INDICATE_MODE_CONNECTED == sg_net_led.mode)) {
        app_elec_net_led_mode_set(sg_net_led.mode);
    }

    return rt;
}

OPERATE_RET app_elec_power_led_mode_set(ELEC_LIGHT_MODE_E light_mode)
{
    OPERATE_RET rt = OPRT_OK;

    sg_power_led.light_mode = light_mode;

    TUYA_CALL_ERR_LOG(tfm_kv_uf_storage_write_data(LIGHT_MODE, &sg_power_led.light_mode, SIZEOF(ELEC_LIGHT_MODE_E)));
    TUYA_CALL_ERR_LOG(app_elec_power_led_mode_upload());

    TUYA_CALL_ERR_RETURN(app_elec_power_led_status_set(sg_power_led.chan_status));

    return rt;
}

ELEC_LIGHT_MODE_E app_elec_power_led_mode_get(VOID_T)
{
    return sg_power_led.light_mode;
}

OPERATE_RET app_elec_power_led_mode_upload(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    TY_OBJ_DP_S dp_obj_data = {0};

    dp_obj_data.dpid = sg_power_led.light_mode_dpid;
    dp_obj_data.type = PROP_ENUM;
    dp_obj_data.value.dp_enum = sg_power_led.light_mode;
    dp_obj_data.time_stamp = 0;

    TAL_PR_DEBUG("light mode dpid: %d, enum: %d", sg_power_led.light_mode_dpid, sg_power_led.light_mode);

    TUYA_CALL_ERR_LOG(dev_report_dp_json_async(NULL, &dp_obj_data, 1));

    return rt;
}

OPERATE_RET app_elec_power_led_mode_data_erase(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;

    TUYA_CALL_ERR_RETURN(tfm_kv_uf_storage_erase_data(LIGHT_MODE));

    return rt;
}


OPERATE_RET app_elec_no_energy_prod_led(VOID_T)
{
    OPERATE_RET rt = OPRT_NOT_FOUND;
    // 单路+不复用+无总控灯+无继电器灯时，产测网络指示灯跟随继电器动作
    if (0 == sg_net_led.is_mux) {
        if((NULL == sg_power_led.hdl) &&(NULL != sg_net_led.hdl)){
            if (NULL != chan_led_name[0]) {
                if(OPRT_OK != tdl_led_dev_find(chan_led_name[0], &sg_net_led.hdl)){
                    sg_power_led.hdl = sg_net_led.hdl;
                    rt = OPRT_OK;
                }            
            }else{
                sg_power_led.hdl = sg_net_led.hdl;
                rt = OPRT_OK;
            }
        }
    } 
    return rt;  
}
