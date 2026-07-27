#include "tuya_app_config.h"

#if (defined(ELEC_FFC_BEACON_REMOTE_EN) && (ELEC_FFC_BEACON_REMOTE_EN == 1))

#include "app_beacon_remote.h"
#include <string.h>
#include <stdio.h>
#include "uni_log.h"
#include "tal_sw_timer.h"
#include "tfm_oem_cfg_parse.h"
#include "app_elec_channel.h"
#include "app_elec_led.h"
#include "tdl_led_manage.h"
#include "ty_app_elec_event_code.h"
#include "tfm_timing_storage.h"
#include "tal_bluetooth.h"
#include "tuya_ble_sdk.h"

#ifndef CONFIG_REMOTE_MAX_PAIR_NUM
#define CONFIG_REMOTE_MAX_PAIR_NUM              5
#endif

#define PAIR_LIST_KV_KEY                        "pair_list"
#define REMOTE_LOCK_KV_KEY                      "remote_lock_info"
#define REMOTE_UNBIND_KEY_CODE                  0x06
#define LED_FLASH_ONOFF_MS                      200
#define BLE_SCAN_HEARTBEAT_MS                   30000
#define REMOTE_KEY_DEBOUNCE_MS                  700

#if defined(T1_PLUG_WAY_NUM)
#define REMOTE_ACTIVE_CHANNEL_NUM               T1_PLUG_WAY_NUM
#else
#define REMOTE_ACTIVE_CHANNEL_NUM               DEFAULT_ELEC_CHANNEL_NUM
#endif

// =========================================================================
// 1. 定义事件枚举
// =========================================================================
typedef enum {
    REMOTE_CTRL_PRESS_START = 0,
    REMOTE_CTRL_PRESS_REPEAT,
    REMOTE_CTRL_PRESS_CLICK,
    REMOTE_CTRL_PRESS_DOUBLE_CLICK,
    REMOTE_CTRL_PRESS_REPEAT_CLICK,
    REMOTE_CTRL_PRESS_LONG_START,
    REMOTE_CTRL_PRESS_LONG_UP
} remote_ctrl_event_t;

void app_socket_ble_action_execute(remote_ctrl_event_t event, uint32_t remote_addr, uint16_t remote_code, uint16_t repeat);
STATIC OPERATE_RET __remote_lock_info_read(VOID_T);
STATIC BOOL_T __remote_lock_owner_match(uint16_t remote_addr);
STATIC BOOL_T __remote_control_allowed(uint16_t remote_addr);
STATIC OPERATE_RET __remote_lock_set(uint16_t remote_addr);
STATIC OPERATE_RET __remote_lock_clear(VOID_T);
STATIC OPERATE_RET __remote_lock_toggle(uint16_t remote_addr);
STATIC OPERATE_RET __ble_scan_restart(CONST CHAR_T *tag);
STATIC UINT16_T __remote_lock_key_code_get(VOID_T);
STATIC BOOL_T __remote_channel_toggle(uint32_t remote_addr, uint16_t remote_code, uint32_t current_time);

/* --- KV 存储结构 --- */
typedef struct {
    uint16_t id;
    uint16_t pair_code;
} ble_remote_pair1_t;

typedef struct {
    uint8_t lock_enable;
    uint16_t lock_remote_addr;
} ble_remote_lock_info_t;

ble_remote_pair1_t ble_remote_pair_list[CONFIG_REMOTE_MAX_PAIR_NUM] = {0};
STATIC ble_remote_lock_info_t sg_remote_lock_info = {0};

// --- 存储相关函数 ---
STATIC TIMER_ID led_restore_timer = NULL;

STATIC VOID __led_restore_cb(TIMER_ID timer_id, PVOID_T arg)
{
    TAL_PR_NOTICE("<<< LED Restore >>>");

    // 释放LED控制权
    app_net_led_ffc_beacon_use_set(0);

    // 恢复系统状态
    app_elec_net_led_refresh();
}

