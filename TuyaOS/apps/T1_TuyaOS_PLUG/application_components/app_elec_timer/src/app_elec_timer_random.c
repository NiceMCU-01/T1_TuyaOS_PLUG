/**
 * @file app_elec_timer_random.c
 * @author www.tuya.com
 * @brief app_elec_timer_random module is used to 
 * @version 0.1
 * @date 2023-04-27
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_app_config.h"
#include "ty_sys.h"

#if (defined(ENABLE_ELEC_RANDOM_TIMER) && (ENABLE_ELEC_RANDOM_TIMER==1))
#include "uni_base64.h"
#include "app_elec_timer_random.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define ELEC_RANDOM_TM_KEY_NAME "dp_time_rnd_key"

/***********************************************************
***********************typedef define***********************
***********************************************************/
#pragma pack(1)
typedef struct {
    UCHAR_T   enable:    1;
    UCHAR_T   channel_id:  7;
}CTRL_BIT_T;

typedef struct {
    CTRL_BIT_T   ctrl;
    UCHAR_T      week;
    UCHAR_T      start_time_h;
    UCHAR_T      start_time_l;
    UCHAR_T      end_time_h;
    UCHAR_T      end_time_l;
}APP_ELEC_RANDOM_RAW_DATA_T;
#pragma pack()

typedef struct {
    UINT8_T dpid;
    TIMER_ID sw_tm_id;
    TM_RANDOM_INFORM_CB cb;
} APP_ELEC_RANDOM_TM_T;

/***********************************************************
********************function declaration********************
***********************************************************/

STATIC OPERATE_RET __app_elec_random_tm_memory_write(VOID_T);
/***********************************************************
***********************variable define**********************
***********************************************************/
STATIC APP_ELEC_RANDOM_TM_T sg_random_tm = {
    .sw_tm_id = NULL,
    .dpid = ELEC_RANDOM_TIMER_DPID,
    .cb = NULL,
};

/***********************************************************
***********************function define**********************
***********************************************************/
STATIC VOID_T __app_elec_random_timer_cb(UCHAR_T point_id, BASIC_TM_STATE_E timer_state)
{
    OPERATE_RET rt = OPRT_OK;
    RANDOM_STATE_E random_state = RANDOM_STATE_START;

    TBL_TM_RANDOM_POINT_CFG_T random_cfg = {{0}};
    UINT8_T channel_id = 0;
    TAL_PR_DEBUG("rand point_id: %d:, timer_state: %d", point_id, timer_state);
    TUYA_CALL_ERR_GOTO(tbl_random_timer_get_point_cfg(point_id, &random_cfg), __EXIT);
    channel_id = random_cfg.cfg_bit.obj_idx + 1;

    random_state = (timer_state==BASIC_TM_STATE_START) ? (RANDOM_STATE_START) : (RANDOM_STATE_END);

    if (NULL != sg_random_tm.cb) {
        sg_random_tm.cb(channel_id, random_state);
    }

__EXIT:
    if (BASIC_TM_STATE_END_FOREVER == timer_state) {
        app_elec_random_timer_upload();
        __app_elec_random_tm_memory_write();
    }

    return;
}

/* random time memory operation */
STATIC OPERATE_RET __app_elec_random_tm_memory_write(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    UCHAR_T random_cnt = 0;
    UINT8_T i = 0;
    TBL_TM_RANDOM_POINT_CFG_T *p_random_tm = NULL;

    random_cnt = tbl_random_timer_get_point_num();

    if (0 == random_cnt) {
        TUYA_CALL_ERR_RETURN(wd_common_delete(ELEC_RANDOM_TM_KEY_NAME));
        return OPRT_OK;
    } else {
        p_random_tm = (TBL_TM_RANDOM_POINT_CFG_T *)tal_malloc(random_cnt*SIZEOF(TBL_TM_RANDOM_POINT_CFG_T));
        TUYA_CHECK_NULL_RETURN(p_random_tm, OPRT_MALLOC_FAILED);
        memset(p_random_tm, 0, random_cnt*SIZEOF(TBL_TM_RANDOM_POINT_CFG_T));
    }

    for (i=0; i<random_cnt; i++) {
        TUYA_CALL_ERR_GOTO(tbl_random_timer_get_point_cfg(i, &p_random_tm[i]), __EXIT);
#ifndef ENABLE_ELEC_COMPATIBLE_DATA_FORMAT
        p_random_tm[i].start_time = UNI_NTOHS(p_random_tm[i].start_time);
        p_random_tm[i].end_time = UNI_NTOHS(p_random_tm[i].end_time);
#endif
    }

    TUYA_CALL_ERR_LOG(wd_common_write(ELEC_RANDOM_TM_KEY_NAME, (BYTE_T *)p_random_tm, random_cnt*SIZEOF(TBL_TM_RANDOM_POINT_CFG_T)));

__EXIT:
    if (NULL != p_random_tm) {
        tal_free(p_random_tm);
        p_random_tm = NULL;
    }

    return rt;
}

