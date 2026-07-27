/**
 * @file event_loop.c
 * @author www.tuya.com
 * @brief event_loop module is used to
 * @version 0.1
 * @date 2022-06-16
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#include "ty_sys.h"

#include "ty_app_msg.h"
#include "ty_event_loop.h"

/***********************************************************
************************macro define************************
***********************************************************/

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    LIST_HEAD list_head;
    APP_MSG_ID msg_id;
    EVENT_GROUP_ID group_id;
    EVENTLOOP_CALLBACK event_cb;
} APP_EVENT_LIST_T;

typedef unsigned char APP_EVENT_ID_TYPE_E;
#define APP_EVENT_MSG_ID   0x00
#define APP_EVENT_GROUP_ID 0x01

/***********************************************************
********************function declaration********************
***********************************************************/

/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC LIST_HEAD g_ty_app_event_list = {
    .next = NULL,
    .prev = NULL,
};

/***********************************************************
***********************function define**********************
***********************************************************/
STATIC APP_EVENT_LIST_T *__find_event_group_handle(APP_EVENT_ID_TYPE_E type, UINT_T id)
{
    LIST_HEAD *pos = NULL;
    APP_EVENT_LIST_T *event_handle = NULL;
    APP_EVENT_LIST_T *list = NULL;
    UINT_T target_id = 0;

    tuya_list_for_each(pos, &g_ty_app_event_list)
    {
        list = tuya_list_entry(pos, APP_EVENT_LIST_T, list_head);
        target_id = (type == APP_EVENT_MSG_ID) ? list->msg_id : list->group_id;
        if (target_id == id) {
            event_handle = list;
            break;
        }
    }

    return event_handle;
}

STATIC void event_msg_callback(APP_MSG_S *msg)
{
    APP_EVENT_LIST_T *handle = __find_event_group_handle(APP_EVENT_MSG_ID, msg->msg_id);
    if (NULL == handle) {
        TAL_PR_ERR("find event handle failed");
        return;
    }

    APP_EVENT_MSG_S *event = (APP_EVENT_MSG_S *)msg->msg_data;

    if (handle->event_cb) {
        handle->event_cb(event);
    }

    tal_free(msg->msg_data);

    return;
}

STATIC OPERATE_RET __eventloop_data_package(APP_EVENT_MSG_S *event_msg, UINT32_T event_id, void *event_data,
                                            UINT32_T event_data_len)
{
    memset(event_msg, 0, sizeof(APP_EVENT_MSG_S) + event_data_len);
    event_msg->event_id = event_id;
    event_msg->size = event_data_len;
    if (NULL != event_data) {
        memcpy((char *)((char *)event_msg + sizeof(APP_EVENT_MSG_S)), event_data, event_data_len);
    }

    return OPRT_OK;
}

/**
 * @brief              触发事件
 *
 * @param[in] :    grop_id           事件组id
 * @param[in] :    event_id          事件id
 * @param[in] :    event_data        事件消息
 * @param[in] :    event_data_len    消息长度
 *
 * @return none
 */
OPERATE_RET ty_app_event_post(EVENT_GROUP_ID grop_id, UINT32_T event_id, void *event_data, UINT32_T event_data_len)
{
    OPERATE_RET op_ret = OPRT_OK;

    if ((NULL == event_data && event_data_len != 0) || (NULL != event_data && event_data_len == 0)) {
        return OPRT_INVALID_PARM;
    }

    APP_EVENT_LIST_T *handle = __find_event_group_handle(APP_EVENT_GROUP_ID, grop_id);
    if (NULL == handle) {
        TAL_PR_ERR("find event handle failed");
        return OPRT_INVALID_PARM;
    }

    APP_EVENT_MSG_S *event_msg = NULL;
    event_msg = (APP_EVENT_MSG_S *)tal_malloc(sizeof(APP_EVENT_MSG_S) + event_data_len);
    if (NULL == event_msg) {
        return OPRT_MALLOC_FAILED;
    }

    op_ret = __eventloop_data_package(event_msg, event_id, event_data, event_data_len);
    if (OPRT_OK != op_ret) {
        tal_free(event_msg);
        return op_ret;
    }

    op_ret =
        ty_app_msg_queue_post(handle->msg_id, event_msg, sizeof(APP_EVENT_MSG_S) + event_data_len, MSG_TYPE_NORMAL);
    if (OPRT_OK != op_ret) {
        tal_free(event_msg);
        return op_ret;
    }

    return op_ret;
}

