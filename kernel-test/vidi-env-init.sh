#!/bin/bash
########################################################################
# File Name: vidi-env-init.sh
# Author: Baiy
# mail: baiyang0223@163.com
# Created Time: 2020-11-08-07:12:41
# Func: install linux-learn environment initial
#########################################################################

export ROOTDIR=$(pwd)
export LINUX="linux-5.8.14"
export QEMU="qemu-5.1.0"
export UBOOT="u-boot-2020.07"
export BUSYBOX="busybox-1.32.0"
export QEMUDIR=${ROOTDIR}/linux-src/${QEMU}
export LINUXDIR=${ROOTDIR}/linux-src/${LINUX}
export UBOOTDIR=${ROOTDIR}/linux-src/${UBOOT}
export BUSYBOXDIR=${ROOTDIR}/linux-src/${BUSYBOX}
export CPUNUM=$(cat /proc/cpuinfo  | grep processor | wc -l)


do_check_linux()
{
    cd $ROOTDIR
    mkdir -p linux-src && cd linux-src
    if [ ! -e  ${LINUX} ]
    then
        wget https://mirror.bjtu.edu.cn/kernel/linux/kernel/v5.x/${LINUX}.tar.gz && tar -zxf ${LINUX}.tar.gz && rm ${LINUX}.tar.gz
    fi
    cd $ROOTDIR
}
do_check_qemu()
{
    cd $ROOTDIR
    mkdir -p linux-src && cd linux-src
    if [ ! -e  ${QEMU} ]
    then
		wget https://download.qemu.org/${QEMU}.tar.xz && tar xJf ${QEMU}.tar.xz && rm ${QEMU}.tar.xz
    fi
    cd $ROOTDIR
}

do_check_uboot()
{
    cd $ROOTDIR
    mkdir -p linux-src && cd linux-src
    if [ ! -e  ${UBOOT} ]
    then
        wget ftp://ftp.denx.de/pub/u-boot/${UBOOT}.tar.bz2 && tar jxf ${UBOOT}.tar.bz2 && rm ${UBOOT}.tar.bz2
    fi
    cd $ROOTDIR
}
do_check_busybox()
{
    cd $ROOTDIR
    mkdir -p linux-src && cd linux-src
    if [ ! -e  ${BUSYBOX} ]
    then
        wget https://busybox.net/downloads/${BUSYBOX}.tar.bz2 && tar jxf ${BUSYBOX}.tar.bz2 && rm ${BUSYBOX}.tar.bz2
    fi
    cd $ROOTDIR
}

do_arm64_env()
{
    cd arm64
    arm64dir=$(pwd)

	echo "check cross-compile-aarch64"
	export "PATH=${arm64dir}/tools/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin/:$PATH"
	which aarch64-none-linux-gnu-gcc
	if [ "$?" != 0  ]
	then
		mkdir -p tools && cd tools
		wget wget https://developer.arm.com/-/media/Files/downloads/gnu-a/9.2-2019.12/binrel/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu.tar.xz
		xz -d gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu.tar.xz
		tar -xf gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu.tar
		cd ${rootdir}

		which aarch64-none-linux-gnu-gcc
		if [ "$?" != 0  ]
		then
			echo "cross-compile install failed"
			return -1
		fi
	fi

	echo "check qemu-system-aarch64"
    export "PATH=${arm64dir}/qemu-aarch64-bin/bin:$PATH" 
	which qemu-system-aarch64
	if [ "$?" != 0  ]
	then
        mkdir -p qemu-aarch64-bin  && qdir=$(realpath qemu-aarch64-bin)
        rm -rf ${qdir}
        cd ${QEMUDIR} && ./configure --prefix=${qdir} --target-list=aarch64-softmmu && make && make install
        cd $qdir
		which qemu-system-aarch64
		if [ "$?" != 0  ]
		then
			echo "qemu install failed"
			return -1
		fi
	fi

	echo "check uboot-system-aarch64"
    if [ ! -e "uboot" ]
    then
        cp -r ${UBOOTDIR}  ./ && ln -s ${arm64dir}/${UBOOT} ${arm64dir}/uboot
		# cp build_uboot.sh uboot
        # cp build_uboot.sh uboot  && cd uboot && ./build_uboot.sh init
        cd ${arm64dir}
    fi

    echo "check busybox-system-aarch64"
    if [ ! -e "busybox" ]
    then
        cp -r ${BUSYBOXDIR}  ./ && ln -s ${arm64dir}/${BUSYBOX} ${arm64dir}/busybox
		# cp build_busybox.sh busybox
		# cp make_rootfs.sh busybox
        # cp build_busybox.sh busybox  && cd busybox && ./build_busybox.sh init
        cd ${arm64dir}
    fi

    echo "check linux-system-aarch64"
    if [ ! -e "linux" ]
    then
        cp -r ${LINUXDIR}  ./ && ln -s ${arm64dir}/${LINUX} ${arm64dir}/linux
		# cp build_linux.sh linux
        # cp build_linux.sh linux  && cd linux && ./build_linux.sh init
        cd ${arm64dir}
    fi
	cd  ${ROOTDIR}
}

############################################################
# install environment
# baiy@baiy-ThinkPad-E470c:kernel-test$ ls linux-src/
# busybox-1.32.0  linux-5.8.14  qemu-5.1.0  u-boot-2020.07
###########################################################
do_check_linux
do_check_qemu
do_check_uboot
do_check_busybox


case $1 in
    "arm64")
        do_arm64_env
		cd arm64 && ./RunLinux.sh arm64_env
        ;;
    "arm32")
        ;;
    "x86-64")
        ;;
    "risc-v")
        ;;
esac


