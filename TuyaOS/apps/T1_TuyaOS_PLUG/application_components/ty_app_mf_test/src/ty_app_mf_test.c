/**
 * @file ty_app_mf_test.c
 * @author www.tuya.com
 * @brief ty_app_mf_test module is used to use mf test module
 * @version 0.1
 * @date 2022-12-05
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */
#include <stdio.h>
#include <string.h>

#include "tuya_cloud_types.h"
#include "tuya_cloud_com_defs.h"

#include "tal_uart.h"
#include "tal_log.h"
#include "tal_memory.h"
#include "ty_app_gpio_test.h"
#include "ty_app_mf_test.h"
#include "tuya_ble_sdk.h"
#include "tal_thread.h"
#include "tal_wifi.h"
#include "tuya_devos_utils.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define MF_GPIO_TEST 0x02

#define TY_MF_UART_NUM              TUYA_UART_NUM_0  //不要随意更改，和测架连线有关

#define PT_RESP_BUF_SIZE            128
/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    TY_MF_STATUS_CALLBACK     status_cb;
    MF_USER_PRODUCT_TEST_CB   user_prod_cb;

}TY_MF_TEST_CFG_T;

typedef OPERATE_RET(*PT_CMD_FUNC_CB)(USHORT_T cmd, UCHAR_T *data, UINT_T len, UCHAR_T **ret_data, USHORT_T *ret_len);

typedef struct {
    USHORT_T cmd;
    PT_CMD_FUNC_CB func;
} PROD_TEST_OP_S;

/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC TY_MF_TEST_CFG_T sg_mf_test_cfg = {0};

/***********************************************************
***********************function define**********************
***********************************************************/
//mf test 串口实现
STATIC VOID_T __ty_app_mf_uart_init(UINT_T baud, UINT_T bufsz)
{
    TAL_UART_CFG_T cfg;

    memset(&cfg, 0, sizeof(TAL_UART_CFG_T));

    cfg.base_cfg.baudrate = baud;
    cfg.base_cfg.databits = TUYA_UART_DATA_LEN_8BIT;
    cfg.base_cfg.parity = TUYA_UART_PARITY_TYPE_NONE;
    cfg.base_cfg.stopbits = TUYA_UART_STOP_LEN_1BIT;
    cfg.rx_buffer_size = bufsz;

    tal_uart_init(TY_MF_UART_NUM, &cfg);

    return;
}

STATIC VOID_T __ty_app_mf_uart_free(VOID)
{
    tal_uart_deinit(TY_MF_UART_NUM);

    return;
}

STATIC VOID_T __ty_app_mf_send(IN BYTE_T *data, IN CONST UINT_T len)
{
    tal_uart_write(TY_MF_UART_NUM, data, len);

    return;
}

STATIC UINT_T __ty_app_mf_recv(OUT BYTE_T *buf, IN CONST UINT_T len)
{
    return tal_uart_read(TY_MF_UART_NUM, buf, len);
}

/**
 * @brief 配置进入产测回调接口
 *
 * @return VOID_T
 *
 * @note 应用必须对其进行实现，如果不需要，则实现空函数
 */
STATIC VOID_T __ty_app_mf_user_enter_cb(VOID_T)
{
    if (sg_mf_test_cfg.status_cb) {
        sg_mf_test_cfg.status_cb(MF_STATUS_ENTER);
    }

    return;
}

/**
 * @brief 配置写入回调接口
 *
 * @return VOID_T
 *
 * @note 应用必须对其进行实现，如果不需要，则实现空函数
 */
STATIC VOID_T __ty_app_mf_user_callback(VOID_T)
{
    if (sg_mf_test_cfg.status_cb) {
        sg_mf_test_cfg.status_cb(MF_STATUS_WR_CFG_FINISH);
    }

    return;
}

/**
 * @brief pre_gpio_test gpio测试前置接口，用于对gpio测试做准备工作，
 * 例如对gpio进行重新初始化，或者是关闭gpio test，关闭gpio test的时候，
 * gpio test会返回Ture
 *
 * @return VOID_T
 *
 * @note 应用必须对其进行实现，如果不需要，则实现空函数
 */
STATIC BOOL_T __ty_app_mf_user_pre_gpio_test_cb(VOID_T)
{
    OPERATE_RET ret = OPRT_OK;

    if (sg_mf_test_cfg.status_cb){
        ret = sg_mf_test_cfg.status_cb(MF_STATUS_GPIOTEST_START);
        if (OPRT_OK != ret) {
            return FALSE;
        }
    }

    return TRUE;
}

