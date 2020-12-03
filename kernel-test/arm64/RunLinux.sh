#!/bin/bash

# Build system.
#
# (C) 2020.10.20 BiscuitOS <buddy.zhang@aliyun.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

ROOT=$(pwd)
QEMU=$(which qemu-system-aarch64)
QEMUT=${QEMU}
ARCH=arm64
BUSYBOX=$ROOT/busybox
OUTPUT=$ROOT
ROOTFS_NAME=ext4
CROSS_COMPILE=aarch64-linux-gnu
FS_TYPE=ext4
FS_TYPE_TOOLS=mkfs.ext4
ROOTFS_SIZE=256
FREEZE_SIZE=512
RAM_SIZE=512
LINUX_DIR=${ROOT}/linux/arch
NET_CFG=${ROOT}/package/networking
CMDLINE="earlycon root=/dev/vda rw rootfstype=${FS_TYPE} console=ttyAMA0 init=/linuxrc loglevel=8"

do_running()
{
	SUPPORT_DEBUG=N
	SUPPORT_NET=N
	[ ${1}X = "debug"X -o ${2}X = "debug"X ] && ARGS+="-s -S "
	if [ ${1}X = "net"X  -o ${2}X = "net"X ]; then
		ARGS+="-net tap "
		ARGS+="-device virtio-net-device,netdev=bsnet0,"
		ARGS+="mac=E0:FE:D0:3C:2E:EE "
		ARGS+="-netdev tap,id=bsnet0,ifname=bsTap0 "
	fi
	

	sudo ${QEMUT} ${ARGS} \
	-M virt \
	-m ${RAM_SIZE}M \
	-cpu cortex-a53 \
	-smp 2 \
	-kernel ${LINUX_DIR}/${ARCH}/boot/Image \
	-device virtio-blk-device,drive=hd1 \
	-drive if=none,file=${ROOT}/Freeze.img,format=raw,id=hd1 \
	-device virtio-blk-device,drive=hd0 \
	-drive if=none,file=${BUSYBOX}/arm64.img,format=raw,id=hd0 \
	-nographic \
	-append "${CMDLINE}"
}




do_mount()
{
	mkdir -p ${ROOT}/FreezeDir
	mountpoint -q ${ROOT}/FreezeDir
	[ $? = 0 ] && echo "FreezeDir has mount!" && exit 0
	## Mount Image
	sudo mount -t ${FS_TYPE} ${ROOT}/Freeze.img ${ROOT}/FreezeDir -o loop >> /dev/null 2>&1
	sudo chown ${USER}.${USER} -R ${ROOT}/FreezeDir
	echo "=============================================="
	echo "FreezeDisk: ${ROOT}/Freeze.img"
	echo "MountDiret: ${ROOT}/FreezeDir"
	echo "=============================================="
}


do_umount()
{
	sync
	mountpoint -q ${ROOT}/FreezeDir
	[ $? = 1 ] && return 0
	sudo umount ${ROOT}/FreezeDir > /dev/null 2>&1
}



do_arm64env()
{

	echo "check qemu-system-aarch64"
	which qemu-system-aarch64
	if [ "$?" != 0  ]
	then
        mkdir qemu-aarch64-bin && cd ../
		mkdir qemu-aarch64-bin  && qdir=$(realpath qemu-aarch64-bin)
		./configure --prefix=${qdir} --target-list=aarch64-softmmu && make && make install
		echo "PATH=${qdir}/bin:\$PATH" >> ~/.bashrc
		cd ${rootdir}

		which qemu-system-aarch64
		if [ "$?" != 0  ]
		then
			echo "qemu install failed"
			return -1
		fi
	fi


	echo "Install corss-compile"
	which aarch64-none-linux-gnu-gcc
	if [ "$?" != 0  ]
	then
		mkdir -p tools && cd tools
		wget wget https://developer.arm.com/-/media/Files/downloads/gnu-a/9.2-2019.12/binrel/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu.tar.xz
		xz -d gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu.tar.xz
		tar -xf gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu.tar
		echo "PATH=${rootdir}/tools/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin/:\$PATH" >> ~/.bashrc
		cd ${rootdir}

		which aarch64-none-linux-gnu-gcc
		if [ "$?" != 0  ]
		then
			echo "qemu install failed"
			return -1
		fi
	fi

	echo "Install ATF"
    if [ ! -e "arm-trusted-firmware" ]
    then
        git clone https://github.com/ARM-software/arm-trusted-firmware.git
	fi

	echo "Build ATF"
	echo "Build uboot"
	echo "Build linux"

	echo "Build busybox"
	cp build_busybox.sh busybox && chmod 777 busybox/build_busybox.sh
	cp make_rootfs.sh busybox  && chmod 777 busybox/make_rootfs.sh
	cp busybox_config busybox/.config

	dd if=/dev/zero of=./Freeze.img bs=1M count=${FREEZE_SIZE}
    mkfs.ext4 -E lazy_itable_init=1,lazy_journal_init=1 -F ./Freeze.img
}


# Lunching Qemu-Linux
case $1 in
	"arm64_env")
		# install aarch-arm64-environment
		do_arm64env
		;;
	"boot")
		dp_running_uboot $1 $2
		;;
	"net")
		# Establish Netwroking
		sudo ${NET_CFG}/bridge.sh
		sudo cp -rf ${NET_CFG}/qemu-ifup /etc
		sudo cp -rf ${NET_CFG}/qemu-ifdown /etc
		do_umount
		do_running net
		;;
	"mount")
		# Mount  Disk
		do_mount
		;;
	"umount")
		# Umount  Disk
		do_umount
		;;
	*)
		# Default Running Linux
		do_umount
		do_running $1 $2
		;;
esac
