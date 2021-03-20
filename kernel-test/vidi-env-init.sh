#!/bin/bash
########################################################################
# File Name: vidi-env-init.sh
# Author: Baiy
# mail: baiyang0223@163.com
# Created Time: 2020-11-08-07:12:41
# Func: install linux-learn environment initial
#########################################################################

export ROOTDIR=$(pwd)
export LINUX="linux-5.8"
export QEMU="qemu-5.1.0"
export UBOOT="u-boot-2020.07"
export BUSYBOX="busybox-1.32"
export CPUNUM=$(cat /proc/cpuinfo  | grep processor | wc -l)
export UBOOTDIR=${ROOTDIR}/linux-src/u-boot
export BUSYBOXDIR=${ROOTDIR}/linux-src/busybox
export LINUXDIR=${ROOTDIR}/linux-src/linux


do_check_linux()
{
    cd $ROOTDIR
    mkdir -p linux-src && cd linux-src
    if [ ! -e  linux ]
    then
        # wget http://ftp.sjtu.edu.cn/sites/ftp.kernel.org/pub/linux/kernel/v5.x/${LINUX}.tar.gz && tar -zxf ${LINUX}.tar.gz && rm ${LINUX}.tar.gz
        git clone https://mirrors.tuna.tsinghua.edu.cn/git/linux.git
    fi
    cd $ROOTDIR
}
do_check_qemu()
{
    cd $ROOTDIR
    mkdir -p linux-src && cd linux-src
    if [ ! -e  qemu ]
    then
		# wget https://download.qemu.org/${QEMU}.tar.xz && tar xJf ${QEMU}.tar.xz && rm ${QEMU}.tar.xz
        git clone https://mirrors.tuna.tsinghua.edu.cn/git/qemu.git 
    fi
    cd $ROOTDIR
}

do_check_uboot()
{
    cd $ROOTDIR
    mkdir -p linux-src && cd linux-src
    if [ ! -e  u-boot ]
    then
        # wget ftp://ftp.denx.de/pub/u-boot/${UBOOT}.tar.bz2 && tar jxf ${UBOOT}.tar.bz2 && rm ${UBOOT}.tar.bz2
        # git clone git@github.com:u-boot/u-boot.git
        git clone git@gitee.com:mirrors/u-boot.git
    fi
    cd $ROOTDIR
}
do_check_busybox()
{
    cd $ROOTDIR
    mkdir -p linux-src && cd linux-src
    if [ ! -e  busybox ]
    then
        # wget https://busybox.net/downloads/${BUSYBOX}.tar.bz2 && tar jxf ${BUSYBOX}.tar.bz2 && rm ${BUSYBOX}.tar.bz2
        git clone git@github.com:mirror/busybox.git
    fi
    cd $ROOTDIR
}