/**
 * @brief              紧急触发事件
 *
 * @param[in] :    grop_id           事件组id
 * @param[in] :    event_id          事件id
 * @param[in] :    event_data        事件消息
 * @param[in] :    event_data_len    消息长度:
 *
 * @return none
 */
int ty_app_event_post_instancy(EVENT_GROUP_ID grop_id, UINT32_T event_id, void *event_data, UINT32_T event_data_len)
{
    OPERATE_RET op_ret = OPRT_OK;

    if ((NULL == event_data && event_data_len != 0) || (NULL != event_data && event_data_len == 0)) {
        return OPRT_INVALID_PARM;
    }

    APP_EVENT_LIST_T *handle = __find_event_group_handle(APP_EVENT_GROUP_ID, grop_id);
    if (NULL == handle) {
        TAL_PR_ERR("find event handle failed");
        return OPRT_INVALID_PARM;
    }

    APP_EVENT_MSG_S *event_msg = NULL;
    event_msg = (APP_EVENT_MSG_S *)tal_malloc(sizeof(APP_EVENT_MSG_S) + event_data_len);
    if (NULL == event_msg) {
        return OPRT_MALLOC_FAILED;
    }

    op_ret = __eventloop_data_package(event_msg, event_id, event_data, event_data_len);
    if (OPRT_OK != op_ret) {
        tal_free(event_msg);
        return op_ret;
    }

    op_ret =
        ty_app_msg_queue_post(handle->msg_id, event_msg, sizeof(APP_EVENT_MSG_S) + event_data_len, MSG_TYPE_INSTANCY);
    if (OPRT_OK != op_ret) {
        tal_free(event_msg);
        return op_ret;
    }

    return op_ret;
}

/**
 * @brief              同步触发事件
 *
 * @param[in] :    grop_id           事件组id
 * @param[in] :    event_id          事件id
 * @param[in] :    event_data        事件消息
 * @param[in] :    event_data_len    消息长度
 *
 * @return none
 */
OPERATE_RET ty_app_event_post_synchronous(EVENT_GROUP_ID grop_id, UINT32_T event_id, void *event_data, UINT32_T event_data_len)
{
    OPERATE_RET rt = OPRT_OK;
    
    APP_EVENT_LIST_T *handle = __find_event_group_handle(APP_EVENT_GROUP_ID, grop_id);
    if (NULL == handle || NULL == handle->event_cb) {
        TAL_PR_ERR("find event handle failed or event cb not exist");
        return OPRT_COM_ERROR;
    }

    APP_EVENT_MSG_S *event_msg = NULL;
    event_msg = (APP_EVENT_MSG_S *)tal_malloc(sizeof(APP_EVENT_MSG_S) + event_data_len);
    if (NULL == event_msg) {
        return OPRT_MALLOC_FAILED;
    }
    
    rt = __eventloop_data_package(event_msg, event_id, event_data, event_data_len);
    if (OPRT_OK != rt) {
        tal_free(event_msg);
        return rt;
    }
    
    handle->event_cb(event_msg);

    tal_free(event_msg);

    return OPRT_OK;
}

/**
 * @brief         应用事件注册
 *
 * @param[in]     group_id       事件组id
 * @param[in]     event_cb       事件组处理回调
 *
 * @return none
 */
OPERATE_RET ty_app_event_register(EVENT_GROUP_ID group_id, EVENTLOOP_CALLBACK event_cb)
{
    OPERATE_RET op_ret = OPRT_OK;
    APP_EVENT_LIST_T *event_handle = NULL;

    if (NULL == event_cb) {
        return OPRT_INVALID_PARM;
    }

    event_handle = (APP_EVENT_LIST_T *)tal_malloc(sizeof(APP_EVENT_LIST_T));
    if (NULL == event_handle) {
        return OPRT_MALLOC_FAILED;
    }

    memset((unsigned char *)event_handle, 0x00, sizeof(APP_EVENT_LIST_T));

    if (g_ty_app_event_list.next == NULL)
        INIT_LIST_HEAD(&(g_ty_app_event_list));

    op_ret = ty_app_msg_queue_reg(event_msg_callback, &event_handle->msg_id);
    if (op_ret != OPRT_OK) {
        tal_free(event_handle);
        TAL_PR_ERR("ty_app_msg_queue_reg failed op_ret:%d", op_ret);
        return op_ret;
    }

    event_handle->group_id = group_id;
    event_handle->event_cb = event_cb;

    tuya_list_add_tail(&event_handle->list_head, &g_ty_app_event_list);

    return op_ret;
}
