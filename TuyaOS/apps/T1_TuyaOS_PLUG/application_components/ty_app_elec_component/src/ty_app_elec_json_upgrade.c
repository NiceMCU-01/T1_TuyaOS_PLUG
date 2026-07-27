/**
 * @file ty_app_elec_json_upgrade.c
 * @author www.tuya.com
 * @brief ty_app_elec_json_upgrade module is used to 
 * @version 0.1
 * @date 2025-06-25
 *
 * @copyright Copyright (c) tuya.inc 2025
 *
 */
#include "tuya_app_config.h"
#if (defined(ELEC_JSON_UPGRADE_EN) && (ELEC_JSON_UPGRADE_EN==1))
#include "tuya_ws_db.h"
#include "tfm_json_cfg_ug.h"
#include "ty_app_elec_json_upgrade.h"
#include "ty_app_starts_up_intf.h"
#include "tuya_ws_db.h"
#include "tfm_timing_storage.h"

/***********************************************************
*************************micro define***********************
***********************************************************/
// json的时间戳，kv区和用户区：{"kv":1703750712,"user":1703750712}
#define JSON_TIMESTAMP "json_timestamp"     

// dp点上报间隔：{"dp21":300}
#define JSON_DP_INTERVAL "json_dp_interval"  

/***********************************************************
***********************typedef define***********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC CHAR_T sg_json_version[12] = {0};

/***********************************************************
***********************function define**********************
***********************************************************/
/**
* @brief JSON版本号
*
* @param[in] json_version JSON版本号
* @return none
*/
VOID app_json_version(CONST CHAR_T *json_version)
{
    strncpy(sg_json_version, json_version, SIZEOF(sg_json_version)-1);
}

/**
 * @brief 获取json热更新和自动更新的json的时间戳
 *
 * @param[out] kv_t   存储kv区json时间戳的指针
 * @param[out] user_t 存储用户区json时间戳的指针
 * @return 操作结果，OPRT_OK表示成功
 *
 * @details
 * 该函数从存储中读取JSON_TIMESTAMP键值，解析出kv和user两个时间戳字段。
 * 如果解析失败，则两个时间戳都置为0。
 */
OPERATE_RET json_timestamp_get(OUT UINT_T *kv_t, OUT UINT_T *user_t)
{
    OPERATE_RET rt = OPRT_OK;
    UCHAR_T timestamp_buf[40] = {0};
    cJSON *root = NULL, *js_tmp = NULL;

    // 1. json热更新和自动更新的json，都记录更新的时间戳，哪个新使用哪个
    // {"kv":1703750712,"user":1703750712}
    tfm_storage_read_data(JSON_TIMESTAMP, timestamp_buf, SIZEOF(timestamp_buf));
    TAL_PR_DEBUG("timestamp_buf: %s", timestamp_buf);
    root = ty_cJSON_Parse((CHAR_T *)timestamp_buf);
    if (root)
    {
        js_tmp = ty_cJSON_GetObjectItem(root, "kv");
        if (js_tmp)
        {
            *kv_t = (UINT_T)js_tmp->valueint;
            js_tmp = NULL;
        }
        js_tmp = ty_cJSON_GetObjectItem(root, "user");
        if (js_tmp)
        {
            *user_t = (UINT_T)js_tmp->valueint;
            js_tmp = NULL;
        }
        ty_cJSON_Delete(root);
        root = NULL;
    }
    else
    {
        *kv_t = 0;
        *user_t = 0;
    }

    return rt;
}

/**
 * @brief 设置json热更新和自动更新的json的时间戳
 *
 * @param[in] kv_t   kv区json时间戳
 * @param[in] user_t 用户区json时间戳
 * @return 操作结果，OPRT_OK表示成功
 *
 * @details
 * 该函数将kv和user两个时间戳字段打包成json字符串，并写入存储（JSON_TIMESTAMP键）。
 */
OPERATE_RET json_timestamp_set(IN UINT_T kv_t, IN UINT_T user_t)
{
    OPERATE_RET rt = OPRT_OK;
    cJSON *root = NULL;
    CHAR_T *out = NULL;

    root = ty_cJSON_CreateObject();
    TUYA_CHECK_NULL_RETURN(root,OPRT_CR_CJSON_ERR);
    ty_cJSON_AddNumberToObject(root, "kv", kv_t);
    ty_cJSON_AddNumberToObject(root, "user", user_t);
    out = ty_cJSON_PrintUnformatted(root);
    ty_cJSON_Delete(root);
    if (out){
        TAL_PR_DEBUG("set timestamp: %s", out);
        tfm_storage_write_data(JSON_TIMESTAMP, (UCHAR_T *)out, strlen(out));
        Free(out);
        out = NULL;
    }

    return rt;
}

