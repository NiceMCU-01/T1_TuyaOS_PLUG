#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##
# @file set_app_iotosconfig.py
# @brief 解析yaml文件，查找应用组件是否存在APPconfig文件，
#        如果存在则source到一个总APPconfig.default文件中
# @author huatuo
# @version 1.0.0
# @date 2021-10-10


import os
import sys
from ruamel import yaml  # pip3 install ruamel.yaml


APP_NAME  = "demo" if len(sys.argv)<2 else sys.argv[1]
MAKE_YAML = "apps/" + APP_NAME + "/app.yaml"
APPCONFIG  = "apps/" + APP_NAME +"/build" + "/APPconfig.default"

def set_app_iotosconfig(make_yaml, appconfig):
    f = open(make_yaml, "r")
    yaml_res = yaml.load(f.read(), yaml.RoundTripLoader)
    f.close()

    if 'dependencies' not in yaml_res.keys():
        return False

    dependencies = yaml_res['dependencies']

    print(make_yaml)
    print(appconfig)
    app_context = ""
    ty_app_context = ""
    app_mianmenu_context = ""
    app_deps = dependencies.get("application_components", [])
    for block in app_deps:
        if "Component" in block.keys():
            d = block['Component']
            if ('name' not in d.keys()) or ('locater' not in d.keys()):
                continue
            name = d['name']
            if name.startswith('ty_app_config'):
                locater = d['locater']
                config_path = os.path.join(locater, name, 'APPconfig')
                if os.path.exists(config_path):
                    app_mianmenu_context += ('rsource '+'\"../../../'+config_path+'\"'+'\n')
         
            elif name.startswith('ty_app'):
                locater = d['locater']
                config_path = os.path.join(locater, name, 'APPconfig')
                if os.path.exists(config_path):
                    ty_app_context += ('rsource '+'\"../../../'+config_path+'\"'+'\n')

            elif name.startswith('app'):
                locater = d['locater']
                config_path = os.path.join(locater, name, 'APPconfig')
                if os.path.exists(config_path):
                    app_context += ('rsource '+'\"../../../'+config_path+'\"'+'\n')
                    
    context = app_mianmenu_context + ty_app_context + app_context     
    print(context)        

    with open(appconfig, 'w+') as f:
        f.write(context)

    return True


def main():
    if not os.path.exists(MAKE_YAML):
        print("path error: ", MAKE_YAML)
        exit()
    config_dir = os.path.dirname(APPCONFIG)
    if not os.path.exists(config_dir):
        os.makedirs(config_dir)

    set_app_iotosconfig(MAKE_YAML, APPCONFIG)

    pass


if __name__ == '__main__':
    main()
    pass