STATIC VOID_T led_flash_times(UINT8_T flash_times)
{
    TAL_PR_NOTICE("<<< ACTION: LED Flash %d Times >>>", flash_times);

    PVOID_T led_hdl = NULL;

    if (OPRT_OK != tdl_led_dev_find(ELEC_NET_LED_NAME, &led_hdl)) {
        tdl_led_dev_find(ELEC_POWER_LED_NAME, &led_hdl);
    }

    if (led_hdl == NULL) return;

    // 抢占LED（非常关键）
    app_net_led_ffc_beacon_use_set(1);

    TDL_LED_CONFIG_T led_cfg = {0};

    led_cfg.stat = TDL_LED_FLASH;
    led_cfg.flash_cnt = flash_times * 2;
    led_cfg.flash_first_time = LED_FLASH_ONOFF_MS;
    led_cfg.flash_second_time = LED_FLASH_ONOFF_MS;
    led_cfg.start_stat = TDL_LED_ON;
    led_cfg.end_stat = TDL_LED_OFF;  // 无所谓，后面会恢复

    tdl_led_ctrl(led_hdl, &led_cfg);

    // 创建恢复定时器（只创建一次）
    if (led_restore_timer == NULL) {
        tal_sw_timer_create(__led_restore_cb, NULL, &led_restore_timer);
    }

    // 计算闪烁总时间
    uint32_t total_time = led_cfg.flash_cnt * 
        (led_cfg.flash_first_time + led_cfg.flash_second_time);

    // 延时恢复（关键！！！）
    tal_sw_timer_start(led_restore_timer, total_time + 100, TAL_TIMER_ONCE);
}

OPERATE_RET cls_all_pair_code(void) {
    memset(ble_remote_pair_list, 0, sizeof(ble_remote_pair_list));
    tfm_kv_uf_storage_erase_data(PAIR_LIST_KV_KEY);
    tfm_kv_uf_storage_write_data(PAIR_LIST_KV_KEY, (UCHAR_T *)ble_remote_pair_list, sizeof(ble_remote_pair_list));
    __remote_lock_clear();
    return OPRT_OK;
}

OPERATE_RET cls_pair_code(uint16_t pair_code) {
    TAL_PR_NOTICE("KV: Executing Unpair for: 0x%04x", pair_code);
    led_flash_times(3);

    for(int j=0; j<CONFIG_REMOTE_MAX_PAIR_NUM; j++) {
        if(ble_remote_pair_list[j].id > 0 && ble_remote_pair_list[j].pair_code == pair_code) {
            ble_remote_pair_list[j].id = 0;
            ble_remote_pair_list[j].pair_code = 0;
        }
    }
    if (__remote_lock_owner_match(pair_code)) {
        __remote_lock_clear();
    }

    OPERATE_RET ret = tfm_kv_uf_storage_write_data(PAIR_LIST_KV_KEY, (UCHAR_T *)ble_remote_pair_list, sizeof(ble_remote_pair_list));
    return ret;
}

OPERATE_RET add_pair_code(uint16_t pair_code) {
    TAL_PR_NOTICE("KV: Executing Pair for: 0x%04x", pair_code);
    led_flash_times(3);

    uint16_t min_id = ble_remote_pair_list[0].id;
    uint16_t max_id = ble_remote_pair_list[0].id;
    uint8_t code_same = 0, code_same_index = 0;

    for(int j = 0; j < CONFIG_REMOTE_MAX_PAIR_NUM; j++) {
        min_id = ble_remote_pair_list[j].id < min_id ? ble_remote_pair_list[j].id : min_id;
        max_id = ble_remote_pair_list[j].id > max_id ? ble_remote_pair_list[j].id : max_id;

        if(ble_remote_pair_list[j].pair_code == pair_code){
            code_same = 1;
            code_same_index = j;
        }
    }

    if(min_id == max_id) { max_id = 0; }

    if(code_same) {
        ble_remote_pair_list[code_same_index].id = max_id + 1;
    } else {
        for(int i = 0; i < CONFIG_REMOTE_MAX_PAIR_NUM; i++) {
            if(ble_remote_pair_list[i].id == min_id) {
                ble_remote_pair_list[i].id = max_id + 1;
                ble_remote_pair_list[i].pair_code = pair_code;
                break;
            }
        }
    }
    
    OPERATE_RET ret = tfm_kv_uf_storage_write_data(PAIR_LIST_KV_KEY, (UCHAR_T *)ble_remote_pair_list, sizeof(ble_remote_pair_list));
    return ret;
}

uint8_t is_pair(uint16_t pair_code) {
    for(int j = 0; j < CONFIG_REMOTE_MAX_PAIR_NUM; j++) {
        if(ble_remote_pair_list[j].id > 0 && ble_remote_pair_list[j].pair_code == pair_code) {
            return 1;
        }
    }
    return 0;
}

