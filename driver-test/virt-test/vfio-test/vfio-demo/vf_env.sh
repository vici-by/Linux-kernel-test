#!/bin/bash
########################################################################
# File Name: build.sh
# Author: Baiy
# mail: baiyang0223@163.com
# Created Time: 2021-01-20-10:58:54
# Func: 
#########################################################################
gcc vfio_guest_app.c -o vfio_guest_app
scp vfio_guest_app baiy@192.168.122.45:/tmp/
scp vf_test.sh baiy@192.168.122.45:/tmp/
ssh baiy@192.168.122.45
