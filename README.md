# T1 TuyaOS 插座应用

这是一个面向 TuyaOS Wi-Fi SoC 的 T1 无计量插座/插排应用源码仓库。应用源码位于：

```text
TuyaOS/apps/T1_TuyaOS_PLUG/
```

该应用通过构建期配置 `T1_PLUG_WAY_NUM` 支持 1、2、3 或 4 路继电器通道，包含通道控制、本地按键、Wi-Fi 配网指示、Tuya DP 处理、上电状态、童锁、倒计时/定时及 Beacon 遥控器等功能。

## 快速开始

1. 通过涂鸦官方或其他授权渠道获取匹配的 TuyaOS 产品开发框架，并完成所需授权。
2. 在 Visual Studio Code 中安装 **Tuya Wind IDE** 插件，按该框架的流程准备 SDK 与构建环境。
3. 将本仓库中的 `TuyaOS/apps/T1_TuyaOS_PLUG` 目录复制到你的私有开发框架中：

   ```text
   <你的 TuyaOS 开发框架>/TuyaOS/apps/T1_TuyaOS_PLUG/
   ```

4. 在个人或公司私有的构建配置中设置已获授权的 `TY_PRODUCT_ID`，并按需要配置产品凭据。
5. 使用该开发框架的正常流程生成配置、编译、烧录和验证。

详细的使用限制、私有产品配置与验证说明见：[应用 README](TuyaOS/apps/T1_TuyaOS_PLUG/README.md)。

## 本仓库不包含的内容

本仓库仅公开应用层源码，不是完整的 TuyaOS SDK，也不是可直接量产的固件包。以下内容被有意排除：

- TuyaOS 开发框架、平台库、工具链、启动文件和固件构建产物。
- 产品 PID、Firmware Key、证书、量产凭据及其他产品专属敏感信息。
- 生成文件 `build/tuya_app.config` 和 `include/tuya_app_config.h`。

本仓库不授予 TuyaOS SDK 或其预编译组件的再分发许可。使用或发布前，请自行确认 TuyaOS SDK、第三方组件及本项目代码的适用许可与再分发权限。

## 安全与配置提示

- 公开源码不提供可用的产品 PID；请仅在私有、已获授权的构建配置中设置 `TY_PRODUCT_ID`。
- 不要提交产品 PID、密钥、证书、Wi-Fi 凭据、产测数据、日志或固件文件。
- `include/tuya_app_config.h` 是构建生成文件，**禁止手工编辑**。
- 构建成功不等同于烧录成功或设备功能验证通过。
