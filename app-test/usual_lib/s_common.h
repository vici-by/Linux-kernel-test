/*************************************************************************
    > File Name: s_common.h
    > Author: baiy
    > Mail: baiyang0223@163.com 
    > Created Time: 2020-12-11-17:46:07
    > Func: 通用Linux常用接口
 ************************************************************************/

#ifndef __S_COMMON_H
#define __S_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

// 注意：size 一定要是2^n 才好使
#define alignment_down(a, size) (a & (~(size-1)) )
#define alignment_up(a, size)   ((a+size-1) & (~ (size-1)))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]) )



static inline int ctoi(const char * s)
{
    int i;
    int n = 0;

    if (s[0] == '0' && (s[1]=='x' || s[1]=='X')) {
        i = 2;
    } else {
        return atoi(s);
    }
    for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') \
            || (s[i] >='A' && s[i] <= 'Z');++i)
    {
        if (tolower(s[i]) > '9') {
            n = 16 * n + (10 + tolower(s[i]) - 'a');
        } else {
            n = 16 * n + (tolower(s[i]) - '0');
        }
    }
    return n;
}

#endif
