# 继电器管理组件

关于驱动的详细内容介绍介绍可以查看 `tdd_relay_driver` 继电器驱动组件下的 README.md 文件。

继电器组件的示例代码如下：

```c
#include "tuya_cloud_types.h"
#include "tal_log.h"
#include "tdd_relay_elec.h"
#include "tdd_relay_mag.h"
#include "tdl_relay_manage.h"

#define ELEC_RELAY_NAME "elec_relay"
#define MAG_RELAY_NAME "mag_relay"

// 电保持继电器硬件注册
RELAY_STATUS_E elec_status = 0;
ELEC_RELAY_DRIVER_CONFIG_T elec_relay_cfg = {
    .pin = 15,
    .mode = TUYA_GPIO_PUSH_PULL,
    .level = TUYA_GPIO_LEVEL_HIGH,
};
TUYA_CALL_ERR_RETURN(tdd_relay_elec_register(ELEC_RELAY_NAME, elec_relay_cfg));
RELAY_HANDLE_T elec_handle = NULL;
TUYA_CALL_ERR_RETURN(tdl_relay_dev_find(ELEC_RELAY_NAME, &elec_handle));
TUYA_CALL_ERR_RETURN(tdl_relay_dev_open(elec_handle));

// 磁保持继电器硬件注册
RELAY_STATUS_E mag_status = 0;
MAG_RELAY_DRIVER_CONFIG_T mag_relay_cfg = {
    .on_pin = 14,
    .off_pin = 16,
    .mode = TUYA_GPIO_PUSH_PULL,
    .level = TUYA_GPIO_LEVEL_LOW,
    .hold_ms = 50,
};
TUYA_CALL_ERR_RETURN(tdd_relay_mag_register(MAG_RELAY_NAME, mag_relay_cfg));
RELAY_HANDLE_T mag_handle = NULL;
TUYA_CALL_ERR_RETURN(tdl_relay_dev_find(MAG_RELAY_NAME, &mag_handle));
TUYA_CALL_ERR_RETURN(tdl_relay_dev_open(mag_handle));

for (;;) {
    // 打开电保持继电器
    TUYA_CALL_ERR_RETURN(tdl_relay_dev_write(elec_handle, RELAY_STATUS_ON));
    TUYA_CALL_ERR_RETURN(tdl_relay_dev_read(elec_handle, &elec_status));
    TAL_PR_DEBUG("elec_status: %d", elec_status);
    // 打开磁保持继电器
    TUYA_CALL_ERR_RETURN(tdl_relay_dev_write(mag_handle, RELAY_STATUS_ON));
    TUYA_CALL_ERR_RETURN(tdl_relay_dev_read(mag_handle, &mag_status));
    TAL_PR_DEBUG("mag_status: %d", mag_status);
    tal_system_sleep(5000);
    // 关闭电保持继电器
    TUYA_CALL_ERR_RETURN(tdl_relay_dev_write(elec_handle, RELAY_STATUS_OFF));
    TUYA_CALL_ERR_RETURN(tdl_relay_dev_read(elec_handle, &elec_status));
    TAL_PR_DEBUG("elec_status: %d", elec_status);
    // 关闭磁保持继电器
    TUYA_CALL_ERR_RETURN(tdl_relay_dev_write(mag_handle, RELAY_STATUS_OFF));
    TUYA_CALL_ERR_RETURN(tdl_relay_dev_read(mag_handle, &mag_status));
    TAL_PR_DEBUG("mag_status: %d", mag_status);
    tal_system_sleep(5000);
}
```
