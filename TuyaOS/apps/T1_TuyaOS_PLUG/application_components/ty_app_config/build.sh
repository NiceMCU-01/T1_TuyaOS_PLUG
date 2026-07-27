#!/bin/bash
cd `dirname $0`

APP_CONFIG_PATH=$(pwd)

cd ../../

APP_NAME=`bash ./scripts/get_sub_dir.sh apps`
echo APP_NAME=$APP_NAME
echo USER_CMD=$USER_CMD

python3 -u $APP_CONFIG_PATH/set_app_iotosconfig.py $APP_NAME