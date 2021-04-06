#!/bin/bash
########################################################################
# File Name: build.sh
# Author: Baiy
# mail: baiyang0223@163.com
# Created Time: 2020-10-20-16:27:11
# Func: 
#########################################################################
sudo rmmod  driver_pcie_test ; make && sudo insmod  driver_pcie_test.ko  && sudo dmesg -c
rm ./app_pcie_test && gcc ./app_pcie_test.c  -o ./app_pcie_test
