/***********************************************************
*  File: tfm_json_cfg_ug.c
*  Author:lck
*  Date: 20231120
***********************************************************/
#include "tuya_iot_com_api.h"
#include "gw_intf.h"
#include "uni_log.h"

#include "tfm_json_cfg_ug.h"
#include "tuya_iot_wifi_api.h"
#include "tal_hash.h"
#include "stdio.h"

#define JSON_LEN_MAX                (4096*3)    // json最大长度
#define JSON_UPDATE_CH              100         // json更新的通道号

typedef struct {
    UINT_T len;
    BYTE_T *content;
    JSON_DATA_CB deal_cb;
}JSON_CFG_ST;

STATIC JSON_CFG_ST g_js_cfg = {
    .len = 0,
    .content = NULL,
    .deal_cb = NULL,
};

/***********************************************************
*  Function:tfm_json_update_init
*  Input: js_ver: json base version
*         js_md5: json md5 string， 32 bytes
*  Output: JSON_DATA_CB
*          js_data: json data
*          js_len:  json length
*          result:  json upload reult, OPRT_OK success; Other fail;
*  Return: none
*  Note: 
***********************************************************/
OPERATE_RET tfm_json_update_init(IN CONST CHAR_T *js_ver, IN CONST CHAR_T *js_md5, JSON_DATA_CB js_data_cb)
{
    OPERATE_RET op_ret = OPRT_OK;
    GW_ATTACH_ATTR_T attach_dev = {0};

    g_js_cfg.deal_cb = js_data_cb;

    attach_dev.tp = JSON_UPDATE_CH;
    strncpy(attach_dev.ver, js_ver, sizeof(attach_dev.ver));
    strncpy(attach_dev.md5, js_md5, sizeof(attach_dev.md5));
    PR_DEBUG("json ver:%s", attach_dev.ver);
    op_ret = tuya_iot_dev_update_attachs(1, &attach_dev);
    if(OPRT_OK != op_ret) {
        PR_ERR("tuya_iot_dev_update_attachs, op_ret:%d",op_ret);
        return op_ret;
    }

    return OPRT_OK;
}

/***********************************************************
*  Function:tfm_json_upload_inform
*  Input: none
*  Output: none
*  Return: none
*  Note: 
***********************************************************/
OPERATE_RET tfm_json_upload_inform(IN CONST FW_UG_S *fw)
{
    // OPERATE_RET op_ret = OPRT_OK;

    if(fw->file_size > JSON_LEN_MAX) {
        PR_ERR("json too long, file size is %d", fw->file_size);
        return OPRT_INVALID_PARM;
    }

    g_js_cfg.len = fw->file_size;
    g_js_cfg.content = (BYTE_T *)tal_malloc(g_js_cfg.len + 1);
    if(NULL == g_js_cfg.content) {
        return OPRT_MALLOC_FAILED;
    }
    memset(g_js_cfg.content, 0, g_js_cfg.len + 1);

    return OPRT_OK;
}

/***********************************************************
*  Function:tfm_json_upload_notify_cb
*  Input: none
*  Output: none
*  Return: none
*  Note: 
***********************************************************/
VOID tfm_json_upload_notify_cb(IN CONST FW_UG_S *fw, IN CONST INT_T download_result, IN PVOID_T pri_data)
{
    if(NULL != g_js_cfg.deal_cb) {
        g_js_cfg.deal_cb(g_js_cfg.content, g_js_cfg.len, download_result);
    }

    if(NULL != g_js_cfg.content) {
        tal_free(g_js_cfg.content);
        g_js_cfg.content = NULL;
    }

    return;
}

/***********************************************************
*  Function:tfm_json_upload_process_cb
*  Input: none
*  Output: none
*  Return: none
*  Note: 
***********************************************************/
OPERATE_RET tfm_json_upload_process_cb(IN CONST FW_UG_S *fw, IN CONST UINT_T total_len,IN CONST UINT_T offset,
                              IN CONST BYTE_T *data,IN CONST UINT_T len,OUT UINT_T *remain_len, IN PVOID_T pri_data)
{
    if(offset + len <= g_js_cfg.len) {
        memcpy(g_js_cfg.content + offset, data, len);
    }

    *remain_len = 0;

    return OPRT_OK;
}

/***********************************************************
*  Function:tfm_json_md5_cal
*  Input: src: data content
*         src_len: data len
*  Output: md5_buf: md5 string, at least 32 bytes
*  Return: none
*  Note: 
***********************************************************/
VOID tfm_json_md5_cal(IN CHAR_T *src, IN UINT_T src_len, OUT CHAR_T *md5_buf)
{
    TKL_HASH_HANDLE md5_ctx;
    UCHAR_T hex_buf[16] = {0};
    UINT_T i = 0;

    tal_md5_create_init(&md5_ctx);
    tal_md5_starts_ret(md5_ctx);
    tal_md5_update_ret(md5_ctx, (UCHAR_T *)src, src_len);
    tal_md5_finish_ret(md5_ctx, hex_buf);

    for(i = 0; i < 16; i++) {
        sprintf(&md5_buf[2*i], "%02x", hex_buf[i]);
    }
    
    return;
}