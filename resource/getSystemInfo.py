#!/usr/bin/env python3
# coding: utf-8

import os
import configparser
from gi.repository import GLib
import sys

#print(sys.argv[0])          #sys.argv[0] 类似于shell中的$0,但不是脚本名称，而是脚本的路径   
#print(sys.argv[1])          #sys.argv[1] 表示传入的第一个参数
#获取版本和授权相关信息
cf = configparser.ConfigParser()
info = ""
text = ""
kylin_version = ""
info_build = ""
update_info = ""
phoneTips = ""
versionTip = ""
authorizationTip = ""
env_dist = os.environ
print env_dist['LANG']
lang = env_dist['LANG']
if os.path.exists("/etc/.kyinfo"):
    cf.read("/etc/.kyinfo")
    secs = cf.sections()
    opts = cf.options("servicekey")
    str_key = cf.get("servicekey", "key")
    str_version = cf.get("dist", "dist_id")
    str_customer = cf.get("os", "to")
    str_term = cf.get("os", "term")
    if lang.startswith("zh_CN"):
        info_key = "服务序列号：" + str_key
        info_version = "授权版本：" + str_version
        info_customer = "客户单位：" + str_customer
        info_term = "技术服务截止日期：" + str_term
	phoneTips = "温馨提示：如有问题请咨询销售\n" + "咨询电话：400-089-1870"
    else:
        info_key = "Service serial number：" + str_key
        info_version = "Authorized version：" + str_version
        info_customer = "Customer company：" + str_customer
        info_term = "Technical service deadline：" + str_term
	phoneTips = "warm tip: if you have any questions, please consult the sales department \n" + "hotline: 400-089-1870"
    str_time = cf.get("dist", "time")
    tmp = str_time[2:4] + str_time[5:7] + str_time[8:10]

    f = open("/etc/lsb-release", 'r')
    lines = f.readlines()
    f.close()

    for line in lines:
        if line.startswith("DISTRIB_ID"):
            name = line[11:len(line)-1]
        if line.startswith("DISTRIB_KYLIN_RELEASE"):
            kylin_version = line[22:len(line)-1]

    info_build = name + " "+kylin_version
    if os.path.exists("/etc/.kylin-update"):
    	if lang.startswith("zh_CN"):
            update_info = "更新状态：已更新" + info_build + "补丁集\n"
        else:
            update_info = "Update status: updated" + info_build + "patch set \ n"
        info_build = ""
    else:
        info_build = info_build + "\n"
	

    if sys.argv[1]=="ShowTerm":
        info = info_version + "\n" + update_info + info_customer.strip("\n") + "\n" + info_key + "\n" + info_term + "\n" + phoneTips
    else:
        info = info_version + "\n" + update_info + info_customer.strip("\n") + "\n" + info_key + "\n" + phoneTips
else:
    f = open("/etc/lsb-release", 'r')
    lines = f.readlines()
    f.close()

    for line in lines:
        if line.startswith("DISTRIB_ID"):
            name = line[11:len(line)-1]
        if line.startswith("DISTRIB_RELEASE"):
            version = line[16:len(line)-1]
        if lang.startswith("zh_CN"):
	    versionTip = "银河麒麟社区版\n"
	    authorizationTip = "当前版本未授权\n"
	else:
            versionTip = "YHkylin community\n"
            authorizationTip = "current version is not authorized\n"
    if len(lines) >= 6 and lines[5].strip().endswith("community"):
        info = name + " " + version + "\n" + versionTip
    else:
        info = name + " " + version + "\n" + authorizationTip

info = info_build + info
f = open(GLib.get_home_dir() + "/.info", 'w')
f.write(info)
f.close()

