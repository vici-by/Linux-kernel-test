#!/bin/bash
########################################################################
# File Name: build_linux.sh
# Author: Baiy
# mail: baiyang0223@163.com
# Created Time: 2020-11-08-11:44:16
# Func: 
#########################################################################
if [ "$1" == "init" ]
then
	make distclean
	make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- defconfig
	#make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- menuconfig
fi
make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu-  -j4