STATIC OPERATE_RET __remote_lock_info_write(VOID_T)
{
    return tfm_kv_uf_storage_write_data(REMOTE_LOCK_KV_KEY, (UCHAR_T *)&sg_remote_lock_info, sizeof(sg_remote_lock_info));
}

STATIC OPERATE_RET __remote_lock_info_clear(VOID_T)
{
    sg_remote_lock_info.lock_enable = 0;
    sg_remote_lock_info.lock_remote_addr = 0;

    tfm_kv_uf_storage_erase_data(REMOTE_LOCK_KV_KEY);
    return __remote_lock_info_write();
}

STATIC OPERATE_RET __remote_lock_info_read(VOID_T)
{
    INT_T read_len = 0;

    memset(&sg_remote_lock_info, 0, sizeof(sg_remote_lock_info));
    read_len = tfm_kv_uf_storage_read_data(REMOTE_LOCK_KV_KEY, (UCHAR_T *)&sg_remote_lock_info, sizeof(sg_remote_lock_info));
    if (read_len != sizeof(sg_remote_lock_info)) {
        memset(&sg_remote_lock_info, 0, sizeof(sg_remote_lock_info));
        return OPRT_OK;
    }

    TAL_PR_NOTICE("KV Loaded -> lock_enable: %d, owner: 0x%04X",
                  sg_remote_lock_info.lock_enable, sg_remote_lock_info.lock_remote_addr);
    return OPRT_OK;
}

STATIC BOOL_T __remote_lock_owner_match(uint16_t remote_addr)
{
    return (sg_remote_lock_info.lock_enable == 1) && (sg_remote_lock_info.lock_remote_addr == remote_addr);
}

STATIC BOOL_T __remote_control_allowed(uint16_t remote_addr)
{
    if (sg_remote_lock_info.lock_enable == 0) {
        return TRUE;
    }

    return __remote_lock_owner_match(remote_addr);
}

STATIC OPERATE_RET __remote_lock_set(uint16_t remote_addr)
{
    sg_remote_lock_info.lock_enable = 1;
    sg_remote_lock_info.lock_remote_addr = remote_addr;
    TAL_PR_NOTICE("remote lock set, owner: 0x%04X", remote_addr);
    return __remote_lock_info_write();
}

STATIC OPERATE_RET __remote_lock_clear(VOID_T)
{
    TAL_PR_NOTICE("remote lock clear, last owner: 0x%04X", sg_remote_lock_info.lock_remote_addr);
    return __remote_lock_info_clear();
}

STATIC OPERATE_RET __remote_lock_toggle(uint16_t remote_addr)
{
    if (__remote_lock_owner_match(remote_addr)) {
        return __remote_lock_clear();
    }

    if (sg_remote_lock_info.lock_enable == 1) {
        TAL_PR_NOTICE("remote lock owner replace, old: 0x%04X, new: 0x%04X",
                      sg_remote_lock_info.lock_remote_addr, remote_addr);
    }

    return __remote_lock_set(remote_addr);
}

STATIC UINT16_T __remote_lock_key_code_get(VOID_T)
{
#if (defined(REMOTE_ACTIVE_CHANNEL_NUM) && (REMOTE_ACTIVE_CHANNEL_NUM == 1))
    return 0x42;
#else
    return 0x41;
#endif
}

STATIC OPERATE_RET __ble_scan_restart(CONST CHAR_T *tag)
{
    OPERATE_RET stop_rt = OPRT_OK;
    OPERATE_RET start_rt = OPRT_OK;
    TAL_BLE_SCAN_PARAMS_T scan_param = {0};

    scan_param.type = TAL_BLE_SCAN_TYPE_ACTIVE;
    scan_param.scan_interval = 0x90;
    scan_param.scan_window = 0x30;
    scan_param.filter_dup = 1;

    stop_rt = tal_ble_scan_stop();
    start_rt = tal_ble_scan_start(&scan_param);

    TAL_PR_NOTICE("ble scan restart[%s], stop_rt: %d, start_rt: %d", tag, stop_rt, start_rt);

    return start_rt;
}

