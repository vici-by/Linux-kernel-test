#!/bin/bash
########################################################################
# File Name: make_rootfs.sh
# Author: Baiy
# mail: baiyang0223@163.com
# Created Time: 2020-10-20-16:10:28
# Func: 
#########################################################################
base=`pwd`
tmpfs=/_tmpfs

sudo rm -rf rootfs
sudo rm -rf ${tmpfs}
sudo rm -f arm64.img
sudo mkdir rootfs
sudo cp _install/*  rootfs/ -raf

#sudo mkdir -p rootfs/{lib,proc,sys,tmp,root,var,mnt}
cd rootfs && sudo mkdir -p lib proc sys tmp root var mnt && cd ${base}

# *****根据自己的实际情况, 找到并 拷贝 arm-gcc 中的 libc中的所有.so 库******
a=$(dirname `echo $(whereis aarch64-none-linux-gnu-gcc) | cut -d ":" -f 2`)
sudo cp -arf ${a}/../aarch64-none-linux-gnu/libc/lib64/*so*  rootfs/lib
sudo cp -arf rootfs/lib/ld-2.30.so  rootfs/lib/ld-linux-aarch64.so.1

echo "1111111111111"
# sudo cp app rootfs
sudo cp examples/bootfloppy/etc rootfs/ -arf
sudo sed -r  "/askfirst/ s/.*/::respawn:-\/bin\/sh/" rootfs/etc/inittab -i
sudo mkdir -p rootfs/dev/
sudo mknod rootfs/dev/tty1 c 4 1
sudo mknod rootfs/dev/tty2 c 4 2
sudo mknod rootfs/dev/tty3 c 4 3
sudo mknod rootfs/dev/tty4 c 4 4
sudo mknod rootfs/dev/console c 5 1
sudo mknod rootfs/dev/null c 1 3

sudo dd if=/dev/zero of=arm64.img bs=1M count=150
sudo mkfs.ext4 arm64.img
sudo mkdir -p ${tmpfs}
sudo chmod 777 ${tmpfs}
sudo mount -t ext4 arm64.img ${tmpfs}/ -o loop
sudo cp -r rootfs/*  ${tmpfs}/
sudo umount ${tmpfs}
