#!/bin/bash
########################################################################
# File Name: env_init.sh
# Author: Baiy
# mail: baiyang0223@163.com
# Created Time: 2021-01-30-10:57:33
# Func: 
#########################################################################

cur=$(pwd)
echo "$cur"


function build_soft_source() {
    cp base-env/sources.list /etc/apt/
    cp base-env/vimrc /home/baiy/.vimrc
    cp base-env/gitconfig /home/baiy/.gitconfig
    cp base-env/gitignore /home/baiy/.gitignore
    cp base-env/taglist.txt /usr/share/vim/vim80/doc/taglist.txt
    cp base-env/taglist.vim /usr/share/vim/vim80/plugin/taglist.vim
    apt-get update
}

function build_base_soft() {
	sudo apt-get install -y net-tools  vim   git gitk tig tree   ssh  nfs-kernel-server \
					dos2unix exuberant-ctags	\
					tftp-hpa tftpd-hpa xinetd  	\
					samba  smbclient  cifs-utils \
					android-tools-fsutils		\
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
					python2.7-dev python-dev bridge-utils \
					uml-utilities net-tools \
					libattr1-dev libcap-dev \
					kpartx libsdl2-dev libsdl1.2-dev \
					debootstrap bsdtar \
					libelf-dev gcc-multilib g++-multilib \
					libcap-ng-dev
}


function build_nfs() {
    apt-get install nfs-kernel-server
    mkdir /nfs_server && sudo chmod 777 /nfs_server -R
    echo "/nfs_server *(rw,sync,no_root_squash,no_subtree_check)" >> /etc/exports
    service rpcbind restart
    service nfs-kernel-server restart
    showmount -e
}

function build_tftp() {
    echo "build tftp"
    cp tftp-conf/tftp   /etc/xinetd.d/tftp
    cp tftp-conf/tftpd-hpa   /etc/default/tftpd-hpa
    cp tftp-conf/xinetd.conf /etc/xinetd.conf 

    sudo service tftpd-hpa restart
    sudo /etc/init.d/xinetd reload
    sudo /etc/init.d/xinetd restart
}

function build_samba() {
    echo "build samba"
    sudo mkdir -p /home/baiy/workspace 
    sudo chown baiy /home/baiy/workspace
    sudo chgrp baiy /home/baiy/workspace
    sudo chmod 777 /home/baiy/workspace
    echo "input smbpasswd:"
    sudo smbpasswd -a baiy
    sudo cp samba-conf/smb.conf  /etc/samba/smb.conf
    sudo service smbd restart
}

function build_jdk() {
    echo "build jdk: not support"
}

function build_conda() {
    echo "build conda: not support"
}

if [ "$1" == "build" ]
then
    echo "Start build develop environment"
    build_soft_source
    build_base_soft
    build_nfs
    build_tftp
    build_samba
    build_conda
fi

