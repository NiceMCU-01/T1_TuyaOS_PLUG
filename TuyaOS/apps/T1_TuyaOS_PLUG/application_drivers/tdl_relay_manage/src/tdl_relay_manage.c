/**
 * @file tdl_relay_manage.c
 * @author www.tuya.com
 * @brief tdl_relay_manage module is used to 
 * @version 0.1
 * @date 2023-03-23
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include <string.h>

#include "tal_log.h"
#include "tal_memory.h"
#include "tal_mutex.h"

#include "tdl_relay_driver.h"
#include "tdl_relay_manage.h"

/***********************************************************
************************macro define************************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct relay_device_node{
    struct relay_device_node *next;
    CHAR_T name[RELAY_DEV_NAME_LEN_MAX+1];
    UINT8_T status;
    RELAY_DRV_INTFS_T drv_intfs;
    RELAY_DRV_HANDLE drv_hdl;
    MUTEX_HANDLE mutex_hdl;
}RELAY_DEV_NODE_T, RELAY_DEV_LIST_T;

/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
RELAY_DEV_LIST_T sg_relay_list_head = {
    .next = NULL,
};

/***********************************************************
***********************function define**********************
***********************************************************/

RELAY_DEV_NODE_T *__tdl_relay_dev_find(CHAR_T *name)
{
    RELAY_DEV_NODE_T *relay_node = sg_relay_list_head.next;

    while (relay_node) {
        if (0 == strcmp(name, relay_node->name)) {
            return relay_node;
        }
        relay_node = relay_node->next;
    }

    return NULL;
}

OPERATE_RET tdl_relay_dev_find(CHAR_T *name, RELAY_HANDLE_T *handle)
{
    RELAY_DEV_NODE_T *node = NULL;

    if(NULL == name || NULL == handle) {
        return OPRT_INVALID_PARM;
    }

    node = __tdl_relay_dev_find(name);
    if (NULL == node) {
        TAL_PR_ERR("%s device not find", name);
        return OPRT_COM_ERROR;
    }

    *handle = (RELAY_HANDLE_T)node;

    return OPRT_OK;
}

OPERATE_RET tdl_relay_dev_open(RELAY_HANDLE_T handle)
{
    OPERATE_RET rt = OPRT_OK;
    RELAY_DEV_NODE_T *relay_dev = (RELAY_DEV_NODE_T *)handle;

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);

    tal_mutex_lock(relay_dev->mutex_hdl);

    TUYA_CHECK_NULL_RETURN(relay_dev->drv_intfs.open, OPRT_COM_ERROR);
    TUYA_CALL_ERR_RETURN(relay_dev->drv_intfs.open(relay_dev->drv_hdl));

    tal_mutex_unlock(relay_dev->mutex_hdl);

    return rt;
}

OPERATE_RET tdl_relay_dev_write(RELAY_HANDLE_T handle, RELAY_STATUS_E status)
{
    OPERATE_RET rt = OPRT_OK;
    RELAY_DEV_NODE_T *relay_dev = (RELAY_DEV_NODE_T *)handle;

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);

    tal_mutex_lock(relay_dev->mutex_hdl);

    TUYA_CHECK_NULL_RETURN(relay_dev->drv_intfs.write, OPRT_COM_ERROR);
    TUYA_CALL_ERR_GOTO(relay_dev->drv_intfs.write(relay_dev->drv_hdl, status), __EXIT);
    relay_dev->status = status;

__EXIT:
    tal_mutex_unlock(relay_dev->mutex_hdl);

    return rt;
}

OPERATE_RET tdl_relay_dev_write_without_lock(RELAY_HANDLE_T handle, RELAY_STATUS_E status)
{
    OPERATE_RET rt = OPRT_OK;
    RELAY_DEV_NODE_T *relay_dev = (RELAY_DEV_NODE_T *)handle;

    if (NULL == handle) {
        return OPRT_INVALID_PARM;
    }

    if (NULL == relay_dev->drv_intfs.write) {
        return OPRT_COM_ERROR;
    }

    rt = relay_dev->drv_intfs.write(relay_dev->drv_hdl, status);
    if (OPRT_OK != rt) {
        return OPRT_COM_ERROR;
    }
    relay_dev->status = status;

    return rt;
}

OPERATE_RET tdl_relay_dev_read(RELAY_HANDLE_T handle, RELAY_STATUS_E *status)
{
    RELAY_DEV_NODE_T *relay_dev = (RELAY_DEV_NODE_T *)handle;

    if (NULL == handle || NULL == status) {
        return OPRT_INVALID_PARM;
    }

    *status = relay_dev->status;

    return OPRT_OK;
}

OPERATE_RET tdl_relay_dev_close(RELAY_HANDLE_T handle)
{
    OPERATE_RET rt = OPRT_OK;
    RELAY_DEV_NODE_T *relay_dev = (RELAY_DEV_NODE_T *)handle;

    TUYA_CHECK_NULL_RETURN(handle, OPRT_INVALID_PARM);
    TUYA_CHECK_NULL_RETURN(relay_dev->drv_intfs.close, OPRT_COM_ERROR);

    tal_mutex_lock(relay_dev->mutex_hdl);

    TUYA_CALL_ERR_RETURN(relay_dev->drv_intfs.close(relay_dev->drv_hdl));

    tal_mutex_unlock(relay_dev->mutex_hdl);

    return rt;
}

OPERATE_RET tdl_relay_driver_register(IN CHAR_T *name, IN RELAY_DRV_INTFS_T *intfs, IN RELAY_DRV_HANDLE drv_hdl)
{
    OPERATE_RET rt = OPRT_OK;

    RELAY_DEV_NODE_T *list_tail = &sg_relay_list_head;
    RELAY_DEV_NODE_T *new_node = NULL;

    if (NULL == name || NULL == intfs || NULL == drv_hdl) {
        return OPRT_INVALID_PARM;
    }

    if (NULL != __tdl_relay_dev_find(name)) {
        TAL_PR_ERR("%s is already exists", name);
        return OPRT_COM_ERROR;
    }

    while(NULL != list_tail->next) {
        list_tail = list_tail->next;
    }

    new_node = tal_malloc(sizeof(RELAY_DEV_NODE_T));
    TUYA_CHECK_NULL_GOTO(new_node, __ERR);
    memset(new_node, 0, sizeof(RELAY_DEV_NODE_T));
    new_node->next = NULL;

    // 添加新节点
    list_tail->next = new_node;

    strncpy(new_node->name, name, RELAY_DEV_NAME_LEN_MAX);
    memcpy(&new_node->drv_intfs, intfs, sizeof(RELAY_DRV_INTFS_T));
    new_node->drv_hdl = drv_hdl;

    TUYA_CALL_ERR_GOTO(tal_mutex_create_init(&new_node->mutex_hdl), __ERR);

    return rt;

__ERR:
    if (NULL != new_node) {
        tal_free(new_node);
        new_node = NULL;
    }

    return rt;
}