/**
 * @brief callback for user basic test
 *
 * @param[in] cmd Test command
 * @param[in] data Test data
 * @param[in] len Test data len
 * @param[out] ret_data Test return data
 * @param[out] ret_len Test return data len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
STATIC BOOL_T __ty_app_mf_user_basic_test_cb(USHORT_T cmd, UCHAR_T *data, UINT_T len)
{
    char buf[24]= {0};

    if (MF_GPIO_TEST == cmd) {
        BOOL_T test = 0;

        memset(buf, 0, sizeof(buf));
        CHAR_T *out = tal_malloc(1024);
        if (NULL == out) {
            sprintf(buf, "{\"ret\":false}");
        } else {
            memset(out, 0, 1024);
            __ty_app_mf_user_pre_gpio_test_cb();
            test = ty_app_gpio_test_all((CHAR_T *)data, out);
            if (test == TRUE) {
                sprintf(buf, "{\"ret\":true}");
            } else {
                if (strlen(out) != 0) {
                    CHAR_T *prefix = "{\"ret\":false,";
                    memmove(out + strlen(prefix), out, strlen(out));
                    memcpy(out, prefix, strlen(prefix));
                    out[strlen(out)] = '}';
                    TAL_PR_DEBUG("buf:%s", out);
                    mf_cmd_basic_send(cmd, (BYTE_T *)out, strlen(out) + 1);
                    tal_free(out);
                    out = NULL;

                    return OPRT_OK;
                } else {
                    sprintf(buf, "{\"ret\":false}");
                }
            }
            tal_free(out);
            out = NULL;
        }
        TAL_PR_DEBUG("buf:%s", buf);
        mf_cmd_basic_send(cmd, (BYTE_T *)buf, strlen(buf) + 1);
    }

    return OPRT_OK;    
}

//{"mac":"11100397*******"}
STATIC OPERATE_RET __pt_get_mac(USHORT_T cmd, UCHAR_T *data, UINT_T len, UCHAR_T **ret_data, USHORT_T *ret_len)
{
    NW_MAC_S dev_mac;
    memset(&dev_mac, 0, SIZEOF(NW_MAC_S));
    if (OPRT_OK != tal_wifi_get_mac(0, &dev_mac)) {
        TAL_PR_ERR("get mac err");
        return OPRT_COM_ERROR;
    }
    ty_cJSON *root = ty_cJSON_CreateObject();
    TUYA_CHECK_NULL_RETURN(root, OPRT_COM_ERROR);

    UCHAR_T i = 0, k = 0;
    CHAR_T uDat[19] = {0};
    CHAR_T ubuf[14] = {0};
    for (i = 0; i < 12; i++) {
        k = i / 2;
        if (0 == i % 2) {
            uDat[i] = (dev_mac.mac[k] >> 4);
        } else {
            uDat[i] = (dev_mac.mac[k] & 0x0f);
        }
        sprintf((char *)ubuf + i, "%x", uDat[i]);
    }

    ty_cJSON_AddStringToObject(root, "mac", ubuf);
    CHAR_T *out = ty_cJSON_PrintUnformatted(root);
    ty_cJSON_Delete(root);
    TUYA_CHECK_NULL_RETURN(out, OPRT_COM_ERROR);

    TAL_PR_NOTICE("get mac %s", out);
    *ret_data = (UCHAR_T *)out;
    *ret_len = strlen(out);
    return OPRT_OK;
}

//{"ret":true,"rssi":xxxx}/{"ret":false}
STATIC OPERATE_RET __pt_get_wifi_rssi(USHORT_T cmd, UCHAR_T *data, UINT_T len, UCHAR_T **ret_data, USHORT_T *ret_len)
{
    ty_cJSON *root = ty_cJSON_CreateObject();
    TUYA_CHECK_NULL_RETURN(root, OPRT_COM_ERROR);

    SCHAR_T rssi = -55;
    OPERATE_RET rt = gw_get_rssi(&rssi);
    if (OPRT_OK != rt) {
        TAL_PR_ERR("get rssi err %d", rt);
        return rt;
    }

    ty_cJSON_AddStringToObject(root, "ret", "true");
    ty_cJSON_AddNumberToObject(root, "rssi", rssi);

    CHAR_T *out = ty_cJSON_PrintUnformatted(root);
    ty_cJSON_Delete(root);
    TUYA_CHECK_NULL_RETURN(out, OPRT_COM_ERROR);

    TAL_PR_NOTICE("get wifi rssi %s", out);
    *ret_data = (UCHAR_T *)out;
    *ret_len = strlen(out);
    return OPRT_OK;
}

/**
 * @note 上位机下发内容：{"ssid":"none"}，默认ssid为none，可以指定ssid
 *       交互成功：{"ret":true,"rssi":-50 }，交互失败：{"ret":false}
 */