STATIC UINT8_T __remote_channel_id_get(uint16_t remote_code)
{
    switch (remote_code) {
#if (defined(REMOTE_ACTIVE_CHANNEL_NUM) && (REMOTE_ACTIVE_CHANNEL_NUM == 1))
        case 0x42:
            return 1;
#elif (defined(REMOTE_ACTIVE_CHANNEL_NUM) && (REMOTE_ACTIVE_CHANNEL_NUM == 2))
        case 0x41:
            return 1;
        case 0x44:
            return 2;
#elif (defined(REMOTE_ACTIVE_CHANNEL_NUM) && (REMOTE_ACTIVE_CHANNEL_NUM == 3))
        case 0x41:
            return 1;
        case 0x42:
            return 2;
        case 0x44:
            return 3;
#else
        case 0x41:
            return 1;
        case 0x42:
            return 2;
        case 0x48:
            return 3;
        case 0x44:
            return 4;
#endif
        case 0x07:
            return 0;
        default:
            return 0xFF;
    }
}

STATIC BOOL_T __remote_channel_toggle(uint32_t remote_addr, uint16_t remote_code, uint32_t current_time)
{
    static uint32_t last_remote_addr = 0xFFFFFFFF;
    static uint16_t last_remote_code = 0xFFFF;
    static uint32_t last_trigger_time = 0;
    APP_ELEC_CHANNEL_CFG_T chan_cfg = {0};
    UINT8_T chan_id = __remote_channel_id_get(remote_code);

    if (chan_id == 0xFF) {
        return FALSE;
    }

    if (last_remote_addr == remote_addr && last_remote_code == remote_code &&
        (current_time - last_trigger_time) < REMOTE_KEY_DEBOUNCE_MS) {
        TAL_PR_NOTICE("BLE REMOTE -> debounce ignored, addr:0x%04X code:0x%02X dt:%d",
                      remote_addr, remote_code, (INT_T)(current_time - last_trigger_time));
        return TRUE;
    }

    last_remote_addr = remote_addr;
    last_remote_code = remote_code;
    last_trigger_time = current_time;

    chan_cfg.chan_id = chan_id;
    chan_cfg.status = 2;

    if (chan_id == 0) {
        TAL_PR_NOTICE("BLE REMOTE -> RELAY--all TOGGLE");
    } else {
        TAL_PR_NOTICE("BLE REMOTE -> RELAY--%d TOGGLE", chan_id);
    }

    ty_app_event_post(APP_EVT_GROUP_ELE, EVT_ELEC_CHANNEL_STATUS_SET, &chan_cfg, sizeof(APP_ELEC_CHANNEL_CFG_T));
    return TRUE;
}

OPERATE_RET read_pair_code(void) {
    memset(ble_remote_pair_list, 0, sizeof(ble_remote_pair_list));
    INT_T read_len = tfm_kv_uf_storage_read_data(PAIR_LIST_KV_KEY, (UCHAR_T *)ble_remote_pair_list, sizeof(ble_remote_pair_list));

    if (read_len > 0) {
        for(int j=0; j<CONFIG_REMOTE_MAX_PAIR_NUM; j++) {
            if(ble_remote_pair_list[j].id > 0) {
                TAL_PR_NOTICE("KV Loaded -> index: %d, id: %d, code: %04x", j, ble_remote_pair_list[j].id, ble_remote_pair_list[j].pair_code);
            }
        }
    }
    return OPRT_OK;
}

// =========================================================================
// 2. 动作执行逻辑
// =========================================================================
void app_socket_ble_action_execute(remote_ctrl_event_t event, uint32_t remote_addr, uint16_t remote_code, uint16_t repeat) {
    static uint8_t pair_init = 0;
    if(pair_init == 0) {
        pair_init = 1;
        __remote_lock_info_read();
    }

// --- 【调试日志：核心位置】 ---
    // 这个日志会告诉你：哪个设备(Addr)、按了哪个键(Code)、触发了什么动作(Event)、连击了几次(Repeat)
    char *event_str[] = {"START", "REPEAT", "CLICK", "DB_CLICK", "RE_CLICK", "L_START", "L_UP"};
    TAL_PR_NOTICE(">>> [BLE_DEBUG] Addr: 0x%04X | Code: 0x%02X | Event: %s | Repeat: %d", 
                  remote_addr, remote_code, event_str[event], repeat);

    uint32_t current_time = tal_system_get_millisecond();

    if(event == REMOTE_CTRL_PRESS_LONG_START && remote_code == __remote_lock_key_code_get()) {
        BOOL_T unlock = __remote_lock_owner_match(remote_addr);

        if(OPRT_OK == __remote_lock_toggle(remote_addr)) {
            led_flash_times(unlock ? 3 : 2);
        }
        return;
    }

    if(event == REMOTE_CTRL_PRESS_START && (FALSE == __remote_control_allowed(remote_addr))) {
        TAL_PR_NOTICE("remote control denied, lock_enable: %d, owner: 0x%04X, requester: 0x%04X",
                      sg_remote_lock_info.lock_enable, sg_remote_lock_info.lock_remote_addr, remote_addr);
        return;
    }

    if(event == REMOTE_CTRL_PRESS_START) {
        __remote_channel_toggle(remote_addr, remote_code, current_time);
    }
}

