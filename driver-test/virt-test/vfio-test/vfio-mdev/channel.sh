#!/bin/bash
########################################################################
# File Name: channel.sh
# Author: Baiy
# mail: baiyang0223@163.com
# Created Time: 2021-01-20-10:12:59
# Func: 
#########################################################################


if [ "$1" == "create" ]
then
    echo "Start create 16 channel"
    for ((i=0; i<3; ++i))
    do
        a=$(printf "%02x" $i)
        echo "83b8f4f2-509f-382f-3c1e-e6bfe0fa10$a" > /sys/devices/virtual/mtty/mtty/mdev_supported_types/mtty-1/create
    done
else
    echo "Start delete 16 channel"
    for ((i=0;i<3;++i))
    do
        a=$(printf "%02x" $i)
        echo 1 > "/sys/bus/mdev/devices/83b8f4f2-509f-382f-3c1e-e6bfe0fa10$a/remove"
    done
fi