STATIC OPERATE_RET __pt_get_ble_rssi(USHORT_T cmd, UCHAR_T *data, UINT_T len, UCHAR_T **ret_data, USHORT_T *ret_len)
{
    OPERATE_RET rt = OPRT_OK;

    ty_cJSON *json = ty_cJSON_Parse((CHAR_T *)data);
    TUYA_CHECK_NULL_RETURN(json, OPRT_CJSON_PARSE_ERR);

    ty_cJSON *jsontmp = ty_cJSON_GetObjectItem(json, "ssid");
    TUYA_CHECK_NULL_GOTO(jsontmp, EXIT);

#if defined(ENABLE_BT_SERVICE) && (ENABLE_BT_SERVICE == 1)
    ty_bt_scan_info_t ble_scan;
    memset(&ble_scan, 0, sizeof(ble_scan));
    strncpy(ble_scan.name, jsontmp->valuestring, 16);
    ble_scan.scan_type = TY_BT_SCAN_BY_NAME;
    ble_scan.timeout_s = 10;
    TAL_PR_DEBUG("ble scan name: %s", ble_scan.name);
    rt = tuya_sdk_bt_assign_scan(&ble_scan);
    if (OPRT_OK != rt) {
        TAL_PR_ERR("bt scan err");
        goto EXIT;
    }
    CHAR_T *out = Malloc(64);
    TUYA_CHECK_NULL_GOTO(out, EXIT);
    *ret_len = sprintf((char *)out, "{\"ret\":true,\"rssi\":%d}", ble_scan.rssi);
    *ret_data = (UCHAR_T *)out;

    TAL_PR_NOTICE("get ble rssi: %s", out);
#endif

EXIT:

    ty_cJSON_Delete(json);
    return rt;
}

//{"ret":true, "firmName":"esp_12F_test", "firmVer":"1.0.0", "flashSize":x "fac_pin":"xxxxxx"}/{"ret":false}
STATIC OPERATE_RET __pt_get_firm_info(USHORT_T cmd, UCHAR_T *data, UINT_T len, UCHAR_T **ret_data, USHORT_T *ret_len)
{
    ty_cJSON *root = ty_cJSON_CreateObject();
    TUYA_CHECK_NULL_RETURN(root, OPRT_COM_ERROR);

    CHAR_T fac[21] = {0};
    mf_test_facpin_get(fac);
    ty_cJSON_AddStringToObject(root, "ret", "true");
    ty_cJSON_AddStringToObject(root, "firmName", APP_BIN_NAME);
    ty_cJSON_AddStringToObject(root, "firmVer", USER_SW_VER);
    ty_cJSON_AddNumberToObject(root, "flashSize", 16);
    ty_cJSON_AddStringToObject(root, "fac_pin", fac);

    CHAR_T *out = ty_cJSON_PrintUnformatted(root);
    ty_cJSON_Delete(root);
    TUYA_CHECK_NULL_RETURN(out, OPRT_COM_ERROR);

    TAL_PR_NOTICE("get firmware info %s", out);
    *ret_data = (UCHAR_T *)out;
    *ret_len = strlen(out);
    return OPRT_OK;
}


STATIC THREAD_HANDLE __ds_thread = NULL;