// =========================================================================
// 3. 数据解析逻辑：重构反死锁机制
// =========================================================================
// =========================================================================
// 3. 数据解析逻辑：重构按键状态机（支持单/双/多连击及长按）
// =========================================================================
#pragma pack(1)
typedef struct {
    uint16_t head; 
    uint16_t id;   
    uint8_t  sn;   
    uint8_t  cmd;  
    uint32_t _a;   
    uint8_t  tick; 
    uint8_t  _b[0]; // 对齐用
}_ble_pack_t;
#pragma pack()

#define LONG_TICK   20

void app_socket_ble_user_remote_handler(uint8_t *data, uint8_t len, uint8_t *mac){
    static uint16_t last_sn = 0xFFFF;
    static uint32_t curr_code = 0xFFFFFFFF;
    static uint32_t curr_addr = 0xFFFFFFFF;
    static uint32_t curr_repeat = 0;
    static uint32_t curr_press_tick = 0;
    static uint32_t start_press_tick = 0;

    if (data == NULL || len < sizeof(_ble_pack_t)) return;
    
    _ble_pack_t *pack = (_ble_pack_t *)data;

    if(pack->head != 0x5655){
        return;
    }

    if(pack->sn != last_sn){
        uint16_t remote_addr = pack->id;
        uint16_t remote_code = pack->cmd;
        last_sn = pack->sn;

        // 1. 刚刚按下 (tick == 1)
        if(pack->tick == 1){
            // 判断是否是新按键动作 (或者是抬起后重新按下)
            if(curr_code != remote_code){
                app_socket_ble_action_execute(REMOTE_CTRL_PRESS_START, remote_addr, remote_code, 0);
                curr_code = remote_code;
                curr_addr = remote_addr;
                curr_repeat = 1;
            }else{
                // 如果没有经历 tick==0 就收到了新的 tick==1，说明是极快速的连击
                curr_repeat += 1;
            }
            
            if(curr_repeat > 1 ){
                app_socket_ble_action_execute(REMOTE_CTRL_PRESS_REPEAT, remote_addr, remote_code, curr_repeat);
            }
        }
        // 2. 按键抬起 (tick == 0)
        else if(pack->tick == 0){
            if(curr_press_tick >= LONG_TICK){
                // 之前已经触发了长按，发送长按抬起
                app_socket_ble_action_execute(REMOTE_CTRL_PRESS_LONG_UP, remote_addr, remote_code, 0);
            }else{
                // 没有达到长按阈值，根据记录的连击次数分发事件
                if(curr_repeat == 1 || curr_repeat == 0){
                    app_socket_ble_action_execute(REMOTE_CTRL_PRESS_CLICK, remote_addr, remote_code, 0);
                }else if(curr_repeat == 2){
                    app_socket_ble_action_execute(REMOTE_CTRL_PRESS_DOUBLE_CLICK, remote_addr, remote_code, curr_repeat);
                }else{
                    app_socket_ble_action_execute(REMOTE_CTRL_PRESS_REPEAT_CLICK, remote_addr, remote_code, curr_repeat);
                }
            }

            // 状态机复位，准备迎接下一次按键
            curr_code = 0xFFFFFFFF;
            curr_addr = 0xFFFFFFFF;
            curr_repeat = 0;
            curr_press_tick = 0;
            start_press_tick = 0;
        }
        // 3. 持续按压中 (tick > 1)
        else{
            // 针对清码/解绑指令 (通常是0xBB或0x04) 做加速触发处理
            if(remote_addr == 0xBB || remote_code == 0x04 || remote_code == 0xBB){
                curr_press_tick = pack->tick + 15; // 加上偏移量，使其更快达到 LONG_TICK 阈值
            }else{
                curr_press_tick = pack->tick;
            }
            
            // 发送持续按压过程中的重复事件 (可用于无级调光等场景)
            app_socket_ble_action_execute(REMOTE_CTRL_PRESS_REPEAT, remote_addr, remote_code, 0x8000 | pack->tick);
            
            // 判定是否达到长按阈值，且仅触发一次 START
            if(curr_press_tick >= LONG_TICK && start_press_tick == 0){
                start_press_tick = curr_press_tick;
                app_socket_ble_action_execute(REMOTE_CTRL_PRESS_LONG_START, remote_addr, remote_code, 0);
            }
        }    
    }
}
// ---------------- 底层 RAW 抓包拆包 ----------------
STATIC VOID __ble_raw_scan_data_cb(TAL_BLE_ADV_REPORT_T *scan_info)
{
    if (scan_info == NULL || scan_info->p_data == NULL || scan_info->data_len == 0) {
        return;
    }

    uint8_t *data = scan_info->p_data;
    uint8_t len = scan_info->data_len;
    uint8_t *mac = scan_info->peer_addr.addr;

    uint8_t parse_len = 0; 
    while (parse_len < len) {
        uint8_t ad_len = data[parse_len];
        if (ad_len == 0 || parse_len + ad_len + 1 > len) break;
        
        uint8_t ad_type = data[parse_len + 1];
        uint8_t *ad_data = data + parse_len + 2;
        uint8_t ad_data_len = ad_len - 1;

        if (ad_type == 0x16) {
            app_socket_ble_user_remote_handler(ad_data, ad_data_len, mac);
        }
        parse_len += ad_len + 1;
    }
}

