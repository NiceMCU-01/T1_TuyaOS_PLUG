/**
 * @file app_elec_delay_off_timer.c
 * @author www.tuya.com
 * @brief app_elec_delay_off_timer module is used to 
 * @version 0.1
 * @date 2023-04-19
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_app_config.h"
#include "ty_sys.h"

#if (defined(ENABLE_ELEC_DELAY_OFF_TIMER) && (ENABLE_ELEC_DELAY_OFF_TIMER==1))
#include "app_elec_delay_off_timer.h"
#include "uni_base64.h"

#include "tbl_countdown_timer.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define APP_ELEC_DELAY_OFF_KEY_NAME "inch_time_key"
#define DELAY_OFF_MIN_SEC (1)
#define DELAY_OFF_MAX_SEC (60*60)
#if (defined(ENABLE_TY_LOAD_OEM_PAR) && (ENABLE_TY_LOAD_OEM_PAR == 1))
#define ELEC_CHANNEL_NUM_USED   ELEC_CHANNEL_NUM
#elif defined(T1_PLUG_WAY_NUM)
#define ELEC_CHANNEL_NUM_USED   T1_PLUG_WAY_NUM
#else
#define ELEC_CHANNEL_NUM_USED   DEFAULT_ELEC_CHANNEL_NUM
#endif
/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    UINT16_T dpid;
    CHAR_T *key_name; // KV 存储名称
    APP_ELEC_DELAY_OFF_CB cb;
    UINT8_T number;
    APP_ELEC_DELAY_OFF_RAW_DATA_T raw_data[ELEC_CHANNEL_NUM_USED];
    TBL_CD_TM_HANDLE tbl_cd_hdl[ELEC_CHANNEL_NUM_USED];
} APP_ELEC_DELAY_OFF_T;

/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC APP_ELEC_DELAY_OFF_T sg_delay_off = {
    .dpid = ELEC_DELAY_OFF_DPID,
    .key_name = APP_ELEC_DELAY_OFF_KEY_NAME,
    .cb = NULL,
};

/***********************************************************
***********************function define**********************
***********************************************************/
STATIC VOID_T __app_delay_off_countdown_cb(TBL_CD_TM_HANDLE cd_hdl, IN UINT32_T remain_s, VOID_T *args)
{
    APP_ELEC_DELAY_OFF_RAW_DATA_T *raw_data = NULL;

    if (args == NULL) {
        return;
    }

    raw_data = (APP_ELEC_DELAY_OFF_RAW_DATA_T *)args;

    if (remain_s > 0) {
        return;
    }

    TAL_PR_DEBUG("delay off cb: %d", raw_data->channel + 1);

    if (NULL != sg_delay_off.cb) {
        sg_delay_off.cb(raw_data->channel + 1);
    }

    return;
}

STATIC OPERATE_RET __app_delay_off_wd_write(CHAR_T *key_name, APP_ELEC_DELAY_OFF_DATA_T *delay_off_data)
{
    OPERATE_RET rt = OPRT_OK;

    TUYA_CHECK_NULL_RETURN(key_name, OPRT_INVALID_PARM);
    TUYA_CHECK_NULL_RETURN(delay_off_data, OPRT_INVALID_PARM);

    TUYA_CALL_ERR_RETURN(wd_common_write(key_name, (BYTE_T *)delay_off_data->raw_data, delay_off_data->num * sizeof(APP_ELEC_DELAY_OFF_RAW_DATA_T)));

    return rt;
}

STATIC OPERATE_RET __app_delay_off_wd_read(CHAR_T *key_name, APP_ELEC_DELAY_OFF_DATA_T **delay_off_data)
{
    OPERATE_RET rt = OPRT_OK;
    APP_ELEC_DELAY_OFF_RAW_DATA_T *raw_data = NULL;
    UINT_T value_size = 0;

    TUYA_CHECK_NULL_RETURN(key_name, OPRT_INVALID_PARM);

    rt = wd_common_read(key_name, (BYTE_T **)&raw_data, &value_size);
    if (OPRT_OK != rt) {
        TAL_PR_DEBUG("kv not find %s", key_name);
        return rt;
    }

    TAL_PR_HEXDUMP_DEBUG("wd read", (UINT8_T *)raw_data, value_size);

    *delay_off_data = tal_malloc(sizeof(APP_ELEC_DELAY_OFF_DATA_T)+value_size);
    if (NULL == (*delay_off_data)) {
        rt = OPRT_MALLOC_FAILED;
    }

    (*delay_off_data)->num = value_size / sizeof(APP_ELEC_DELAY_OFF_RAW_DATA_T);
    memcpy((*delay_off_data)->raw_data, raw_data, value_size);

    if (NULL != raw_data) {
        wd_common_free_data((BYTE_T *)raw_data);
        raw_data = NULL;
    }

    return rt;
}

