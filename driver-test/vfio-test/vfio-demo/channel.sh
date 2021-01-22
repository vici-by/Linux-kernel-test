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
        if [ "$i" -lt 10 ]
        then
            echo "83b8f4f2-509f-382f-3c1e-e6bfe0fa100$i" > /sys/devices/virtual/mtty/mtty/mdev_supported_types/mtty-1/create
        else
            echo "83b8f4f2-509f-382f-3c1e-e6bfe0fa10$i" > /sys/devices/virtual/mtty/mtty/mdev_supported_types/mtty-1/create
        fi
    done
else
    echo "Start delete 16 channel"
    for ((i=0;i<3;++i))
    do
        if [ "$i" -lt 10 ]
        then
            echo 1 > "/sys/bus/mdev/devices/83b8f4f2-509f-382f-3c1e-e6bfe0fa100$i/remove"
        else
            echo 1 > "/sys/bus/mdev/devices/83b8f4f2-509f-382f-3c1e-e6bfe0fa10$i/remove"
        fi
    done
fi

