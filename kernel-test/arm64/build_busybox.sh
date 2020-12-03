#!/bin/bash
########################################################################
# File Name: build_busybox.sh
# Author: Baiy
# mail: baiyang0223@163.com
# Created Time: 2020-11-08-12:12:41
# Func: 
#########################################################################
if [ "$1" == "init" ]
then
	make distclean
	#make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- menuconfig
fi
make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu-
make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- install 
sudo ./make_rootfs.sh
