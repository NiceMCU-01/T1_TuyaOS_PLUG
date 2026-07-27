# app_elec_dev_control

该模块为设备控制模块，包括了指示灯、通道和按键模块。

## 目录
- [按键模块](#按键模块)
  - [按键-数据结构](#按键-数据结构)
  - [按键-函数说明](#按键-函数说明)
    - [app_elec_button_init](#app_elec_button_init)
- [通道模块](#通道模块)
  - [通道-数据结构](#通道-数据结构)
  - [通道-函数说明](#通道-函数说明)
    - [app_elec_channel_init()](#app_elec_channel_init)
    - [app_elec_channel_status_set()](#app_elec_channel_status_set)
    - [app_elec_get_all_channel_status()](#app_elec_get_all_channel_status)
    - [app_elec_channel_status_get()](#app_elec_channel_status_get)
    - [app_elec_channel_mode_set()](#app_elec_channel_mode_set)
    - [app_elec_channel_config()](#app_elec_channel_config)

## 按键模块

### 按键-数据结构

#### ELEC_BUTTON_MODE_E

```c
typedef UINT_T ELEC_BUTTON_MODE_E;

#define ELEC_BUTTON_MODE_SHORT          0x01
#define ELEC_BUTTON_MODE_DOUBLE_CLICK   0x02
#define ELEC_BUTTON_MODE_LONG           0x04
#define ELEC_BUTTON_MODE_REPEAT         0x08
```

该枚举类型定义了按键支持的支持的操作模式。

#### ELEC_BUTTON_FUNC_E

```c
typedef UINT8_T ELEC_BUTTON_FUNC_E;

typedef UINT8_T ELEC_BUTTON_FUNC_E;
#define ELEC_BUTTON_FUNC_EMPTY                  0 // 无功能
#define ELEC_BUTTON_CHANNEL_ALL_TOGGLE          1 // 所有通道反转。只要有一个通道打开，就会关闭所有通道；只有所有都关闭才会打开所有通道。
#define ELEC_BUTTON_CHANNEL_1_TOGGLE            2 // 通道 1 状态反转
#define ELEC_BUTTON_CHANNEL_2_TOGGLE            3
#define ELEC_BUTTON_CHANNEL_3_TOGGLE            4
#define ELEC_BUTTON_CHANNEL_4_TOGGLE            5
#define ELEC_BUTTON_CHANNEL_5_TOGGLE            6
#define ELEC_BUTTON_CHANNEL_6_TOGGLE            7
#define ELEC_BUTTON_CHANNEL_7_TOGGLE            8
#define ELEC_BUTTON_CHANNEL_8_TOGGLE            9
#define ELEC_BUTTON_LOCAL_RESET                 10 // 本地移除设备
#define ELEC_BUTTON_CHILD_LOCK_UNLOCK           11 // 解锁童锁
```

该枚举类型定义了按键支持的功能类型。

#### APP_ELEC_BUTTON_FUN_CB

```c
typedef VOID_T (*APP_ELEC_BUTTON_FUN_CB)(ELEC_BUTTON_MODE_E mode, ELEC_BUTTON_FUNC_E fun);
```

按键功能回调函数。

### 按键-函数说明

#### app_elec_button_init

```c
OPERATE_RET app_elec_button_init(APP_ELEC_BUTTON_FUN_CB func_cb);
```

初始化电力点动开关的按键功能。

- 参数
    - `func_cb`：当按键按下且该按键的操作有对应的功能，就会调用该回调函数。
- 返回值
    - 返回 `OPRT_OK` 表示成功，其他返回值请参考 `tuya_error_code.h`。

## 通道模块

此代码库提供了对电子通道的管理和控制，包括通道状态设置、获取和通道模式设置等。

### 通道-数据结构

#### APP_ELEC_CHANNEL_CFG_T

```c
typedef struct {
    UINT_T  chan_id; // 0： 所有通道，1-8： 通道0-通道8
    APP_CHANNEL_STATE_E status; // 通道要设置的状态
} APP_ELEC_CHANNEL_CFG_T;
```

通道状态设置结构体，包括通道 ID 和状态。

#### APP_CHANNEL_STATE_E

```c
typedef enum {
    STATE_OFF, // 关闭
    STATE_ON, // 开启
    STATE_TOGGLE // 反转通道状态
} APP_CHANNEL_STATE_E;
```

通道状态枚举类型，包括关闭、开启和反转状态。

#### APP_CHANNEL_MODE_E

```c
// 通道上电模式
typedef enum {
    MODE_TURN_OFF, // 全部关闭
    MODE_TURN_ON, // 全部打开
    MODE_MEMORY, // 断电记忆模式，断电前状态
    MODE_MAX,
} APP_CHANNEL_MODE_E;
```

通道上电模式枚举类型，包括全关、全开和断电记忆模式等。

#### ELEC_CHANNEL_CMD_E

```c
typedef UINT8_T ELEC_CHANNEL_CMD_E;
#define ELEC_CHANNEL_NUM_SET            (0) // 设置通道个数
#define ELEC_CHANNEL_NUM_GET            (1) // 获取通道个数
```

电子通道控制命令枚举类型，包括设置通道个数、获取通道个数、设置总控指示灯名称、使能总控指示灯和不使能总控指示灯等。

> 由于总控指示灯可能会与网络指示灯使用同一个硬件。使用同一个硬件的情况下，当指示灯在指示网络状态的时候，总控指示灯的功能应先暂时关闭；指示灯不在用作指示网络之后在打开总控指示功能。

### 通道-函数说明

#### app_elec_channel_init

```c
OPERATE_RET app_elec_channel_init(VOID_T);
```

初始化电力点动开关的通道。

- 参数
    - 无
- 返回值
    - 返回 `OPRT_OK` 表示成功，其他返回值请参考 `tuya_error_code.h`。

#### app_elec_channel_status_set

```c
OPERATE_RET app_elec_channel_status_set(APP_ELEC_CHANNEL_CFG_T *cfg);
```

> 设置通道状态，触发所有通道反转，只要有一个通道开启，就会关闭所有通道状态；只有所有通道都关闭时触发所有通道状态反转才会打开所有通道。

- 参数
    - `cfg`：通道状态配置结构体，包括通道ID和要设置的状态。其中：
        - `chan_id`：通道ID，取值范围为0-8，0表示所有通道；
        - `status`：通道要设置的状态，枚举类型，取值范围为 `APP_CHANNEL_OFF`（关闭）和 `APP_CHANNEL_ON`（开启）。
- 返回值
    - 返回 `OPRT_OK` 表示成功，其他返回值请参考 `tuya_error_code.h`。

#### app_elec_get_all_channel_status

```c
OPERATE_RET app_elec_get_all_channel_status(UINT_T *status);
```

获取所有通道的状态。

- 参数
    - `status`：指向存储通道状态的变量的指针。status 中每个位表示一个通道的状态（bit0：通道 1 状态，bit1：通道 2 状态...）。

- 返回值
    - 返回 `OPRT_OK` 表示成功，其他返回值请参考 `tuya_error_code.h`。

#### app_elec_channel_status_get

```c
APP_CHANNEL_STATE_E app_elec_channel_status_get(UINT_T chan_id);
```

获取指定通道的状态。

- 参数
    - `chan_id`：要获取状态的通道，取值范围为 1 - 8，不能为 0。
- 返回值
    - 返回 `STATE_OFF` 表示通道关闭；返回 `STATE_ON` 表示通道开启。

#### app_elec_channel_mode_set

```c
OPERATE_RET app_elec_channel_mode_set(APP_CHANNEL_MODE_E mode);
```

设置电力点动开关的通道模式。

- 参数
    - `mode`：通道模式。可选值为：
        - `MODE_TURN_OFF`：上电时关闭全部通道。
        - `MODE_TURN_ON`：上电时打开全部通道。
        - `MODE_MEMORY`：断电记忆模式，恢复到断电前状态。
- 返回值
    - 返回 `OPRT_OK` 表示成功，其他返回值请参考 `tuya_error_code.h`。

### app_elec_channel_config

```c
OPERATE_RET app_elec_channel_config(ELEC_CHANNEL_CMD_E cmd, VOID_T *arg);
```

对通道功能进行配置。

- 参数
  - `cmd`：配置命令。具体见 `ELEC_CHANNEL_CMD_E` 定义。
  - `arg`：与 `cmd` 配合使用，输入或输出参数。
- 返回值
  - 返回 `OPRT_OK` 表示成功，其他返回值请参考 `tuya_error_code.h`。
