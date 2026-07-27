/**
 * @file event_loop.h
 * @author www.tuya.com
 * @brief event_loop module is used to event loop driver
 * @version 0.1
 * @date 2022-07-11
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__

#include "tuya_cloud_types.h"

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
    UINT32_T event_id;
    UINT32_T size;
    char data[0];
} APP_EVENT_MSG_S;

typedef UINT16_T EVENT_GROUP_ID;

typedef VOID_T (*EVENTLOOP_CALLBACK)(APP_EVENT_MSG_S *msg);

/***********************************************************
********************function declaration********************
***********************************************************/
/**
 * @brief         应用事件注册
 * 
 * @param[in]     group_id       事件组id
 * @param[in]     event_cb       事件组处理回调
 *
 * @return none
 */
OPERATE_RET ty_app_event_register(EVENT_GROUP_ID group_id, EVENTLOOP_CALLBACK event_cb);

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
int ty_app_event_post_instancy(EVENT_GROUP_ID grop_id, UINT32_T event_id, void *event_data, UINT32_T event_data_len);

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
OPERATE_RET ty_app_event_post(EVENT_GROUP_ID grop_id, UINT32_T event_id, void *event_data, UINT32_T event_data_len);

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
OPERATE_RET ty_app_event_post_synchronous(EVENT_GROUP_ID grop_id, UINT32_T event_id, void *event_data, UINT32_T event_data_len);


#ifdef __cplusplus
}
#endif

#endif /* __EVENT_LOOP_H__ */


