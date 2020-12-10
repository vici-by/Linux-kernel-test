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
export EDK2="edk2"
export OPENBIOS="openbios"
export EDK2DIR=${ROOTDIR}/linux-src/${EDK2}
export OPENBIOSDIR=${ROOTDIR}/linux-src/${OPENBIOS}
export QEMUDIR=${ROOTDIR}/linux-src/${QEMU}
export LINUXDIR=${ROOTDIR}/linux-src/${LINUX}
export UBOOTDIR=${ROOTDIR}/linux-src/${UBOOT}
export BUSYBOXDIR=${ROOTDIR}/linux-src/${BUSYBOX}
export CPUNUM=$(cat /proc/cpuinfo  | grep processor | wc -l)


do_check_env()
{
    sudo apt-get install -y net-tools  vim   git  tree   ssh  nfs-kernel-server \
              dos2unix exuberant-ctags  \
              tftp-hpa tftpd-hpa xinetd   \
              samba  smbclient  cifs-utils \
              android-tools-fsutils   \
              autoconf automake build-essential \
              libass-dev libfreetype6-dev libsdl2-dev \
              libtheora-dev libtool libva-dev libvdpau-dev libvorbis-dev \
              libxcb1-dev libxcb-shm0-dev libxcb-xfixes0-dev pkg-config  \
              texinfo wget zlib1g-dev \
              lib32z1 libc6-i386 lib32ncurses5 \
              flex bison \
              qemu gcc make gdb git figlet \
              libncurses5-dev iasl wget \
              device-tree-compiler \
              flex bison libssl-dev libglib2.0-dev \
              libfdt-dev libpixman-1-dev \
              python pkg-config u-boot-tools intltool xsltproc \
              gperf libglib2.0-dev libgirepository1.0-dev \
              gobject-introspection \
              python2.7-dev python-dev bridge-utils\
              uml-utilities net-tools \
              libattr1-dev libcap-dev \
              kpartx libsdl2-dev libsdl1.2-dev \
              debootstrap bsdtar \
              libelf-dev gcc-multilib g++-multilib \
              libcap-ng-dev

    sudo apt-get install -y build-essential uuid-dev nasm
    sudo apt-get install -y qemu gcc make gdb git figlet
    sudo apt-get install -y libncurses5-dev iasl wget
    sudo apt-get install -y device-tree-compiler
    sudo apt-get install -y flex bison libssl-dev libglib2.0-dev
    sudo apt-get install -y libfdt-dev libpixman-1-dev
    sudo apt-get install -y python pkg-config u-boot-tools intltool xsltproc
    sudo apt-get install -y gperf libglib2.0-dev libgirepository1.0-dev
    sudo apt-get install -y gobject-introspection
    sudo apt-get install -y python2.7-dev python-dev bridge-utils
    sudo apt-get install -y uml-utilities net-tools
    sudo apt-get install -y libattr1-dev libcap-dev
    sudo apt-get install -y kpartx libsdl2-dev libsdl1.2-dev
    sudo apt-get install -y debootstrap bsdtar
    sudo apt-get install -y libelf-dev gcc-multilib g++-multilib
    sudo apt-get install -y libcap-ng-dev libaio-dev
    sudo apt-get install -y libcap-dev libattr1-dev figlet libssl-dev
}
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
do_check_bios()
{
    cd $ROOTDIR
    mkdir -p linux-src && cd linux-src
    if [ ! -e  ${EDK2} ]
    then
        echo "aaa"
        git clone https://github.com/tianocore/edk2.git
        cd ${EDK2}
        git checkout  --track origin/UDK2018
        cd ../
    fi

    if [ ! -e  ${OPENBIOS} ]
    then
        git clone https://github.com/openbios/openbios.git
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
        else
            echo "PATH=${arm64dir}/tools/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin/:$PATH" >> ~/.bashrc
		fi
	fi

	echo "check qemu-system-aarch64"
    export "PATH=${arm64dir}/qemu-aarch64-bin/bin:$PATH" 
	which qemu-system-aarch64
	if [ "$?" != 0  ]
	then
        mkdir -p qemu-aarch64-bin  && qdir=$(realpath qemu-aarch64-bin)
        rm -rf ${qdir}
        cd ${QEMUDIR} && ./configure --prefix=${qdir} --target-list=aarch64-softmmu && make clean && make && make install
        cd $qdir
		which qemu-system-aarch64
		if [ "$?" != 0  ]
		then
			echo "qemu install failed"
			return -1
        else
            echo "PATH=${arm64dir}/qemu-aarch64-bin/bin:$PATH" >> ~/.bashrc
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

do_x8664_env()
{
    cd x86_64
    x8664dir=$(pwd)

	echo "check gcc"
	which gcc
	if [ "$?" != 0  ]
	then
        echo "gcc not support ????!!! why???"
        return -1
	fi

	echo "check qemu-system-aarch64"
	which qemu-system-x86_64
	if [ "$?" != 0  ]
	then
        rm -rf ${qdir}
        cd ${QEMUDIR} && ./configure --prefix=/opt/x86_64/qemu-x86-bin --target-list=x86_64-softmmu --gdb=/usr/bin/gdb \
	        --enable-linux-aio --enable-debug --enable-debug-info  && make clean && make && make install
        cd $qdir
		which qemu-system-aarch64
		if [ "$?" != 0  ]
		then
			echo "qemu install failed"
			return -1
        fi
	fi

	echo "check uefi"
    if [ ! -e "edk2" ]
    then
        cp -r ${EDK2DIR}  ./
        cd ${x8664dir}
    fi
	echo "check openbios"
    if [ ! -e "openbios" ]
    then
        cp -r ${OPENBIOSDIR}  ./
        cd ${x8664dir}
    fi

    echo "check busybox-system-aarch64"
    if [ ! -e "busybox" ]
    then
        cp -r ${BUSYBOXDIR}  ./ && ln -s ${x8664dir}/${BUSYBOX} ${x8664dir}/busybox
		# cp build_busybox.sh busybox
		# cp make_rootfs.sh busybox
        # cp build_busybox.sh busybox  && cd busybox && ./build_busybox.sh init
        cd ${x8664dir}
    fi

    echo "check linux-system-x86"
    if [ ! -e "linux" ]
    then
        cp -r ${LINUXDIR}  ./ && ln -s ${x8664dir}/${LINUX} ${x8664dir}/linux
		# cp build_linux.sh linux
        # cp build_linux.sh linux  && cd linux && ./build_linux.sh init
        cd ${x8664dir}
    fi
	cd  ${ROOTDIR}
}
do_arm32_env()
{
    cd arm32
    arm32dir=$(pwd)

	echo "check cross-compile-arm32"
	export "PATH=${arm32dir}/tools/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin/:$PATH"
	which arm-none-linux-gnueabihf-gcc
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
        else
            echo "PATH=${arm64dir}/tools/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin/:$PATH" >> ~/.bashrc
		fi
	fi

	echo "check qemu-system-aarch64"
    export "PATH=${arm64dir}/qemu-aarch64-bin/bin:$PATH"
	which qemu-system-aarch64
	if [ "$?" != 0  ]
	then
        mkdir -p qemu-aarch64-bin  && qdir=$(realpath qemu-aarch64-bin)
        rm -rf ${qdir}
        cd ${QEMUDIR} && ./configure --prefix=${qdir} --target-list=aarch64-softmmu && make clean && make && make install
        cd $qdir
		which qemu-system-aarch64
		if [ "$?" != 0  ]
		then
			echo "qemu install failed"
			return -1
        else
            echo "PATH=${arm64dir}/qemu-aarch64-bin/bin:$PATH" >> ~/.bashrc
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
#do_check_env
do_check_linux
do_check_qemu
do_check_uboot
do_check_bios
do_check_busybox


case $1 in
    "arm64")
        do_arm64_env
		cd arm64 && ./RunLinux.sh arm64_env
        ;;
    "arm32")
        do_arm32_env
		cd arm32 && ./RunLinux.sh arm32_env
        ;;
    "x86-64")
        do_x8664_env
		#cd x86 && ./RunLinux.sh x86_env
        ;;
esac