do_x86_env()
{
    cd x86_64
    x8664dir=$(pwd)

	echo "check gcc"
	which gcc
	if [ "$?" != 0  ]
	then
        echo "Not Support gcc"
        return -1
	fi

	echo "check qemu-system-x86_64"
	which qemu-system-x86_64
	if [ "$?" != 0  ]
	then
        # retry 
        export "PATH=${x8664dir}/qemu-system-x86_64/bin:$PATH" 
        which qemu-system-x86_64
        if [ "$?" != 0  ]
        then

            mkdir -p qemu-system-x86_64  && qdir=$(realpath qemu-system-x86_64)
            rm -rf ${qdir}
            cd ${QEMUDIR} && ./configure --prefix=${qdir} --target-list=aarch64-softmmu && make && make install
			make clean
			./configure  --prefix= ${qdir}\
				--enable-kvm \
				--enable-trace-backends=log \
				--disable-strip  \
				--enable-debug   \
				--enable-debug-info  \
				--enable-libusb      \
				--enable-usb-redir   \
				--enable-curl  	\
				--enable-sdl 	\
				--enable-opengl \
				--enable-virglrenderer \
				--enable-system \
				--enable-modules \
				--audio-drv-list=pa \
				--enable-vhost-net  \
				--enable-vnc        \
				--target-list=x86_64-softmmu
	`		make && make install

            cd ${x8664dir}
            which qemu-system-x86_64
            if [ "$?" != 0  ]
            then
                echo "qemu install failed"
                return -1
            fi
        fi
	fi


    echo "check busybox-system-x8664"
    if [ ! -e "busybox" ]
    then
        echo "cp busybox"
        version1=$(echo "$BUSYBOX" | cut -d "-" -f 2 | cut -d "." -f 1)
        version2=$(echo "$BUSYBOX" | cut -d "-" -f 2 | cut -d "." -f 2)
        version=${version1}_${version2}_stable
        cp -r ${BUSYBOXDIR}  ./${BUSYBOX} && cd ${BUSYBOX} && git checkout --track origin/${version}  && cd ../ && ln -s ${x8664dir}/${BUSYBOX} ${x8664dir}/busybox
		cd ${x8664dir}
    fi

    echo "check linux-system-x8664"
    if [ ! -e "linux" ]
    then
        echo "cp linux"
        version=$(echo "$LINUX" | cut -d "-" -f 2)
        version=v${version}
        cp -r ${LINUXDIR}  ./${LINUX} && cd ${LINUXDIR} &&  git checkout  ${version} && git checkout  -b ${version} && cd ../ && ln -s ${x8664dir}/${LINUX} ${x8664dir}/linux
		# cp build_linux.sh linux
        # cp build_linux.sh linux  && cd linux && ./build_linux.sh init
		cd ${x8664dir}
    fi

	cd  ${ROOTDIR}
}



do_arm64_env()
{
    cd arm64
    arm64dir=$(pwd)

	echo "check cross-compile-aarch64"
	which aarch64-none-linux-gnu-gcc
	if [ "$?" != 0  ]
	then
        # retry
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
	fi

	echo "check qemu-system-aarch64"
	which qemu-system-aarch64
	if [ "$?" != 0  ]
	then
        # retry 
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
	fi

	echo "check uboot-system-aarch64"
    if [ ! -e "uboot" ]
    then
        echo "cp uboot"
        version=$(echo "$UBOOT" | cut -d "-" -f 3)
        version=v${version}
        cp -r ${UBOOTDIR}  ./${UBOOT}  && cd ${UBOOT} && git checkout  ${version} && git checkout  -b ${version} && cd ../ && ln -s ${arm64dir}/${UBOOT} ${arm64dir}/uboot
		# cp build_uboot.sh uboot
        # cp build_uboot.sh uboot  && cd uboot && ./build_uboot.sh init
        cd ${arm64dir}
    fi

    echo "check busybox-system-aarch64"
    if [ ! -e "busybox" ]
    then
        echo "cp busybox"
        version1=$(echo "$BUSYBOX" | cut -d "-" -f 2 | cut -d "." -f 1)
        version2=$(echo "$BUSYBOX" | cut -d "-" -f 2 | cut -d "." -f 2)
        version=${version1}_${version2}_stable
        cp -r ${BUSYBOXDIR}  ./${BUSYBOX} && cd ${BUSYBOX} && git checkout --track origin/${version}  && cd ../ && ln -s ${arm64dir}/${BUSYBOX} ${arm64dir}/busybox
		# cp build_busybox.sh busybox
		# cp make_rootfs.sh busybox
        # cp build_busybox.sh busybox  && cd busybox && ./build_busybox.sh init
        cd ${arm64dir}
    fi

    echo "check linux-system-aarch64"
    if [ ! -e "linux" ]
    then
        echo "cp linux"
        version=$(echo "$LINUX" | cut -d "-" -f 2)
        version=v${version}
        cp -r ${LINUXDIR}  ./${LINUX} && cd ${LINUXDIR} &&  git checkout  ${version} && git checkout  -b ${version} && cd ../ && ln -s ${arm64dir}/${LINUX} ${arm64dir}/linux
		# cp build_linux.sh linux
        # cp build_linux.sh linux  && cd linux && ./build_linux.sh init
        cd ${arm64dir}
    fi

    if [ ! -e "arm-trusted-firmware" ]
    then
        echo "download atf"
        # git clone https://github.com/ARM-software/arm-trusted-firmware.git
	fi

	cd  ${ROOTDIR}
}


# Download git src
do_check_linux
do_check_qemu
do_check_uboot
do_check_busybox

echo -e "Source Code Download OK\n\n"


case $1 in
    "arm64")
        do_arm64_env
		# cd arm64 && ./RunLinux.sh arm64_env
        ;;
    "x86")
        do_x86_env
		cd x86 && ./RunLinux.sh x86_env
        ;;
esac


