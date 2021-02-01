#!/bin/bash
########################################################################
# File Name: install.sh
# Author: Baiy
# mail: baiyang0223@163.com
# Created Time: 2021-01-20-13:55:43
# Func: 
#########################################################################
make clean && make
#insmod  vfio_test.ko 
insmod  vfio_mtty.ko 
./channel.sh create

