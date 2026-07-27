/**
 * @file app_elec_button.c
 * @author www.tuya.com
 * @brief app_elec_button module is used to 
 * @version 0.1
 * @date 2023-03-21
 *
 * @copyright Copyright (c) tuya.inc 2023
 *
 */

#include "tuya_app_config.h"

#include "tal_log.h"

#include "tdd_button_gpio.h"

#include "app_elec_button.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define ELEC_BUTTON_NAME(seq)           ELEC_BUTTON_##seq##_NAME
#define ELEC_BUTTON_DEBOUNCE_MS(seq)    ELEC_BUTTON_##seq##_DEBOUNCE_MS
#define ELEC_BUTTON_LONG_START_MS(seq)  ELEC_BUTTON_##seq##_LONG_START_MS
#define ELEC_BUTTON_LONG_KEEP_MS(seq)   ELEC_BUTTON_##seq##_LONG_KEEP_MS
#define ELEC_BUTTON_REPEAT_COUNT(seq)   ELEC_BUTTON_##seq##_REPEAT_COUNT
#define ELEC_BUTTON_REPEAT_MS(seq)      ELEC_BUTTON_##seq##_REPEAT_MS
#define ELEC_BUTTON_SINGLE_CLICK(seq)   ELEC_BUTTON_##seq##_SINGLE_CLICK
#define ELEC_BUTTON_DOUBLE_CLICK(seq)   ELEC_BUTTON_##seq##_DOUBLE_CLICK
#define ELEC_BUTTON_LONG_PRESS(seq)     ELEC_BUTTON_##seq##_LONG_PRESS
#define ELEC_BUTTON_REPEAT_CLICK(seq)   ELEC_BUTTON_##seq##_REPEAT_CLICK

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    ELEC_BUTTON_FUNC_E    short_press;
    ELEC_BUTTON_FUNC_E    double_click;
    ELEC_BUTTON_FUNC_E    long_press;
    ELEC_BUTTON_FUNC_E    repeat_click;
}APP_BUTTON_FUNC_T;

typedef struct {
    TDL_BUTTON_HANDLE handle;
    CHAR_T *name;
    TDL_BUTTON_CFG_T cfg;
    APP_BUTTON_FUNC_T fun;
}APP_ELEC_BUTTON_T;

#define ELEC_BUTTON_DEF_CFG_INIT(seq) \
{ \
    .name = ELEC_BUTTON_NAME(seq), \
    .cfg.button_debounce_time = ELEC_BUTTON_DEBOUNCE_MS(seq), \
    .cfg.long_start_vaild_time = ELEC_BUTTON_LONG_START_MS(seq), \
    .cfg.long_keep_timer = ELEC_BUTTON_LONG_KEEP_MS(seq), \
    .cfg.button_repeat_vaild_count = ELEC_BUTTON_REPEAT_COUNT(seq), \
    .cfg.button_repeat_vaild_time = ELEC_BUTTON_REPEAT_MS(seq), \
    .fun.short_press = ELEC_BUTTON_SINGLE_CLICK(seq), \
    .fun.double_click = ELEC_BUTTON_DOUBLE_CLICK(seq), \
    .fun.long_press = ELEC_BUTTON_LONG_PRESS(seq), \
    .fun.repeat_click = ELEC_BUTTON_REPEAT_CLICK(seq), \
} \

/***********************************************************
********************function declaration********************
***********************************************************/


/***********************************************************
***********************variable define**********************
***********************************************************/
#if (defined(ENABLE_TY_LOAD_OEM_PAR) && (ENABLE_TY_LOAD_OEM_PAR == 1))
STATIC UINT32_T sg_button_num = ELEC_BUTTON_NUM;
#else
STATIC UINT32_T sg_button_num = DEFAULT_ELEC_BUTTON_NUM;
#endif

STATIC APP_ELEC_BUTTON_T sg_elec_button[ELEC_BUTTON_NUM_MAX] = {
#if (defined(ELEC_BUTTON_0_NAME))
    ELEC_BUTTON_DEF_CFG_INIT(0),
#endif
#if (defined(ELEC_BUTTON_1_NAME))
    ELEC_BUTTON_DEF_CFG_INIT(1),
#endif
#if (defined(ELEC_BUTTON_2_NAME))
    ELEC_BUTTON_DEF_CFG_INIT(2),
#endif
#if (defined(ELEC_BUTTON_3_NAME))
    ELEC_BUTTON_DEF_CFG_INIT(3),
#endif
#if (defined(ELEC_BUTTON_4_NAME))
    ELEC_BUTTON_DEF_CFG_INIT(4),
#endif
#if (defined(ELEC_BUTTON_5_NAME))
    ELEC_BUTTON_DEF_CFG_INIT(5),
#endif
#if (defined(ELEC_BUTTON_6_NAME))
    ELEC_BUTTON_DEF_CFG_INIT(6),
#endif
#if (defined(ELEC_BUTTON_7_NAME))
    ELEC_BUTTON_DEF_CFG_INIT(7),
#endif
#if (defined(ELEC_BUTTON_8_NAME))
    ELEC_BUTTON_DEF_CFG_INIT(8),
#endif
};

