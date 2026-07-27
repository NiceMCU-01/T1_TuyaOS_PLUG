#include "tuya_app_config.h"

#if (defined(ELEC_FFC_BEACON_REMOTE_EN) && (ELEC_FFC_BEACON_REMOTE_EN == 1))

#include "app_elec_ffc.h"
#include "ffc_app.h"

#include "app_beacon_remote.h"
#include <string.h>
#include "uni_log.h"
#include "tal_sw_timer.h"
#include "tfm_oem_cfg_parse.h"
#include "app_elec_channel.h"
#include "app_elec_led.h"
#include "tdl_led_manage.h"
#include "ty_app_elec_event_code.h"

/***********************************************************
*************************micro define***********************
***********************************************************/

#define WIFI_MESH_ONOFF_CMD 0xFF01  //ffc遥控器开关


/***********************************************************
*************************type define************************
***********************************************************/

/*ffc control*/
typedef enum {
    FFC_DIS,
    FFC_EN
} FFC_CFG_TYPE_E;

typedef struct {
    TIMER_ID mesh_timer;
    UCHAR_T blink_cnt;
    FFC_CFG_TYPE_E bFfcEnable;
    UCHAR_T ffc_select;
    UINT_T pair_time;
} APP_FFC_CFG_T;


/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC APP_FFC_CFG_T sg_ffc_cfg = {
    .mesh_timer = NULL,
    .blink_cnt = 0,
    .bFfcEnable = FFC_EN,
    .ffc_select = 0,
    .pair_time = 30
};


/**
 * @name: __mesh_timer_cb
 * @msg:  
 * @param {INUINT_T} timerID
 * @param {INPVOID_T} pTimerArg
 * @return {*}
 */
STATIC VOID __mesh_timer_cb(IN TIMER_ID timerID, IN PVOID_T pTimerArg)
{
    if (sg_ffc_cfg.blink_cnt >= 6) {
        tal_sw_timer_stop(sg_ffc_cfg.mesh_timer);
        app_elec_net_led_refresh();
        sg_ffc_cfg.blink_cnt = 0;
        return;
    }

    if (sg_ffc_cfg.blink_cnt == 0) {
        app_net_led_ffc_beacon_use_set(1);//wifi灯暂时不指示wifi状态
    }

    if (0 == sg_ffc_cfg.blink_cnt % 2) {
        app_elec_net_led_ffc_beacon_set(TDL_LED_ON);
    } else {
        app_elec_net_led_ffc_beacon_set(TDL_LED_OFF);
    }

    sg_ffc_cfg.blink_cnt++;
}


/**
 * @name: __local_wifi_mesh_recv_cb
 * @msg:  本地wifi mesh解析函数
 * @param {BYTE_T} *data_cmd
 * @param {SHORT_T} data_len
 * @return {*}
 */
STATIC INT_T __local_wifi_mesh_recv_cb(BYTE_T *data_cmd, SHORT_T data_len)
{
    USHORT_T cmd = 0;
    APP_ELEC_CHANNEL_CFG_T chan_cfg = {0};

    if ((NULL == data_cmd) || (0 == data_len)) {
        PR_ERR("Param err...");
        return OPRT_COM_ERROR;
    }

    if (FALSE == sg_ffc_cfg.bFfcEnable) {
        return OPRT_OK;
    }

    cmd = (USHORT_T)((*data_cmd << 8) | (*(data_cmd + 1)));
    PR_NOTICE("cmd %x", cmd);

    //遥控器控制开关逻辑和按键保持一致
    switch (cmd) {
    case WIFI_MESH_ONOFF_CMD:
        if (*(data_cmd + 2) == 0x01) {
            PR_NOTICE("remoter turn on!");
            chan_cfg.status = TRUE;
        } else {
            PR_NOTICE("remoter turn off!");
            chan_cfg.status = FALSE;
        }
        chan_cfg.chan_id = 0;
        ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_CHANNEL_STATUS_SET, &chan_cfg, SIZEOF(APP_ELEC_CHANNEL_CFG_T));
        break;

    default:
        PR_ERR("Unknown cmd!");
        break;
    }

    return OPRT_OK;
}

/**
 * @name: __local_wifi_mesh_state_cb
 * @msg:  
 * @param {INffc_cb_state_t} state
 * @return {*}
 */
STATIC INT_T __local_wifi_mesh_state_cb(IN ffc_cb_state_t state)
{
    OPERATE_RET op_ret = -1;
    STATIC BOOL_T init_flag = 0;

    PR_NOTICE("########--local wifi mesh state:%d", state);

    if (FALSE == sg_ffc_cfg.bFfcEnable) {
        return OPRT_OK;
    }

    if (FFC_BINDING_SUCCESS_STATE == state) {
        PR_NOTICE("*********** MESH BIND SUCCESS ***********");

        PR_NOTICE("MESH BIND SUCCESS  is %d", tal_system_get_free_heap_size());
        if (0 == init_flag) {
            op_ret = tal_sw_timer_create(__mesh_timer_cb, NULL, &sg_ffc_cfg.mesh_timer);
            if (OPRT_OK != op_ret) {
                PR_ERR("sys_add_timer create error");
                return OPRT_COM_ERROR;
            }
            sg_ffc_cfg.blink_cnt = 0;
            tal_sw_timer_start(sg_ffc_cfg.mesh_timer, 500, TAL_TIMER_CYCLE); /* 500ms blink */
            init_flag = 1;
            PR_NOTICE("MESH BIND SUCCESS 222 is %d", tal_system_get_free_heap_size());
        }
    }

    return OPRT_OK;
}


/**
 * @name: app_Wifi_Ffc_Register
 * @msg:  注册ffc功能
 * @param {*}
 * @return {*}
 */
VOID app_Wifi_Ffc_Register(VOID)
{
    STATIC BOOL_T FfcStatus = FALSE;
   
    if((1 == sg_ffc_cfg.ffc_select) && (0 != sg_ffc_cfg.pair_time)){
        if (TRUE == sg_ffc_cfg.bFfcEnable) {
            if (FALSE == FfcStatus) {
                ffc_init(FFC_SLAVER, __local_wifi_mesh_state_cb, __local_wifi_mesh_recv_cb);
                if (TRUE == sg_ffc_cfg.bFfcEnable) {
                    //ffc_bind(WIFI_FFC_BIND_TIMEOUT);
                    PR_NOTICE("wifi_pair_time %d", sg_ffc_cfg.pair_time);
                    ffc_bind(sg_ffc_cfg.pair_time);
                }
                FfcStatus = TRUE;
            }
        }
    }
    
}

/**
 * @name: app_wifi_ffc_init
 * @msg:  ffc初始化
 * @param {UCHAR_T} cfg
 * @return {*}
 */
VOID app_wifi_ffc_init(UINT_T select , UINT_T time)
{
    tal_wifi_lp_disable();
    tal_cpu_lp_disable();
    sg_ffc_cfg.bFfcEnable = TRUE;
    sg_ffc_cfg.ffc_select = select;
    sg_ffc_cfg.pair_time = time;
}

#endif
