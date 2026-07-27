# 继电器驱动组件

目前适配的继电器驱动有下面两种继电器类型：

+ 电保持继电器
+ 磁保持继电器

## 电保持继电器

电保持继电器硬件注册时，配置结构体原型如下：

```c
typedef struct {
    TUYA_GPIO_NUM_E pin;
    TUYA_GPIO_MODE_E mode;
    TUYA_GPIO_LEVEL_E level;
}ELEC_RELAY_DRIVER_CONFIG_T;
```

+ `pin`： 为继电器控制引脚；
+ `mode`：继电器控制引脚初始化模式，具体内容查看 `tuya_cloud_types.h` 文件中的 `TUYA_GPIO_MODE_E` 枚举。
+ `level`：可设置参数有 `TUYA_GPIO_LEVEL_LOW` 和 `TUYA_GPIO_LEVEL_HIGH`。继电器导通时，控制引脚应的输出电平值。例：当 `pin` 为高电平时继电器导通，那么 `level` 应设置为 `TUYA_GPIO_LEVEL_HIGH`；当 `pin` 为低电平时继电器导通，那么 `level` 应设置为 `TUYA_GPIO_LEVEL_LOW`。

## 磁保持继电器

磁保持继电器硬件注册时，配置结构体原型如下：

```c
typedef struct {
    TUYA_GPIO_NUM_E on_pin;
    TUYA_GPIO_NUM_E off_pin;
    TUYA_GPIO_MODE_E mode;
    TUYA_GPIO_LEVEL_E level;
    UINT_T hold_ms;
}MAG_RELAY_DRIVER_CONFIG_T;
```

+ `on_pin` 和 `off_pin` ：磁保持继电器驱动组件设计时，没有使用正极性和反极性的概念。这里认为当 `on_pin` 产生一个高电平脉冲， `off_pin` 维持低电平时会打开磁保持继电器；当 `on_pin` 维持低电平， `off_pin` 产生一个高电平脉冲时会关闭磁保持继电器。
+ `mode`：继电器控制引脚初始化模式，具体内容查看 `tuya_cloud_types.h` 文件中的 `TUYA_GPIO_MODE_E` 枚举。
+ `level`：可设置参数有 `TUYA_GPIO_LEVEL_LOW` 和 `TUYA_GPIO_LEVEL_HIGH`。磁保持继电器初始化、打开和关闭动作过后平常状态下的电平值。例子：`level` 设置为 `TUYA_GPIO_LEVEL_LOW`，在初始化继电器时会将 `on_pin` 和 `off_pin` 都设置为低电平，在打开或关闭继电器产生的脉冲激励过后会将 `on_pin` 和 `off_pin` 都设置为低电平；`level` 设置为 `TUYA_GPIO_LEVEL_HIGH`，在初始化继电器时会将 `on_pin` 和 `off_pin` 都设置为高电平，在打开或关闭继电器产生的脉冲激励过后会将 `on_pin` 和 `off_pin` 都设置为高电平。

具体的示例使用代码请查看 `tdl_relay_manage` 组件下 README.md 文件。