OPERATE_RET app_elec_delay_off_data_erase(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    UINT8_T i = 0;

    for (i=0; i<ELEC_CHANNEL_NUM_USED; i++) {
        TUYA_CALL_ERR_LOG(tbl_countdown_time_stop(sg_delay_off.tbl_cd_hdl[i]));
    }

    memset(sg_delay_off.raw_data, 0, ELEC_CHANNEL_NUM_USED*sizeof(APP_ELEC_DELAY_OFF_RAW_DATA_T));

    TUYA_CALL_ERR_RETURN(wd_common_delete(sg_delay_off.key_name));

    return rt;
}

OPERATE_RET app_elec_delay_off_dp_data_parse(CHAR_T *dp_string, APP_ELEC_DELAY_OFF_DATA_T **delay_off_data)
{
    OPERATE_RET rt = OPRT_OK;
    UINT32_T decode_len = 0;
    APP_ELEC_DELAY_OFF_DATA_T *out_data = NULL;
    UINT32_T malloc_size = 0;

    TUYA_CHECK_NULL_RETURN(dp_string, OPRT_INVALID_PARM);

    TAL_PR_DEBUG("delay off base64: %s", dp_string);

    malloc_size = strlen(dp_string) + sizeof(APP_ELEC_DELAY_OFF_DATA_T);
    out_data = tal_malloc(malloc_size);
    TUYA_CHECK_NULL_RETURN(out_data, OPRT_MALLOC_FAILED);
    *delay_off_data = out_data;
    
    //判断是否删除
    if (0 == strlen(dp_string)) {
        out_data->num = 0;
        return malloc_size;
    }

    decode_len = tuya_base64_decode(dp_string, (unsigned char *)out_data->raw_data);
    if (decode_len <= 0) {
        rt = OPRT_COM_ERROR;
        goto __ERR;
    }
    out_data->num = decode_len / sizeof(APP_ELEC_DELAY_OFF_RAW_DATA_T);

    TAL_PR_HEXDUMP_DEBUG("decode", (UINT8_T *)((*delay_off_data)->raw_data), decode_len);

    return malloc_size;

__ERR:
    if (NULL != out_data) {
        tal_free(out_data);
        out_data = NULL;
    }
    *delay_off_data = NULL;

    return rt;
}


OPERATE_RET app_elec_delay_off_init(APP_ELEC_DELAY_OFF_CB cb)
{
    OPERATE_RET rt = OPRT_OK;
    UINT8_T i = 0, idx = 0;
    APP_ELEC_DELAY_OFF_DATA_T *delay_off_data = NULL;

    TUYA_CHECK_NULL_RETURN(cb, OPRT_INVALID_PARM);
    sg_delay_off.cb = cb;

    memset(sg_delay_off.raw_data, 0, ELEC_CHANNEL_NUM_USED*sizeof(APP_ELEC_DELAY_OFF_RAW_DATA_T));

    rt = __app_delay_off_wd_read(sg_delay_off.key_name, &delay_off_data);
    if (OPRT_OK != rt) {
        rt = OPRT_OK;
        goto __EXIT;
    }

    for (i=0; i<delay_off_data->num; i++) {
        if (delay_off_data->raw_data[i].channel >= ELEC_CHANNEL_NUM_USED) {
            TAL_PR_DEBUG("channel id error: %d", delay_off_data->raw_data[i].channel);
            continue;
        }
        idx = delay_off_data->raw_data[i].channel;
        memcpy(&sg_delay_off.raw_data[idx], &delay_off_data->raw_data[i], sizeof(APP_ELEC_DELAY_OFF_RAW_DATA_T));
    }

__EXIT:
    for (i=0; i<ELEC_CHANNEL_NUM_USED; i++) {
        tbl_countdown_time_create(__app_delay_off_countdown_cb, &sg_delay_off.raw_data[i], &sg_delay_off.tbl_cd_hdl[i]);
    }

    app_elec_delay_off_start(0);

    if (NULL != delay_off_data) {
        tal_free(delay_off_data);
        delay_off_data = NULL;
    }

    return rt;
}

