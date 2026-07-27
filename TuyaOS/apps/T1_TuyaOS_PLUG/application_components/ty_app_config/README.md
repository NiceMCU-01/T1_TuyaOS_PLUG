# ty_app_config

1.管理无线一公共配置

2.生成app IoTOSconfig 文件

3.根据app IoTOSconfig文件生成tuya_app.config

4.根据tuya_app.config 文件生成tuya_app_config.h文件



编译语句

sh build.sh <cmd>

	"*************************cmd list********************************"
	"***** set_config : generate app IoTOSconfig"
	"***** menuconfig : generate tuya_app.config and tuya_app_config.h"
	"***** config     : generate tuya_app_config.h"
	"***** clean      : clear app IoTOSconfig tuya_app.config"
	"*****************************************************************"
	
	"***** please input cmd! for example: sh build.sh menuconfig *****"