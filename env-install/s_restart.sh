#!/bin/bash

if [ "$1" == "smb" ]
then
	sudo service smbd restart
elif [ "$1" == "tftp" ]
then
	sudo service tftpd-hpa restart
	sudo /etc/init.d/xinetd reload
	sudo /etc/init.d/xinetd restart
elif [ "$1" == "nfs" ]
then
	sudo service rpcbind restart
	sudo service nfs-kernel-server restart
	sudo showmount -e
fi
