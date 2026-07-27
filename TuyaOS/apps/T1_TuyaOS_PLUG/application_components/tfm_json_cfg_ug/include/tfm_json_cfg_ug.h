#ifndef __TFM_JSON_CFG_UG_H
#define __TFM_JSON_CFG_UG_H

#include "tuya_cloud_com_defs.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef VOID (*JSON_DATA_CB)(IN CONST BYTE_T *js_data, IN CONST UINT_T js_len,IN CONST OPERATE_RET result);

/***********************************************************
*  Function:tfm_json_upload_inform
*  Input: 
*  Output: none
*  Return: none
*  Note: 
***********************************************************/
OPERATE_RET tfm_json_upload_inform(IN CONST FW_UG_S *fw);

/***********************************************************
*  Function:tfm_json_upload_notify_cb
*  Input: none
*  Output: none
*  Return: none
*  Note: 
***********************************************************/
VOID tfm_json_upload_notify_cb(IN CONST FW_UG_S *fw, IN CONST INT_T download_result, IN PVOID_T pri_data);

/***********************************************************
*  Function:tfm_json_upload_process_cb
*  Input: none
*  Output: none
*  Return: none
*  Note: 
***********************************************************/
OPERATE_RET tfm_json_upload_process_cb(IN CONST FW_UG_S *fw, IN CONST UINT_T total_len,IN CONST UINT_T offset,
                              IN CONST BYTE_T *data,IN CONST UINT_T len,OUT UINT_T *remain_len, IN PVOID_T pri_data);

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
OPERATE_RET tfm_json_update_init(IN CONST CHAR_T *js_ver, IN CONST CHAR_T *js_md5, JSON_DATA_CB js_data_cb);

/***********************************************************
*  Function:tfm_json_md5_cal
*  Input: src: data content
*         src_len: data len
*  Output: md5_buf: md5 string, at least 32 bytes
*  Return: none
*  Note: 
***********************************************************/
VOID tfm_json_md5_cal(IN CHAR_T *src, IN UINT_T src_len, OUT CHAR_T *md5_buf);


#ifdef __cplusplus
}
#endif 

#endif