STATIC VOID_T __pt_deepsleep(VOID_T *args)
{
    
    tal_system_sleep(500);

    #define TUYA_CPU_SUPER_DEEP_SLEEP 2
    tal_cpu_sleep_mode_set(TRUE, TUYA_CPU_SUPER_DEEP_SLEEP);

    while(1) tal_system_sleep(500);
}
/**
* @brief 通用bool量测试，0xFF03
* @note 上位机发送:{"testItem":"serialPort1"};设备返回:{"ret":true}/{"ret":false,"errCode":500000}
*/
STATIC OPERATE_RET __pt_bool_test(USHORT_T cmd, UCHAR_T *data, UINT_T len, UCHAR_T **ret_data, USHORT_T *ret_len)
{
    OPERATE_RET rt = OPRT_OK;

    TAL_PR_NOTICE("bool test %s", data);
    ty_cJSON *root_json = ty_cJSON_Parse((CHAR_T *)data);
    TUYA_CHECK_NULL_RETURN(root_json, OPRT_CJSON_PARSE_ERR);
    ty_cJSON *json_item = ty_cJSON_GetObjectItem(root_json, "testItem");
    TUYA_CHECK_NULL_GOTO(json_item, EXIT);
    if (0 == strncmp(json_item->valuestring, "LowPowerMode", strlen("LowPowerMode"))) {
        CHAR_T *out = tal_malloc(PT_RESP_BUF_SIZE);
        TUYA_CHECK_NULL_GOTO(out, EXIT);

        memset(out, 0, PT_RESP_BUF_SIZE);
        strcpy(out, "{\"ret\":true}");

        *ret_data = (UCHAR_T *)out;
        *ret_len = strlen(out);
        THREAD_CFG_T thrd_param = {0};

        thrd_param.thrdname = "__ds_thread";
        thrd_param.priority = THREAD_PRIO_1;
        thrd_param.stackDepth = 4096;
        tal_thread_create_and_start(&__ds_thread, NULL, NULL, __pt_deepsleep, NULL, &thrd_param);
    } else {
        TAL_PR_NOTICE("app user product test");
        rt = OPRT_NOT_FOUND;
    }

EXIT:
    ty_cJSON_Delete(root_json);
    return rt;
}

//https://wiki.tuya-inc.com:7799/page/64260322
STATIC CONST PROD_TEST_OP_S pt_cmd_proc_arr[] = {
    { CMD_GET_DEV_MAC,             __pt_get_mac             }, //module
    { CMD_GET_WIFI_RSSI,           __pt_get_wifi_rssi       }, //module
    { CMD_GET_BLE_RSSI,            __pt_get_ble_rssi        }, //sdk
    { CMD_GET_FIRM_INFO,           __pt_get_firm_info       }, //module
    { CMD_BOOL_TEST,               __pt_bool_test           },
};

/**
 * @brief 成品产测指令回调接口
 *
 * @return VOID_T
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 *
 */
OPERATE_RET ty_sys_user_product_test_cb(USHORT_T cmd, UCHAR_T *data, UINT_T len,\
                                        OUT UCHAR_T **ret_data, OUT USHORT_T *ret_len)
{
    OPERATE_RET rt = OPRT_NOT_FOUND;

    UCHAR_T idx = 0;
    for (idx = 0; idx < CNTSOF(pt_cmd_proc_arr); idx++) {
        if (cmd == (USHORT_T)pt_cmd_proc_arr[idx].cmd) {
            if (pt_cmd_proc_arr[idx].func) {
                rt = pt_cmd_proc_arr[idx].func(cmd, data, len, ret_data, ret_len);
            }
        }
    }

    //如果在上面的回调中没有找到对应的cmd，则调用用户自定义的回调
    if (OPRT_NOT_FOUND == rt && sg_mf_test_cfg.user_prod_cb) {
        rt = sg_mf_test_cfg.user_prod_cb(cmd, data, len, ret_data, ret_len);
    }

    return rt;
}

/**
 * @brief mftest状态回调注册
 *
 * @param[in] : MFTEST_STATUS_CALLBACK:状态回调
 *
 * @return none
 */
OPERATE_RET ty_sys_mf_status_cb_reg(TY_MF_STATUS_CALLBACK callback)
{
    if(NULL == callback) {
        return OPRT_INVALID_PARM;
    }

    sg_mf_test_cfg.status_cb = callback;

    return OPRT_OK;
}

/**
 * @brief     mf test 成品产测回调注册
 *
 * @param[in] : callback     成品产测回调
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_sys_mf_product_test_reg(MF_USER_PRODUCT_TEST_CB callback)
{
    if(NULL == callback) {
        return OPRT_INVALID_PARM;
    }

    sg_mf_test_cfg.user_prod_cb = callback;

    return OPRT_OK;
}

/**
 * @brief 获取 mf test 初始化接口
 *
 * @param[in] : intf  初始化接口指针
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_sys_mf_test_get_intf(MF_IMPORT_INTF_S *intf)
{
    if(NULL == intf) {
        return OPRT_INVALID_PARM;
    }

    intf->uart_init = __ty_app_mf_uart_init;
    intf->uart_free = __ty_app_mf_uart_free;
    intf->uart_recv = __ty_app_mf_recv;
    intf->uart_send = __ty_app_mf_send;

    intf->mf_user_basic_test     = __ty_app_mf_user_basic_test_cb;
    intf->mf_user_product_test   = ty_sys_user_product_test_cb;
    intf->user_callback          = __ty_app_mf_user_callback;
    intf->user_enter_mf_callback = __ty_app_mf_user_enter_cb;
  
    return OPRT_OK;
}