/**
* @brief JSON升级回调
*
* @param[in] js_data json数据
* @param[in] js_len json长度
* @param[in] result 升级结果
* @return none
*/
VOID app_json_upgrade_cb(IN CONST BYTE_T *js_data, IN CONST UINT_T js_len, IN CONST OPERATE_RET result)
{
    OPERATE_RET op_ret = OPRT_OK;
    // UINT_T kv_timestamp = 0, user_timestamp = 0;

    if (OPRT_OK != result){
        TAL_PR_ERR("json_update fail !!!");
        return;
    }
    TAL_PR_NOTICE(">>>>> update js_data = %s", js_data);
    
    op_ret = wd_user_param_write(js_data, js_len);
    if (OPRT_OK != op_ret){
        TAL_PR_ERR("wd_user_param_write err: %d", op_ret);
        return;
    }
    ty_app_elec_oem_nvs_write_config(js_data, js_len);
    
    // json_timestamp_get(&kv_timestamp, &user_timestamp);
    // kv_timestamp = tal_time_get_cur_posix();
    // json_timestamp_set(kv_timestamp, user_timestamp);

    tal_system_reset();
}

/**
 * @brief 设备OTA回调函数
 *
 * @param[in] fw
 * @param[in] total_len
 * @param[in] offset
 * @param[in] data
 * @param[in] len
 * @param[in] remain_len
 * @param[in] pri_data
 * @return
 */
STATIC OPERATE_RET __get_file_data_cb(IN CONST FW_UG_S *fw, IN CONST UINT_T total_len, IN CONST UINT_T offset,
                                      IN CONST BYTE_T *data, IN CONST UINT_T len, OUT UINT_T *remain_len,
                                      IN PVOID_T pri_data)
{
    OPERATE_RET rt = OPRT_OK;

    TAL_PR_DEBUG("Rev File Data, Total_len:%d, Offset:%d, Len:%d", total_len, offset, len);

    TUYA_CALL_ERR_RETURN(tfm_json_upload_process_cb(fw, total_len, offset, data, len, remain_len, pri_data));

    return rt;
}

/**
 * @brief 设备更新回调
 *
 * @param[in] fw
 * @param[in] download_result
 * @param[in] pri_data
 * @return none
 */
STATIC OPERATE_RET __upgrade_notify_cb(IN CONST FW_UG_S *fw, IN CONST INT_T download_result, IN PVOID_T pri_data)
{
    OPERATE_RET rt = OPRT_OK;

    TAL_PR_DEBUG("download Finish ,download_result:%d", download_result);

    tfm_json_upload_notify_cb(fw, download_result, pri_data);

    return rt;
}

/**
 * @brief 设备更新通知回调
 *
 * @param[in] fw
 * @return 更新结果
 */
int app_json_upgrade_pre_cb(IN CONST FW_UG_S *fw)
{
    OPERATE_RET rt = OPRT_OK;

    TAL_PR_DEBUG("Rev GW Upgrade Info");
    TAL_PR_DEBUG("fw->tp:%d", fw->tp);
    TAL_PR_DEBUG("fw->fw_url:%s", fw->fw_url);
    TAL_PR_DEBUG("fw->fw_hmac:%s", fw->fw_hmac);
    TAL_PR_DEBUG("fw->sw_ver:%s", fw->sw_ver);
    TAL_PR_DEBUG("fw->file_size:%d", fw->file_size);
    TAL_PR_DEBUG("fw->fw_md5:%s", fw->fw_md5);

    TUYA_CALL_ERR_RETURN(tfm_json_upload_inform(fw));
    TUYA_CALL_ERR_RETURN(tuya_iot_upgrade_gw(fw, __get_file_data_cb, __upgrade_notify_cb, NULL));

    return rt;
}
/***********************************************************
*  Function:app_json_upgrade_init
*  Input: 
*  Output: 
*  Return: none
***********************************************************/
OPERATE_RET app_json_upgrade_init(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    UCHAR_T *pConfig = NULL;
    UINT_T len = 0;
    CHAR_T md5_buf[32 + 1] = {0};

    rt = wd_user_param_read(&pConfig, &len);
    if (OPRT_OK != rt){
        TAL_PR_ERR("wd_user_param_read op_ret: %d", rt);
        memset(md5_buf, 0x30, SIZEOF(md5_buf) - 1);
        tfm_json_update_init("0.0.1", md5_buf, app_json_upgrade_cb);
        return rt;
    }
    TAL_PR_DEBUG("jv: %s", sg_json_version);
    UCHAR_T *tmp_buf = (UCHAR_T*)tal_malloc(1024);
    TUYA_CHECK_NULL_RETURN(tmp_buf,OPRT_MALLOC_FAILED);

    for (UCHAR_T i = 0; i < len/950 + 1; i++) {
        memset(tmp_buf, 0x00, 1024);
        memcpy(tmp_buf, &pConfig[950*i], 950);
        TAL_PR_DEBUG("json: %s", tmp_buf);
    }

    if(NULL != tmp_buf){
        tal_free(tmp_buf);
        tmp_buf = NULL;
    }
    
    tfm_json_md5_cal((CHAR_T *)pConfig, len, md5_buf);

    if(NULL != pConfig){
        tal_free(pConfig);
        pConfig = NULL;
    }

    TAL_PR_DEBUG("json md5:%s", md5_buf);
    TUYA_CALL_ERR_RETURN(tfm_json_update_init(sg_json_version, md5_buf, app_json_upgrade_cb));

    return rt;
}
#endif