STATIC OPERATE_RET __app_elec_random_tm_memory_read(TBL_TM_RANDOM_POINT_CFG_T **p_tm, UCHAR_T *tm_num)
{
    OPERATE_RET rt = OPRT_OK;
    UINT_T read_len = 0;

    rt = wd_common_read(ELEC_RANDOM_TM_KEY_NAME, (BYTE_T **)p_tm, &read_len);
    if (OPRT_OK != rt) {
        *tm_num = 0;
        return rt;
    }

    *tm_num = read_len / SIZEOF(TBL_TM_RANDOM_POINT_CFG_T);

    return rt;
}

STATIC VOID_T __next_minute_dp_upload_cb(TIMER_ID timer_id, VOID_T *arg)
{
    OPERATE_RET rt = OPRT_OK;

    TUYA_CALL_ERR_LOG(app_elec_random_timer_upload());

    tal_sw_timer_delete(timer_id);
    sg_random_tm.sw_tm_id = NULL;

    return;
}

/* random time init */
OPERATE_RET app_elec_random_timer_init(TM_RANDOM_INFORM_CB inform_cb)
{
    OPERATE_RET rt = OPRT_OK;
    TBL_TM_RANDOM_POINT_CFG_T *p_tm = NULL;
    UCHAR_T tm_num = 0, i = 0;

    TUYA_CHECK_NULL_RETURN(inform_cb, OPRT_INVALID_PARM);

    sg_random_tm.cb = inform_cb;

    TUYA_CALL_ERR_RETURN(tbl_random_timer_init(BASIC_TM_START_TP_HALF, __app_elec_random_timer_cb));

    rt = __app_elec_random_tm_memory_read(&p_tm, &tm_num);
    if (OPRT_OK != rt || 0 == tm_num) {
        TAL_PR_DEBUG("Not find random timer data");
        rt = OPRT_OK;
        goto __EXIT;
    }

#ifndef ENABLE_ELEC_COMPATIBLE_DATA_FORMAT
    for (i=0; i<tm_num; i++) {
        p_tm[i].start_time = UNI_NTOHS(p_tm[i].start_time);
        p_tm[i].end_time = UNI_NTOHS(p_tm[i].end_time);
    }
#endif
    app_elec_random_timer_set(p_tm, tm_num);
    app_elec_random_timer_upload();

__EXIT:
    if (NULL != p_tm) {
        tal_free(p_tm);
        p_tm = NULL;
    }

    return rt;
}

