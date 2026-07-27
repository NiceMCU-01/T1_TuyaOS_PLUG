/**
 * @file tuya_iot_wifi_api.h
 * @brief
 * @author tuya
 * @version 1.0.0
 * @date 2021-01-12
 * @copyright Copyright(C),2017, 涂鸦科技 www.tuya.comm
 */

#ifndef _TUYA_IOT_MATTER_WIFI_API_H
#define _TUYA_IOT_MATTER_WIFI_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_cloud_types.h"
#include "tuya_cloud_com_defs.h"
#include "tuya_cloud_wifi_defs.h"
#include "tuya_iot_com_api.h"
#include "gw_intf.h"

/***********************************************************
*************************micro define***********************
***********************************************************/

/***********************************************************
*************************variable define********************
***********************************************************/

/***********************************************************
*************************function define********************
***********************************************************/
/***********************************************************
*  Function: tuya_iot_wf_timeout_set
*  Desc:     set wifi timeout
*  Input:    upload in sec
*  Note: must call first
***********************************************************/
VOID tuya_iot_matter_wf_timeout_set(IN CONST UINT_T timeout);

/***********************************************************
*  Function: tuya_iot_matter_wf_soc_dev_init_param->The devcie consists of wifi soc
*  Input: cfg
*         cbs->tuya wifi sdk user callbacks,note cbs->dev_ug_cb is useless
*         product_key->product key/proudct id,get from tuya open platform
*         wf_sw_ver->wifi module software version format:xx.xx.xx (0<=x<=9)
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET tuya_iot_matter_wf_soc_dev_init_param(IN CONST GW_WF_CFG_MTHD_SEL cfg, IN CONST GW_WF_START_MODE start_mode,
                                                  IN CONST TY_IOT_CBS_S *cbs, IN CHAR_T *firmware_key,
                                                  IN CHAR_T *product_key, IN CHAR_T *wf_sw_ver);

/**
+ * @brief tuya_iot_wf_set_network_hostname
+ * 
+ * @param[in] hostname
+ * 
+ * @return OPERATE_RET
+*/
OPERATE_RET tuya_iot_wf_set_network_hostname(const char *hostname);
#ifdef __cplusplus
}
#endif

#endif  /*_TUYA_IOT_API_H*/