STATIC VOID __ble_app_scan_adv_cb(CHAR_T *data, UINT_T len, UCHAR_T* mac, UINT8_T type) { }

// =========================================================================
// 4. 初始化：心跳守护，确保扫描永不掉线
// =========================================================================

// 【新增核心】：心跳定时器回调，每 30 秒执行一次强制扫描激活
STATIC VOID __ble_scan_heartbeat_cb(TIMER_ID timer_id, PVOID_T arg) {
    __ble_scan_restart("heartbeat");
    return;
    TAL_BLE_SCAN_PARAMS_T scan_param = {0};
    scan_param.type = TAL_BLE_SCAN_TYPE_ACTIVE;
    scan_param.scan_interval = 0x90; 
    scan_param.scan_window = 0x30;   
    scan_param.filter_dup = 1;
    // 忽略返回值：如果被系统杀后台了就能重新激活，如果在扫描则无影响
    tal_ble_scan_start(&scan_param);
}

STATIC VOID __delay_start_scan_cb(TIMER_ID timer_id, PVOID_T arg) {
    TAL_PR_NOTICE(">>> Delayed Execution: Secure BLE Scan Start <<<");
    static TIMER_ID heartbeat_timer;
    
    tuya_ble_set_bind_window(300);
    __ble_scan_restart("delayed_start");

    if (heartbeat_timer == NULL) {
        tal_sw_timer_create(__ble_scan_heartbeat_cb, NULL, &heartbeat_timer);
    }
    tal_sw_timer_start(heartbeat_timer, BLE_SCAN_HEARTBEAT_MS, TAL_TIMER_CYCLE);
    return;

    tuya_ble_set_bind_window(300); 
    tal_ble_scan_stop();
    
    TAL_BLE_SCAN_PARAMS_T scan_param = {0};
    scan_param.type = TAL_BLE_SCAN_TYPE_ACTIVE;
    scan_param.scan_interval = 0x90; 
    scan_param.scan_window = 0x30;   
    scan_param.filter_dup = 1;
    tal_ble_scan_start(&scan_param);

    // 启动心跳守护 (每 30000ms = 30秒 唤醒一次扫描)
    static TIMER_ID heartbeat_timer_unused;
    tal_sw_timer_create(__ble_scan_heartbeat_cb, NULL, &heartbeat_timer);
    tal_sw_timer_start(heartbeat_timer, 100000, TAL_TIMER_CYCLE); 
}

OPERATE_RET app_beacon_remote_init(UINT_T pair_time){
    TAL_PR_NOTICE("Socket Remote Init V5.0 (Bugfix)...");
    
    tfm_timing_storage_init(500);
    __remote_lock_info_read();
    
    tuya_ble_reg_app_scan_adv_cb(__ble_app_scan_adv_cb);
    tuya_ble_reg_raw_scan_adv_cb(__ble_raw_scan_data_cb);
    
    static TIMER_ID delay_timer;
    tal_sw_timer_create(__delay_start_scan_cb, NULL, &delay_timer);
    tal_sw_timer_start(delay_timer, 5000, TAL_TIMER_ONCE); 
    
    return OPRT_OK;
}

#endif
