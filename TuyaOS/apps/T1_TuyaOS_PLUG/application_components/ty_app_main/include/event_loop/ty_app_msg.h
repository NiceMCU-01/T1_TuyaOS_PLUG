/**
 * @file ty_app_msg.h
 * @author www.tuya.com
 * @brief ty_app_msg module is used to 
 * @version 0.1
 * @date 2022-06-16
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#ifndef __TY_APP_MSG_H__
#define __TY_APP_MSG_H__

#include "tuya_list.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    UINT16_T    msg_id;            // message id
    UINT32_T    msg_data_len; // message data len
    void        *msg_data;     // message data
} APP_MSG_S;

typedef struct {
    LIST_HEAD list_head;  // list head
    APP_MSG_S msg;         // message info
} SYS_MSG_LIST_S;

typedef enum {
    MSG_TYPE_NORMAL = 0,
    MSG_TYPE_INSTANCY,
} APP_MSG_TYPE_E;

typedef void(*SYS_MSG_CALLBACK)(APP_MSG_S *msg);

typedef UINT16_T APP_MSG_ID;

/***********************************************************
********************function declaration********************
***********************************************************/
/**
 * @brief ty_app_msg_queue_post
 *
 * @param[in] : msg_id
 * @param[in] : msg_data
 * @param[in] : msg_data_len
 * @param[in] : msg_type
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_msg_queue_post(UINT16_T msg_id, VOID_T *msg_data, UINT32_T msg_data_len, APP_MSG_TYPE_E msg_type);

/**
 * @brief ty_app_msg_queue_post_malloc
 *
 * @param[in] : msg_id
 * @param[in] : msg_data
 * @param[in] : msg_data_len
 * @param[in] : msg_type
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_msg_queue_post_malloc(UINT16_T msg_id, VOID_T *msg_data, UINT32_T msg_data_len, APP_MSG_TYPE_E msg_type);

/**
 * @brief ty_app_msg_queue_init
 *
 * @param[in] : none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_msg_queue_init(VOID_T);

/**
 * @brief ty_app_msg_queue_reg
 *
 * @param[in] : msg_cb
 * @param[in] : msg_id
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_msg_queue_reg(SYS_MSG_CALLBACK msg_cb, UINT16_T *msg_id);


#ifdef __cplusplus
}
#endif

#endif /* __TY_APP_MSG_H__ */