OPERATE_RET app_elec_delay_off_override_set(APP_ELEC_DELAY_OFF_DATA_T *data)
{
    OPERATE_RET rt = OPRT_OK;
    UINT8_T idx = 0, set_idx = 0;

    TUYA_CHECK_NULL_RETURN(data, OPRT_INVALID_PARM);

    TAL_PR_DEBUG("num: %d", data->num);
     if (data->num == 0) {
        TAL_PR_DEBUG("delay off num is 0, erase delay off data");
        app_elec_delay_off_data_erase();
        return OPRT_OK;
    }

    TAL_PR_HEXDUMP_DEBUG("delay off event", (UINT8_T *)data->raw_data, data->num*sizeof(APP_ELEC_DELAY_OFF_RAW_DATA_T));

    // 将点动开关数据写入 kv flash 中
    TUYA_CALL_ERR_LOG(__app_delay_off_wd_write(sg_delay_off.key_name, data));

    for (idx=0; idx<ELEC_CHANNEL_NUM_USED; idx++) {
        for (set_idx=0; set_idx<data->num; set_idx++) {
            if (idx == data->raw_data[set_idx].channel) {
                if (sg_delay_off.raw_data[idx].channel == data->raw_data[set_idx].channel && \
                    sg_delay_off.raw_data[idx].enable == data->raw_data[set_idx].enable && \
                    sg_delay_off.raw_data[idx].time_s == data->raw_data[set_idx].time_s) {
                    TAL_PR_DEBUG("delay off %d not update", data->raw_data[set_idx].channel);
                    break;
                }

                if (data->raw_data[set_idx].enable) {
                    TUYA_CALL_ERR_RETURN(tbl_countdown_time_start(sg_delay_off.tbl_cd_hdl[idx], 30, UNI_NTOHS(data->raw_data[set_idx].time_s)));
                    TAL_PR_DEBUG("delay off %d start %ds", data->raw_data[set_idx].channel, UNI_NTOHS(data->raw_data[set_idx].time_s));
                } else {
                    TUYA_CALL_ERR_RETURN(tbl_countdown_time_stop(sg_delay_off.tbl_cd_hdl[idx]));
                    TAL_PR_DEBUG("delay off %d stop", data->raw_data[set_idx].channel);
                }

                memcpy(&sg_delay_off.raw_data[idx], &data->raw_data[set_idx], sizeof(APP_ELEC_DELAY_OFF_RAW_DATA_T));
                break;
            } else if (data->num-1 == set_idx) {
                // 如果没有找到对应的通道，将该通道的定时器删除
                TUYA_CALL_ERR_RETURN(tbl_countdown_time_stop(sg_delay_off.tbl_cd_hdl[idx]));
                TAL_PR_DEBUG("delay off %d delete", idx);
                memset(&sg_delay_off.raw_data[idx], 0, sizeof(APP_ELEC_DELAY_OFF_RAW_DATA_T));
            }
        }
    }

    return rt;
}

OPERATE_RET app_elec_delay_off_start(UINT8_T channel_id)
{
    OPERATE_RET rt = OPRT_OK;
    UINT8_T i = 0, channel_idx = 0;

    if (channel_id > ELEC_CHANNEL_NUM_USED) {
        return OPRT_INVALID_PARM;
    }

    // 内部通道是从 0 开始的
    channel_idx = channel_id - 1;

    if (NULL == sg_delay_off.cb) {
        // 点动开关可能没有进行初始化，直接退出
        TAL_PR_DEBUG("delay off not init");
        return OPRT_OK;
    }

    if (0 == channel_id) {
        for (i=0; i<ELEC_CHANNEL_NUM_USED; i++) {
            if (UNI_NTOHS(sg_delay_off.raw_data[i].time_s) < DELAY_OFF_MIN_SEC || UNI_NTOHS(sg_delay_off.raw_data[i].time_s) > DELAY_OFF_MAX_SEC) {
                continue;
            }
            if (sg_delay_off.raw_data[i].enable && sg_delay_off.raw_data[i].channel == i) {
                TUYA_CALL_ERR_RETURN(tbl_countdown_time_start(sg_delay_off.tbl_cd_hdl[i], 30, UNI_NTOHS(sg_delay_off.raw_data[i].time_s)));
                TAL_PR_DEBUG("delay off %d start %ds", i+1, UNI_NTOHS(sg_delay_off.raw_data[i].time_s));
            } else {
                TUYA_CALL_ERR_RETURN(tbl_countdown_time_stop(sg_delay_off.tbl_cd_hdl[i]));
                TAL_PR_DEBUG("delay off %d stop", i+1);
            }
        }
    } else {
        if (UNI_NTOHS(sg_delay_off.raw_data[channel_idx].time_s) < DELAY_OFF_MIN_SEC || UNI_NTOHS(sg_delay_off.raw_data[channel_idx].time_s) > DELAY_OFF_MAX_SEC) {
            TAL_PR_DEBUG("E: delay off %d", UNI_NTOHS(sg_delay_off.raw_data[channel_idx].time_s));
            return OPRT_OK;
        }
        if (sg_delay_off.raw_data[channel_idx].enable && sg_delay_off.raw_data[channel_idx].channel == channel_idx) {
            TUYA_CALL_ERR_RETURN(tbl_countdown_time_start(sg_delay_off.tbl_cd_hdl[channel_idx], 30, UNI_NTOHS(sg_delay_off.raw_data[channel_idx].time_s)));
            TAL_PR_DEBUG("delay off %d start %ds", channel_id, UNI_NTOHS(sg_delay_off.raw_data[channel_idx].time_s));
        } else {
            TUYA_CALL_ERR_RETURN(tbl_countdown_time_stop(sg_delay_off.tbl_cd_hdl[channel_idx]));
            TAL_PR_DEBUG("delay off %d stop", channel_id);
        }
    }

    return rt;
}

