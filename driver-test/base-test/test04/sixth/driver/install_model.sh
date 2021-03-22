#!/bin/bash
########################################################################
# File Name: install_model.sh
# Author: Baiy
# mail: baiyang0223@163.com
# Created Time: 2019年09月10日 星期二 15时48分51秒
# Func: 
#########################################################################
module="global_fifo_block"
device="globalfifo"
mode="664"
# invoke insmod with all arguments we got
# and use a pathname, as newer modutils don't look in . by default
/sbin/rmmod $module
/sbin/insmod ./$module.ko $* || exit 1
# remove stale nodes
rm -f /dev/${device}[0-3]
major=$(awk "\\$2==\"$module\" {print \\$1}" /proc/devices)
echo $major
echo "mknod /dev/${device} c $major 0"