/* random time set */
OPERATE_RET app_elec_random_timer_set(TBL_TM_RANDOM_POINT_CFG_T *p_cfg, UINT8_T cfg_num)
{
    OPERATE_RET rt = OPRT_OK;
    UINT8_T i = 0;

    POSIX_TM_S cur_posix_tm = {0};
    USHORT_T cur_min = 0;
    TBL_TM_RANDOM_POINT_CFG_T tmp_point = {{0}};
    TIME_MS remain_ms = 0;

    TUYA_CALL_ERR_RETURN(tbl_random_timer_stop());
    TUYA_CALL_ERR_RETURN(tbl_random_timer_delete_all_point());

    if (NULL == p_cfg || 0 == cfg_num) {
        goto __EXIT;
    }

    for (i=0; i<cfg_num; i++) {
        TUYA_CALL_ERR_RETURN(tbl_random_timer_add_point(&p_cfg[i], NULL, 0));
        if ((p_cfg[i].start_time < p_cfg[i].end_time) && p_cfg[i].week==0 && p_cfg[i].cfg_bit.en) {
            TAL_PR_DEBUG("random single timer");
            TUYA_CALL_ERR_LOG(tal_time_get_local_time_custom(0, &cur_posix_tm));
            cur_min = cur_posix_tm.tm_hour*60 + cur_posix_tm.tm_min;
            TUYA_CALL_ERR_LOG(tbl_random_timer_get_random_cfg(i, &tmp_point));
            if (OPRT_OK != rt) {
                continue;
            }
            TAL_PR_DEBUG("cur_min: %d, %d", cur_min, tmp_point.end_time);
            if (cur_min >= tmp_point.end_time) {
                // 当天单次定时，设置随机定时结束时间已经过去，下一个整分时刻上报关闭该定时
                tbl_random_timer_stop_point(i);
                TAL_PR_DEBUG("stop random : %d", i);
                if (NULL == sg_random_tm.sw_tm_id) {
                    remain_ms = (60 - cur_posix_tm.tm_sec) * 1000;
                    TAL_PR_DEBUG("cur tm_sec: %d, remain_ms: %d", cur_posix_tm.tm_sec, remain_ms);
                    tal_sw_timer_create(__next_minute_dp_upload_cb, NULL, &sg_random_tm.sw_tm_id);
                    tal_sw_timer_start(sg_random_tm.sw_tm_id, remain_ms, TAL_TIMER_ONCE);
                }
            }
        }
    }
    TUYA_CALL_ERR_LOG(tbl_random_timer_start());

__EXIT:
    TUYA_CALL_ERR_RETURN(__app_elec_random_tm_memory_write());

    return rt;
}

OPERATE_RET app_elec_random_timer_dp_data_set(CHAR_T *dp_data)
{
    OPERATE_RET rt = OPRT_OK;
    UINT8_T i = 0;
    UINT16_T dp_data_len = 0;
    APP_ELEC_RANDOM_RAW_DATA_T *decode_data = NULL;
    UINT16_T decode_len = 0;
    TBL_TM_RANDOM_POINT_CFG_T *tbl_random_data = NULL;
    UINT8_T tbl_data_number = 0;

    TY_OBJ_DP_S dp_obj_data = {0};

    if (NULL == sg_random_tm.cb) {
        TAL_PR_ERR("random time not init");
        return OPRT_COM_ERROR;
    }

    dp_obj_data.dpid = sg_random_tm.dpid;
    dp_obj_data.type = PROP_STR;
    dp_obj_data.value.dp_str = dp_data;
    dp_obj_data.time_stamp = 0;
    TUYA_CALL_ERR_LOG(dev_report_dp_json_async(NULL, &dp_obj_data, 1));

    // dp data is NULL, delete random time
    if (NULL == dp_data || 0 == strlen(dp_data)) {
        TAL_PR_DEBUG("random time clear");
        tbl_random_data = NULL;
        tbl_data_number = 0;
    } else {
        // base64 decode
        dp_data_len = strlen(dp_data);
        decode_data = tal_malloc(dp_data_len);
        TUYA_CHECK_NULL_RETURN(decode_data, OPRT_MALLOC_FAILED);
        decode_len = tuya_base64_decode(dp_data, (unsigned char *)decode_data);
        tbl_data_number = decode_len / SIZEOF(APP_ELEC_RANDOM_RAW_DATA_T);
        tbl_random_data = tal_malloc(tbl_data_number*SIZEOF(TBL_TM_RANDOM_POINT_CFG_T));
        if (NULL == tbl_random_data) {
            rt = OPRT_MALLOC_FAILED;
            goto __EXIT;
        }
        for (i=0; i<tbl_data_number; i++) {
            tbl_random_data[i].cfg_bit.en = decode_data[i].ctrl.enable;
            tbl_random_data[i].cfg_bit.obj_idx = decode_data[i].ctrl.channel_id;
            tbl_random_data[i].week = decode_data[i].week;
            tbl_random_data[i].start_time = (decode_data[i].start_time_h<<8 | decode_data[i].start_time_l);
            tbl_random_data[i].end_time = (decode_data[i].end_time_h<<8 | decode_data[i].end_time_l);
        }
    }

    app_elec_random_timer_set(tbl_random_data, tbl_data_number);

__EXIT:
    if (NULL != decode_data) {
        tal_free(decode_data);
        decode_data = NULL;
    }

    if (NULL != tbl_random_data) {
        tal_free(tbl_random_data);
        tbl_random_data = NULL;
    }

    return rt;
}