OPERATE_RET app_elec_delay_off_str_get(CHAR_T **p_data)
{
    OPERATE_RET rt = OPRT_OK;
    UINT8_T delay_off_num = 0, i = 0;
    APP_ELEC_DELAY_OFF_RAW_DATA_T raw_data[ELEC_CHANNEL_NUM_USED] = {{0}};
    UINT16_T malloc_size = 0;
    CHAR_T *base64_encode = NULL;

    *p_data = NULL;

    for (i=0; i<ELEC_CHANNEL_NUM_USED; i++) {
        if (UNI_NTOHS(sg_delay_off.raw_data[i].time_s) >= DELAY_OFF_MIN_SEC && UNI_NTOHS(sg_delay_off.raw_data[i].time_s <= DELAY_OFF_MAX_SEC)) {
            memcpy(&raw_data[delay_off_num], &sg_delay_off.raw_data[i], sizeof(APP_ELEC_DELAY_OFF_RAW_DATA_T));
            TAL_PR_DEBUG("chan: %d, en: %d, time_s: %d", raw_data[delay_off_num].channel, raw_data[delay_off_num].enable, UNI_NTOHS(raw_data[delay_off_num].time_s));
            delay_off_num++;
        }
    }

    if (0 == delay_off_num) {
        TAL_PR_DEBUG("delay_off_num is 0");
        return OPRT_COM_ERROR;   
    }

    malloc_size = TY_BASE64_BUF_LEN_CALC(delay_off_num * sizeof(APP_ELEC_DELAY_OFF_RAW_DATA_T));
    if (0 == malloc_size) {
        TAL_PR_DEBUG("base64 buffer len is 0");
        return OPRT_COM_ERROR;
    }

    base64_encode = (CHAR_T *)tal_malloc(malloc_size);
    TUYA_CHECK_NULL_RETURN(base64_encode, OPRT_MALLOC_FAILED);

    tuya_base64_encode((const unsigned char *)raw_data, (char *)base64_encode, delay_off_num *sizeof(APP_ELEC_DELAY_OFF_RAW_DATA_T));
    *p_data = base64_encode;

    return rt;
}

OPERATE_RET app_elec_delay_off_dp_data_upload(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    UINT8_T tmp_str = '\0';
    TY_OBJ_DP_S dp_obj_data = {0};
    CHAR_T *base64_encode = NULL;

    TUYA_CALL_ERR_LOG(app_elec_delay_off_str_get(&base64_encode));

    if ((NULL == base64_encode) || (rt != OPRT_OK)) {
        TAL_PR_DEBUG("delay off dp_str is null, rt:%d", rt);
        dp_obj_data.value.dp_str = (CHAR_T *)&tmp_str;
    } else {
        dp_obj_data.value.dp_str = (CHAR_T *)base64_encode;
    }

    TAL_PR_DEBUG("delay off dp_str: %s", dp_obj_data.value.dp_str);

    dp_obj_data.dpid = sg_delay_off.dpid;
    dp_obj_data.type = PROP_STR;
    dp_obj_data.time_stamp = 0;

    TUYA_CALL_ERR_LOG(dev_report_dp_json_async(NULL, &dp_obj_data, 1));

    if (NULL != base64_encode) {
        tal_free(base64_encode);
        base64_encode = NULL;
    }

    return rt;
}

#endif /* ENABLE_ELEC_DELAY_OFF_TIMER */
