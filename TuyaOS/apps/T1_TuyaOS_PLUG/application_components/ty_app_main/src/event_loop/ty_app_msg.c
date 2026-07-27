/**
 * @file ty_app_msg.c
 * @author www.tuya.com
 * @brief ty_app_msg module is used to
 * @version 0.1
 * @date 2022-06-16
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#include "ty_sys.h"
#include "ty_app_msg.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define MSG_MAX_NUM     64
#define MSGCB_POOL_STEP 4
#define MSGID_UNVALUED  0xffff // invalid message id

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    LIST_HEAD list_head;
    APP_MSG_ID msgNodeNum;

    MUTEX_HANDLE mutex_handle;
    THREAD_HANDLE msg_handle;
    SEM_HANDLE sem_handle;

    SYS_MSG_CALLBACK *msg_cb;
    UINT16_T msgcb_num;
    UINT16_T msgcb_pool_size;
} TY_APP_MSG_S;

/***********************************************************
********************function declaration********************
***********************************************************/

/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC TY_APP_MSG_S ty_app_msg;

/***********************************************************
***********************function define**********************
***********************************************************/

STATIC OPERATE_RET __add_msg_to_queue(UINT16_T msg_id, VOID_T *msg_data, UINT32_T msg_data_len, APP_MSG_TYPE_E msg_type)
{
    OPERATE_RET op_ret = OPRT_OK;

    // add msg to queue
    SYS_MSG_LIST_S *pMsgListNode = NULL;
    pMsgListNode = tal_malloc(sizeof(SYS_MSG_LIST_S));
    if (NULL == pMsgListNode)
        return OPRT_MALLOC_FAILED;

    op_ret = tal_mutex_lock(ty_app_msg.mutex_handle);
    if (OPRT_OK != op_ret) {
        tal_free(pMsgListNode);
        return op_ret;
    }

    pMsgListNode->msg.msg_id = msg_id;
    pMsgListNode->msg.msg_data = msg_data;
    pMsgListNode->msg.msg_data_len = msg_data_len;

    if (MSG_TYPE_INSTANCY == msg_type) {
        tuya_list_add(&(pMsgListNode->list_head), &(ty_app_msg.list_head));
    } else {
        tuya_list_add_tail(&(pMsgListNode->list_head), &(ty_app_msg.list_head));
    }

    ty_app_msg.msgNodeNum++;

    op_ret = tal_mutex_unlock(ty_app_msg.mutex_handle);
    if (OPRT_OK != op_ret) {
        tal_free(pMsgListNode);
        return op_ret;
    }

    return OPRT_OK;
}

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
OPERATE_RET ty_app_msg_queue_post(UINT16_T msg_id, VOID_T *msg_data, UINT32_T msg_data_len, APP_MSG_TYPE_E msg_type)
{
    OPERATE_RET op_ret = OPRT_OK;

    if (ty_app_msg.msgNodeNum >= MSG_MAX_NUM) {
        return OPRT_COM_ERROR;
    }

    op_ret = __add_msg_to_queue(msg_id, msg_data, msg_data_len, msg_type);
    if (OPRT_OK != op_ret) {
        return op_ret;
    }
    
    op_ret = tal_semaphore_post(ty_app_msg.sem_handle);
    if (OPRT_OK != op_ret) {
        return op_ret;
    }

    return OPRT_OK;
}

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
OPERATE_RET ty_app_msg_queue_post_malloc(UINT16_T msg_id, VOID_T *msg_data, UINT32_T msg_data_len,
                                         APP_MSG_TYPE_E msg_type)
{
    OPERATE_RET op_ret = OPRT_OK;
    VOID_T *data = NULL;

    if (ty_app_msg.msgNodeNum >= MSG_MAX_NUM) {
        return OPRT_COM_ERROR;
    }

    if (msg_data != NULL && msg_data_len != 0) {
        data = (VOID_T *)tal_malloc(msg_data_len);
        if (NULL == data)
            return OPRT_MALLOC_FAILED;

        memset(data, 0, msg_data_len);
        memcpy(data, msg_data, msg_data_len);
    } else {
        data = NULL;
    }

    BYTE_T *buf = (BYTE_T *)data;
    for (int i = 0; i < msg_data_len; i++)
        TAL_PR_DEBUG_RAW("%02x ", buf[i]);
    TAL_PR_DEBUG_RAW("\r\n");

    op_ret = __add_msg_to_queue(msg_id, data, msg_data_len, msg_type);
    if (OPRT_OK != op_ret) {
        if (NULL != data)
            tal_free(data);
        return op_ret;
    }
    op_ret = tal_semaphore_post(ty_app_msg.sem_handle);
    if (OPRT_OK != op_ret) {
        if (NULL != data)
            tal_free(data);
        return op_ret;
    }

    return OPRT_OK;
}

STATIC OPERATE_RET ty_msg_queue_get(SYS_MSG_LIST_S **ppMsgListNode)
{
    OPERATE_RET op_ret = OPRT_OK;

    op_ret = tal_semaphore_wait_forever(ty_app_msg.sem_handle);
    if (OPRT_OK != op_ret) {
        return op_ret;
    }

    op_ret = tal_mutex_lock(ty_app_msg.mutex_handle);
    if (OPRT_OK != op_ret) {
        return op_ret;
    }

    SYS_MSG_LIST_S *pTmpMsgListNode = NULL;

    GetFirstNode(SYS_MSG_LIST_S, &ty_app_msg, list_head, pTmpMsgListNode);

    op_ret = tal_mutex_unlock(ty_app_msg.mutex_handle);
    if (OPRT_OK != op_ret) {
        return op_ret;
    }

    *ppMsgListNode = pTmpMsgListNode;

    return OPRT_OK;
}

