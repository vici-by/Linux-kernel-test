#!/bin/bash
########################################################################
# File Name: install.sh
# Author: Baiy
# mail: baiyang0223@163.com
# Created Time: 2021-01-20-13:55:43
# Func: 
#########################################################################
./channel.sh
rmmod  vfio_mtty.ko 
rmmod  vfio_test.ko 