OPERATE_RET app_elec_random_timer_memory_easer(VOID_T)
{
    return wd_common_delete(ELEC_RANDOM_TM_KEY_NAME);
}

OPERATE_RET app_elec_random_timer_str_get(CHAR_T **p_data)
{
    OPERATE_RET rt = OPRT_OK;
    UCHAR_T random_cnt = 0;
    UINT8_T i = 0;
    UINT32_T base64_len = 0;
    CHAR_T *base64_data = NULL;
    APP_ELEC_RANDOM_RAW_DATA_T *p_random_tm = NULL;
    TBL_TM_RANDOM_POINT_CFG_T tbl_tm = {{0}};

    (*p_data) = NULL;

    random_cnt = tbl_random_timer_get_point_num();
    if (0 == random_cnt) {
        return OPRT_OK;
    }

    p_random_tm = (APP_ELEC_RANDOM_RAW_DATA_T *)tal_malloc(random_cnt*SIZEOF(APP_ELEC_RANDOM_RAW_DATA_T));
    TUYA_CHECK_NULL_RETURN(p_random_tm, OPRT_MALLOC_FAILED);
    memset(p_random_tm, 0, random_cnt*SIZEOF(APP_ELEC_RANDOM_RAW_DATA_T));

    for (i=0; i<random_cnt; i++) {
        TUYA_CALL_ERR_GOTO(tbl_random_timer_get_point_cfg(i, &tbl_tm), __EXIT);
        p_random_tm[i].ctrl.enable = tbl_tm.cfg_bit.en;
        p_random_tm[i].ctrl.channel_id = tbl_tm.cfg_bit.obj_idx;
        p_random_tm[i].week = tbl_tm.week;
        p_random_tm[i].start_time_h = (tbl_tm.start_time >> 8) & 0xFF;
        p_random_tm[i].start_time_l = tbl_tm.start_time & 0xFF;
        p_random_tm[i].end_time_h = (tbl_tm.end_time >> 8) & 0xFF;
        p_random_tm[i].end_time_l = tbl_tm.end_time & 0xFF;
    }

    // base64
    base64_len = TY_BASE64_BUF_LEN_CALC(random_cnt*sizeof(APP_ELEC_RANDOM_RAW_DATA_T));
    base64_data = (CHAR_T *)tal_malloc(base64_len);
    TUYA_CHECK_NULL_GOTO(base64_data, __EXIT);
    memset(base64_data, 0, base64_len);
    tuya_base64_encode((unsigned char *)p_random_tm, base64_data, random_cnt*SIZEOF(APP_ELEC_RANDOM_RAW_DATA_T));

    (*p_data) = base64_data;

__EXIT:
    if (NULL != p_random_tm) {
        tal_free(p_random_tm);
        p_random_tm = NULL;
    }

    return rt;
}

OPERATE_RET app_elec_random_timer_upload(VOID_T)
{
    OPERATE_RET rt = OPRT_OK;
    CHAR_T tmp_str = '\0', *upload_str = NULL;
    TY_OBJ_DP_S dp_obj_data = {0};

    if (NULL == sg_random_tm.cb) {
        TAL_PR_ERR("random time not init");
        return OPRT_COM_ERROR;
    }

    rt = app_elec_random_timer_str_get(&upload_str);
    if (OPRT_OK != rt || NULL == upload_str) {
        dp_obj_data.value.dp_str = &tmp_str;
    } else {
        dp_obj_data.value.dp_str = upload_str;
    }

    dp_obj_data.dpid = sg_random_tm.dpid;
    dp_obj_data.type = PROP_STR;
    dp_obj_data.time_stamp = 0;

    TUYA_CALL_ERR_LOG(dev_report_dp_json_async(NULL, &dp_obj_data, 1));

    if (NULL != upload_str) {
        tal_free(upload_str);
        upload_str = NULL;
    }

    return rt;
}

#endif /* ENABLE_ELEC_RANDOM_TIMER */
