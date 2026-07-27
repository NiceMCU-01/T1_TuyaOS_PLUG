# 应用程序-电子童锁

该模块提供了电子童锁的功能。您可以使用此模块锁定或解锁设备，并设置自动锁定功能。

## 目录
- [宏定义](#宏定义)
- [类型定义](#类型定义)
- [函数声明](#函数声明)
  - [app_elec_child_lock_init](#app_elec_child_lock_init)
  - [app_elec_child_lock_status_set](#app_elec_child_lock_status_set)
  - [app_elec_child_lock_status_get](#app_elec_child_lock_status_get)
  - [app_elec_child_auto_lock_status_set](#app_elec_child_auto_lock_status_set)
  - [app_elec_child_auto_lock_status_get](#app_elec_child_auto_lock_status_get)
  - [app_elec_child_lock_dp_data_upload](#app_elec_child_lock_dp_data_upload)
- [使用注意](#使用注意)

## 宏定义
- `STATUS_UNLOCK`：该值为`0`，表示设备已解锁。
- `STATUS_LOCK`：该值为`1`，表示设备已锁定。

## 类型定义
- `CHILD_LOCK_STATUS_E`：用于表示电子童锁状态的`UINT8_T`类型定义。
  - `STATUS_UNLOCK`：设备已解锁。
  - `STATUS_LOCK`：设备已锁定。
- `APP_ELEC_CHILD_LOCK_CB`：回调函数类型。当童锁状态发生改变时，将调用此函数。
- `APP_ELEC_CHILD_LOCK_CONFIG_T`：用于存储童锁配置的结构体类型定义。
  - `auto_lock_enable`：`UINT8_T`值，指示是否启用自动锁定功能（`0`: 未开启自动上锁功能；`1`: 开启自动上锁功能）。
  - `auto_lock_time_ms`：`UINT32_T`值，指示设备自动锁定的时间（以毫秒为单位）。

## 函数声明

### `app_elec_child_lock_init`

用于初始化电子童锁模块的函数。

  - 参数：
    - `usr_cfg`：指向配置结构体的指针。如果设置为`NULL`，将使用默认配置。
    - `cb`：童锁状态发生改变时将调用的回调函数。
  - 返回值：
    - `OPRT_OK`：操作成功。
    - 其他：请参考`tuya_error_code.h`。


### `app_elec_child_lock_status_set`

用于设置电子童锁状态的函数。

  - 参数：
    - `status`：`CHILD_LOCK_STATUS_E`值，指示要设置的状态（`STATUS_UNLOCK`或`STATUS_LOCK`）。
  - 返回值：
    - `OPRT_OK`：操作成功。
    - 其他：请参考`tuya_error_code.h`。


### `app_elec_child_lock_status_get`

用于获取当前电子童锁状态的函数。

  - 参数：
    - 无。
  - 返回值：
    - `CHILD_LOCK_STATUS_E`：当前电子童锁状态（`STATUS_UNLOCK`或`STATUS_LOCK`）。


### `app_elec_child_auto_lock_status_set`

设置自动上锁功能的函数，开启自动上锁后，设备解锁童锁后 `auto_lock_time_ms` 时间后会自动上锁。

  - 参数：
    - `auto_lock_enable`：自动上锁开关。0：关闭自动上锁功能；1：开启自动上锁功能。
  - 返回值：
    - `OPRT_OK`：操作成功。
    - 其他：请参考`tuya_error_code.h`。


### `app_elec_child_auto_lock_status_get`

用于获取当前电子童锁自动上锁状态的函数。

  - 参数：无。
  - 返回值：
    - `0`：自动上锁功能未开启。
    - `1`：自动上锁功能已开启。


### `app_elec_child_lock_dp_data_upload`

童锁 DP 上报函数。

  - 参数：无。
  - 返回值：
    - `OPRT_OK`：操作成功。
    - 其他：请参考`tuya_error_code.h`。


## 使用注意

当打开童锁的自动上锁功能后，每次调用 `app_elec_child_lock_status_set(STATUS_UNLOCK);` 接口进行解锁后，经过一段时间后都会自动上锁，当然上锁后锁的状态发生了变化，会通过初始化时传入的回调函数进行通知。当需要关闭童锁后不再进行自动上锁操作，需要在关闭童锁前调用 `app_elec_child_auto_lock_status_set(0);` 函数关闭自动上锁功能。
