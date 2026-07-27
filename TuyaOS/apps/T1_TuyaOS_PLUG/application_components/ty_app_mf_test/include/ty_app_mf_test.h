/**
 * @file ty_app_mf_test.h
 * @author www.tuya.com
 * @brief ty_app_mf_test module is used to 
 * @version 0.1
 * @date 2022-12-05
 *
 * @copyright Copyright (c) tuya.inc 2022
 *
 */

#ifndef __TY_APP_MF_TEST_H__
#define __TY_APP_MF_TEST_H__

#include "mf_test.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/

/**
 * @brief prod test sub cmd
 */
typedef USHORT_T PT_CMD_E;
/*0xF0 was product test cmd,  sub-cmd below*/
#define CMD_EXIT_OFFLINE_TEST  0xFF01 // 退出产测命令
#define CMD_ENTER_OFFLINE_TEST 0xFF02 // 进入产测命令
#define CMD_BOOL_TEST          0xFF03
#define CMD_RAW_TEST           0xFF04 // 通用透传测试命令
//#define CMD_INFORM_TEST_RESULT 0xFF07 // 通知测试结果命令
//#define CMD_JSON_UPDATE        0x000D // JSON更新命令 *通用固件没有这个功能*
//#define CMD_READ_RELAY_STATUS  0x0039 // 读继电器开关状态命令
//#define CMD_READ_DLTJ_POWER    0x003A // 读电量统计功率命令
//#define CMD_CLEAR_RECORD_INFO  0x003B // 清本地记录信息命令
//#define CMD_UPDATE_DEV_STATE   0x003C // 设备状态变化上报命令,暂不使用
//#define CMD_DEV_SELF_CHECK     0x003D // 设备自检命令,主要用于按键测试
#define CMD_LED_TEST           0x0001 // LED功能测试
#define CMD_RELAY_TEST         0x0002 // 继电器测试
#define CMD_KEY_TEST           0x0003 // 按键功能测试
#define CMD_MOTOR_TEST         0x0007 // 电机测试
#define CMD_WR_ELEC_STAT       0x0008 // 写入电量统计校准参数
#define CMD_ELEC_CHCK          0x0009 // 电量校准
#define CMD_AGING_TEST         0x000B // 老化测试
#define CMD_IR_SEND_TEST       0x000C // 红外发射
#define CMD_IR_RECV_TEST       0x000D // 红外接收
#define CMD_IR_MODE_CHANGE     0x000E // 红外工作模式切换,即退出接收模式
#define CMD_GET_DEV_MAC        0x0013 // 读取设备mac命令
#define CMD_COM_DATA_CFG       0x0014 // 成品通用数据配置
#define CMD_GET_WIFI_RSSI      0x002B // 读取wifi信号强度
#define CMD_GET_FIRM_INFO      0x002C // 读固件信息(固件版本,固件标识符...)
#define CMD_ANALOG_SENSOR_TEST 0x0030 // 模拟量传感器测试
#define CMD_BEEP_TEST          0x0031 // 喇叭/蜂鸣器测试
#define CMD_COM_SWITCH_TEST    0x0033 // 通用开关量测试
#define CMD_RECORD_TEST        0x0034 // 录音播音测试
#define CMD_GEAR_TEST          0x0036 // 挡位测试
#define CMD_COM_TEST           0x0035 // 通用功能测试，需要单独解析对PID获取 testitem:pid的处理
#define CMD_GET_BLE_RSSI       0x0100 // 获取BLE的RSSI，用于校验双模蓝牙的功能
#define CMD_GET_RF_RSSI        0x0101 // 获取RF的RSSI，用于校验RF遥控器功能
/*cmd定义*/
#define CMD_UPGRADE_START 0x0F // 模组固件更新开始命令
#define CMD_UPGRADE_END   0x11 // 模组固件更新结束命令

/**
 * @brief prod test error code
 */
typedef enum {
    PT_TEST_OK,
    PT_ERR_INVALID        = 500000,
    PT_ERR_UART_RECV      = 500001,
    PT_ERR_LEN_ERR        = 500002,
    PT_ERR_DATA_CHECK     = 500003,
    PT_ERR_DATA_ERR       = 500004,
    PT_ERR_WR_FLASH       = 500005,
    PT_ERR_RD_FLASH       = 500006,
    PT_ERR_CRC32          = 500007,
    PT_ERR_MD5            = 500008,
    PT_ERR_NFIND_SSID     = 500009,
    PT_ERR_CJSON          = 500010,
    PT_ERR_DATA_NULL      = 500011,
    PT_ERR_DATA_CRC_ERR   = 500012,
    PT_ERR_MCU_NOACK      = 500100,
    PT_ERR_MCU_UNKOWN     = 500101,
    PT_ERR_MCU_NOT_CONN   = 500102,
} PT_ERR_E;

/**
 * @brief prod test cmd type
 */
typedef enum {
    PT_CMD_READ,
    PT_CMD_WRITE,
    PT_CMD_NOTIFY
} PT_CMD_TYPE_E;

/**
 * @brief prod test vaule type
 */
typedef enum {
    PT_VALUE_INVAILD,
    PT_VALUE_STRING,
    PT_VALUE_INT,
    PT_VALUE_DOUBLE,
    PT_VALUE_HEX
} PT_VALUE_TYPE_E;



typedef unsigned char TY_APP_MF_STATUS_E;
#define MF_STATUS_ENTER                 0x00
#define MF_STATUS_GPIOTEST_START        0x01  // GPIO开始
#define MF_STATUS_WR_CFG_FINISH         0x02  //写配置文件结束           


typedef OPERATE_RET (*TY_MF_STATUS_CALLBACK)(TY_APP_MF_STATUS_E status);

/***********************************************************
********************function declaration********************
***********************************************************/
/**
 * @brief     成品产测指令回调接口的实现
 * @param[in] : cmd          成品产测子命令字
 * @param[in] : data         指令内容
 * @param[in] : len          指令长度
 * @param[out] : ret_data    应答数据指针存放的位置
 * @param[out] : ret_len     应答数据长度
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 *
 */
OPERATE_RET ty_sys_user_product_test_cb(USHORT_T cmd, UCHAR_T *data, UINT_T len,\
                                        OUT UCHAR_T **ret_data, OUT USHORT_T *ret_len);

/**
 * @brief  模组产测状态回调注册
 *
 * @param[in] : callback    状态回调
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_sys_mf_status_cb_reg(TY_MF_STATUS_CALLBACK callback);

/**
 * @brief     mf test 成品产测回调注册
 *
 * @param[in] : callback     成品产测回调
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_sys_mf_product_test_reg(MF_USER_PRODUCT_TEST_CB callback);

/**
 * @brief 获取 mf test 初始化接口
 *
 * @param[in] : intf  初始化接口指针
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET ty_sys_mf_test_get_intf(MF_IMPORT_INTF_S *intf);




#ifdef __cplusplus
}
#endif

#endif /* __TY_APP_MF_TEST_H__ */