STATIC APP_ELEC_BUTTON_FUN_CB sg_app_button_cb = NULL;

/***********************************************************
***********************function define**********************
***********************************************************/

STATIC APP_ELEC_BUTTON_T *__app_elec_get_button_dev(CHAR_T *name)
{
    UINT_T i = 0;

    TUYA_CHECK_NULL_RETURN(name, NULL);

    for (i = 0; i < sg_button_num; i++) {
        if (0 == strcmp(name, sg_elec_button[i].name)) {
            return &sg_elec_button[i];
        }
    }

    return NULL;
}

STATIC VOID_T __app_elec_button_event_cb(IN CHAR_T *name, IN TDL_BUTTON_TOUCH_EVENT_E event, IN VOID *arg)
{
    ELEC_BUTTON_MODE_E mode = ELEC_BUTTON_MODE_SHORT;
    ELEC_BUTTON_FUNC_E fun  = ELEC_BUTTON_FUNC_EMPTY;

    TAL_PR_NOTICE("button event name:%s event:%d", name, event);

    APP_ELEC_BUTTON_T *p_dev = __app_elec_get_button_dev(name);
    if (NULL == p_dev) {
        return;
    }

    switch (event) {
        case (TDL_BUTTON_PRESS_SINGLE_CLICK):
            mode = ELEC_BUTTON_MODE_SHORT;
            fun = p_dev->fun.short_press;
        break;
        case (TDL_BUTTON_PRESS_DOUBLE_CLICK):
            mode = ELEC_BUTTON_MODE_DOUBLE_CLICK;
            fun = p_dev->fun.double_click;
        break;
        case (TDL_BUTTON_LONG_PRESS_START):
            mode = ELEC_BUTTON_MODE_LONG;
            fun = p_dev->fun.long_press;
        break;
        case (TDL_BUTTON_PRESS_REPEAT):
            mode = ELEC_BUTTON_MODE_REPEAT;
            fun = p_dev->fun.repeat_click;
        break;
        default : return;
    }

    if (NULL != sg_app_button_cb) {
        sg_app_button_cb(mode, fun);
    }

    return;
}

OPERATE_RET app_elec_button_init(APP_ELEC_BUTTON_FUN_CB func_cb)
{
    UINT_T i = 0;

    if(NULL == func_cb) {
        return OPRT_INVALID_PARM;
    }

    sg_app_button_cb = func_cb;

    for (i=0; i<sg_button_num; i++) {
        tdl_button_create(sg_elec_button[i].name, &sg_elec_button[i].cfg, &sg_elec_button[i].handle);
        if (NULL == sg_elec_button[i].handle) {
            TAL_PR_ERR("button create failed name:%s", sg_elec_button[i].name);
            continue;
        }
        TAL_PR_NOTICE("button init name:%s short:%d long:%d debounce:%d long_start:%d",
                      sg_elec_button[i].name,
                      sg_elec_button[i].fun.short_press,
                      sg_elec_button[i].fun.long_press,
                      sg_elec_button[i].cfg.button_debounce_time,
                      sg_elec_button[i].cfg.long_start_vaild_time);
        /* 注册对应的按键功能 */
        if (sg_elec_button[i].fun.short_press != ELEC_BUTTON_FUNC_EMPTY) {
            TAL_PR_NOTICE("button register short press");
            tdl_button_event_register(sg_elec_button[i].handle, TDL_BUTTON_PRESS_SINGLE_CLICK, __app_elec_button_event_cb);
        }
        if (sg_elec_button[i].fun.double_click != ELEC_BUTTON_FUNC_EMPTY) {
            TAL_PR_NOTICE("button register double click");
            tdl_button_event_register(sg_elec_button[i].handle, TDL_BUTTON_PRESS_DOUBLE_CLICK, __app_elec_button_event_cb);
        }
        if (sg_elec_button[i].fun.long_press != ELEC_BUTTON_FUNC_EMPTY) {
            TAL_PR_NOTICE("button register long press");
            tdl_button_event_register(sg_elec_button[i].handle, TDL_BUTTON_LONG_PRESS_START, __app_elec_button_event_cb);
        }
        if (sg_elec_button[i].fun.repeat_click != ELEC_BUTTON_FUNC_EMPTY) {
            TAL_PR_NOTICE("button register repeat click");
            tdl_button_event_register(sg_elec_button[i].handle, TDL_BUTTON_PRESS_REPEAT, __app_elec_button_event_cb);
        }
    }

    return OPRT_OK;
}

VOID_T app_elec_button_long_time_set(UINT8_T index, USHORT_T long_time_ms)
{
    if (index >= ELEC_BUTTON_NUM_MAX || 0 == long_time_ms) {
        return;
    }

    sg_elec_button[index].cfg.long_start_vaild_time = long_time_ms;
    return;
}

// sg_button_num
VOID_T app_elec_button_number_set(UINT8_T num)
{
    sg_button_num = num;
    return;
}