STATIC OPERATE_RET ty_msg_queue_del(SYS_MSG_LIST_S *pMsgListNode)
{
    tal_mutex_lock(ty_app_msg.mutex_handle);

    tuya_list_del(&(pMsgListNode->list_head));

    tal_free(pMsgListNode);

    ty_app_msg.msgNodeNum--;

    tal_mutex_unlock(ty_app_msg.mutex_handle);

    return OPRT_OK;
}

STATIC VOID_T app_msg_queue_thread(VOID_T *param)
{
    OPERATE_RET op_ret = OPRT_OK;
    SYS_MSG_LIST_S *msg_node = NULL;
    SYS_MSG_CALLBACK msg_callback = NULL;

    while (1) {
        op_ret = ty_msg_queue_get(&msg_node);
        if (OPRT_OK != op_ret) {
            continue;
        }

        tal_mutex_lock(ty_app_msg.mutex_handle);
        if ((msg_node->msg.msg_id != MSGID_UNVALUED) && (msg_node->msg.msg_id < ty_app_msg.msgcb_pool_size)) {
            msg_callback = ty_app_msg.msg_cb[msg_node->msg.msg_id];
        }
        tal_mutex_unlock(ty_app_msg.mutex_handle);

        if (msg_callback) {
            msg_callback(&(msg_node->msg));
            msg_callback = NULL;
        }

        ty_msg_queue_del(msg_node);
    }
}

/**
 * @brief ty_app_msg_queue_init
 *
 * @param[in] : none
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_msg_queue_init(VOID_T)
{
    OPERATE_RET op_ret = OPRT_OK;

    op_ret = tal_mutex_create_init(&ty_app_msg.mutex_handle);
    if (OPRT_OK != op_ret) {
        TAL_PR_ERR("cre_mutex err");
        return OPRT_CR_MUTEX_ERR;
    }

    op_ret = tal_semaphore_create_init(&(ty_app_msg.sem_handle), 0, MSG_MAX_NUM);
    if (OPRT_OK != op_ret) {
        tal_mutex_release(ty_app_msg.mutex_handle);

        TAL_PR_ERR("cre_sem err");
        return op_ret;
    }

    INIT_LIST_HEAD(&(ty_app_msg.list_head)); // 初始化一个空的消息链

    ty_app_msg.msgNodeNum = 0;
    ty_app_msg.msgcb_num = 0;
    ty_app_msg.msgcb_pool_size = 0;

    THREAD_CFG_T thread_cfg = {.thrdname = "app_msg", .priority = THREAD_PRIO_1, .stackDepth = 2048 + 1024};

    op_ret = tal_thread_create_and_start(&ty_app_msg.msg_handle, NULL, NULL, app_msg_queue_thread, NULL, &thread_cfg);
    if (OPRT_OK != op_ret) {
        tal_semaphore_release(ty_app_msg.sem_handle);
        tal_mutex_release(ty_app_msg.mutex_handle);
        TAL_PR_ERR("start thread app_msg_queue_thread err");
        return op_ret;
    }

    return OPRT_OK;
}

/**
 * @brief ty_app_msg_queue_reg
 *
 * @param[in] : msg_cb
 * @param[in] : msg_id
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_app_msg_queue_reg(SYS_MSG_CALLBACK msg_cb, UINT16_T *msg_id)
{
    if (NULL == msg_cb || NULL == msg_id)
        return OPRT_INVALID_PARM;

    *msg_id = MSGID_UNVALUED;

    tal_mutex_lock(ty_app_msg.mutex_handle);

    if (ty_app_msg.msgcb_num >= ty_app_msg.msgcb_pool_size) {
        SYS_MSG_CALLBACK *cb_pool = NULL;
        ty_app_msg.msgcb_pool_size += MSGCB_POOL_STEP;
        cb_pool = tal_malloc(sizeof(SYS_MSG_CALLBACK) * ty_app_msg.msgcb_pool_size);
        if (NULL == cb_pool) {
            ty_app_msg.msgcb_pool_size -= MSGCB_POOL_STEP;
            tal_mutex_unlock(ty_app_msg.mutex_handle);
            return OPRT_MALLOC_FAILED;
        }

        memset(cb_pool, 0, sizeof(SYS_MSG_CALLBACK) * ty_app_msg.msgcb_pool_size);
        if (ty_app_msg.msgcb_num > 0) {
            memcpy(cb_pool, ty_app_msg.msg_cb, sizeof(SYS_MSG_CALLBACK) * ty_app_msg.msgcb_num);
        }

        if (ty_app_msg.msg_cb)
            tal_free(ty_app_msg.msg_cb);

        ty_app_msg.msg_cb = cb_pool;
    }

    UINT16_T i;
    for (i = 0; i < ty_app_msg.msgcb_pool_size; i++) {
        if (NULL == ty_app_msg.msg_cb[i]) {
            *msg_id = i;
            ty_app_msg.msg_cb[i] = msg_cb;
            ty_app_msg.msgcb_num++;
            tal_mutex_unlock(ty_app_msg.mutex_handle);
            return OPRT_OK;
        }
    }

    tal_mutex_unlock(ty_app_msg.mutex_handle);

    return OPRT_COM_ERROR;
}

OPERATE_RET ty_app_msg_queue_unreg(UINT16_T msg_id)
{
    if (msg_id >= ty_app_msg.msgcb_pool_size)
        return OPRT_INVALID_PARM;

    tal_mutex_lock(ty_app_msg.mutex_handle);
    if (ty_app_msg.msg_cb[msg_id]) {
        ty_app_msg.msgcb_num--;
        ty_app_msg.msg_cb[msg_id] = NULL;
    }
    tal_mutex_unlock(ty_app_msg.mutex_handle);

    return OPRT_OK;
}
