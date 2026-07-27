/**
 * @file app_elec_channel.h
 * @author www.tuya.com
 * @brief app_elec_channel module is used to 
 * @version 0.1
 * @date 2023-03-22
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#ifndef __APP_ELEC_CHANNEL_H__
#define __APP_ELEC_CHANNEL_H__

#include "tuya_cloud_types.h"


#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
#define ELEC_CHANNEL_NUM_MAX         8

typedef UINT8_T ELEC_CHANNEL_CMD_E;
#define ELEC_CHANNEL_NUM_SET            (0) // 设置通道个数
#define ELEC_CHANNEL_NUM_GET            (1) // 获取通道个数

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef enum {
    STATE_OFF, // 关闭
    STATE_ON, // 开启
    STATE_TOGGLE // 反转通道状态
} APP_CHANNEL_STATE_E;

// 通道上电模式
typedef enum {
    MODE_TURN_OFF, // 全部关闭
    MODE_TURN_ON, // 全部打开
    MODE_MEMORY, // 断电记忆模式，断电前状态
    MODE_MAX,
} APP_CHANNEL_MODE_E;

typedef struct {
    UINT_T  chan_id; // 0： 所有通道，1-8： 通道0-通道8
    APP_CHANNEL_STATE_E status; // 通道要设置的状态
}APP_ELEC_CHANNEL_CFG_T;

/***********************************************************
********************function declaration********************
***********************************************************/

/**
 * @brief     通道初始化
 *
 * @param[in] none
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_channel_init(VOID_T);

/**
 * @brief     通道状态设置。触发所有通道反转，只要有一个通道开启，就会关闭所有通道状态；
 *                          只有所有通道都关闭时触发所有通道状态反转才会打开所有通道。
 *
 * @param[in] cfg: 查看上面 APP_ELEC_CHANNEL_CFG_T 的注释介绍
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_channel_status_set(APP_ELEC_CHANNEL_CFG_T *cfg);

/**
 * @brief     通道指示灯句柄设置
 *
 * @param[in] chan_id: 通道 1 - 通道 8，不可输入 0
 * @param[in] led_hdl: LED 句柄
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_chan_led_handle_set(UINT_T chan_id, PVOID_T led_hdl);

/**
 * @brief     获取所有通道状态
 *
 * @param[out] status: 通道状态（bit0：通道 1 状态，bit1：通道 2 状态...）
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_get_all_channel_status(UINT_T *status);

/**
 * @brief     获取指定通道状态
 *
 * @param[in] chan_id: 通道 1 - 通道 8，不可输入 0
 * 
 * @return STATE_OFF: 通道关闭；STATE_ON: 通道开启
 */
APP_CHANNEL_STATE_E app_elec_channel_status_get(UINT_T chan_id);

/**
 * @brief     通道模式设置，不会将设置的 mode 写入到 flash 中
 *
 * @param[in] mode: 通道模式。MODE_TURN_OFF 上电时关闭全部通道；
 *                              MODE_TURN_ON 上电时打开全部通道；
 *                              MODE_MEMORY：断电记忆模式，恢复到断电前状态。
 * 
 * @return none
 */
VOID_T app_elec_channel_mode_default_set(APP_CHANNEL_MODE_E mode);

/**
 * @brief     通道模式设置，会将设置的 mode 写入到 flash 中
 *
 * @param[in] mode: 通道模式。MODE_TURN_OFF 上电时关闭全部通道；
 *                              MODE_TURN_ON 上电时打开全部通道；
 *                              MODE_MEMORY：断电记忆模式，恢复到断电前状态。
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_channel_mode_set(APP_CHANNEL_MODE_E mode);

/**
 * @brief     通道模式设置
 *
 * @param[in] cmd: 配置命令。详情查看 ELEC_CHANNEL_CMD_E
 * @param[inout] arg: 与 cmd 配合使用，输入或输出参数
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_channel_config(ELEC_CHANNEL_CMD_E cmd, VOID_T *arg);

/**
 * @brief     擦除用户存储的和通道相关的数据
 *
 * @param[in] none
 * 
 * @return none
 */
VOID_T app_elec_channel_data_erase(VOID_T);

/**
 * @brief     上报指定通道状态
 *
 * @param[in] chan_id: 通道 1 - 通道 8，不可输入 0
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_channel_status_upload(UINT_T chan_id);

/**
 * @brief 上报指定通道状态，通过通道的方式。有该接口是因为 OEM 中通道的 DPID 是会通过配置文件进行配置的，不是不变的。
 *
 * @param[in] dpid: 通道 DPID
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_channel_status_upload_by_dpid(BYTE_T dpid);

/**
 * @brief     上报所有通道状态
 *
 * @param[in] none
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_all_channel_status_upload(VOID_T);

/**
 * @brief     上报通道模式状态
 *
 * @param[in] none
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET app_elec_channel_mode_upload(VOID_T);

/**
 * @brief 判断上电模式和通道状态是否已经存储过
 *
 * @param[in] none: 
 *
 * @return TRUE 已经有存储数据, FALSE 没有存储过
 */
BOOL_T app_elec_channel_data_is_save(VOID_T);

/**
 * @brief 设置指定通道 DPID
 *
 * @param[in] chan_id: 通道 1 - 通道 8，不可输入 0
 * @param[in] dpid: 对应通道的 DPID
 *
 * @return none
 */
VOID_T app_elec_channel_dpid_set(UINT_T chan_id, BYTE_T dpid);

/**
 * @brief 获取通道 LED 个数
 *
 * @param[in] none:
 *
 * @return 通道 LED 个数
 */
UINT8_T app_elec_channel_led_num_get(VOID_T);

OPERATE_RET app_elec_channel_led_refresh(VOID_T);

/**
 * @brief     运行时长开关状态上报
 *
 * @param[in] chan_idx: 通道索引（0-7），目前仅支持第一路（chan_idx=0）
 * 
 * @return none
 */
VOID_T app_run_time_switch_upload(UINT8_T chan_idx);
/**
 * @brief     本地缓存的运行时长数据补报
 *
 * @param[in] none
 * 
 * @return none
 */
VOID_T app_run_time_switch_local_upload(VOID_T);
/**
 * @brief     清除运行时长开关本地缓存数据
 *
 * @param[in] none
 * 
 * @return OPERATE_RET OPRT_OK on success, others on error
 */
OPERATE_RET app_run_time_switch_data_erase(VOID_T);
#ifdef __cplusplus
}
#endif

#endif /* __APP_ELEC_CHANNEL_H__ */
