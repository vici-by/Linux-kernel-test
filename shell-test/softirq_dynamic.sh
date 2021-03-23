#!/bin/bash
########################################################################
# File Name: loop.sh
# Author: Baiy
# mail: baiyang0223@163.com
# Created Time: 2021-03-22-16:45:19
# Func: 
#########################################################################


function softirqs(){
    c_path="/proc/softirqs"  
    len=$(grep "CPU0" $c_path | wc -w) 
    num=8
    loop=$((len/num))
    i=0
    j=1

    while [ $i -lt $loop ]
    do
        grep "CPU0" $c_path | awk -v k=$j        '{printf "%-10s","Title"; for(p=0;p<8;p++){printf "%-10s", $k; k++}; printf "\r\n" }'
        grep ":"    $c_path | awk -v k=$(($j+1)) '{u=k; printf "%-10s",$1; for(p=0;p<8;p++){printf "%-10s", $u; u++}; printf "\r\n" }'
        printf "\r\n"
        i=$(($i+1))
        j=$((1+$i*$num))
    done
}

trap "clear;exit" 2
# 翻一个新屏幕
clear
# 每秒刷新输出屏幕的端口统计结果
while [ true ]
do
    # 需要执行的功能命令，各写各的业务
	softirqs
    # echo输出特殊处理，见引用2
    #echo -ne "$report"
    # 使用ASCI码控制光标定位回到第一行第一列，见引用1
    echo -ne "\033[1;1H"
    # 进程睡眠1秒
    sleep 1
done
