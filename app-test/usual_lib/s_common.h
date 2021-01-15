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
#include <sys/types.h>
#include <errno.h>


#include "s_compiler.h"

/**********************************LINUX内核接口（kernel.h）*******************************************/
// #include <linux/kernel.h>
// Linux对齐操作
#define __ALIGN_KERNEL_MASK(x, mask)	(((x) + (mask)) & ~(mask))
#define __ALIGN_KERNEL(x, a)		__ALIGN_KERNEL_MASK(x, (typeof(x))(a) - 1)
#define __KERNEL_DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define ALIGN(x, a)		__ALIGN_KERNEL((x), (a))
#define ALIGN_DOWN(x, a)	__ALIGN_KERNEL((x) - ((a) - 1), (a))
#define __ALIGN_MASK(x, mask)	__ALIGN_KERNEL_MASK((x), (mask))
#define PTR_ALIGN(p, a)		((typeof(p))ALIGN((unsigned long)(p), (a)))
#define PTR_ALIGN_DOWN(p, a)	((typeof(p))ALIGN_DOWN((unsigned long)(p), (a)))
#define IS_ALIGNED(x, a)		(((x) & ((typeof(x))(a) - 1)) == 0)

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y) ((x) & ~__round_mask(x, y))



#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))


// 注意：size 一定要是2^n 才好使
#define alignment_down(a, size) (a & (~(size-1)) )
#define alignment_up(a, size)   ((a+size-1) & (~ (size-1)))


#ifndef max
#define max(x, y) ({				\
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	(void) (&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })
#endif

#ifndef min
#define min(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })
#endif



static inline unsigned long  stoi(const char * s)
{
    unsigned long tmp_data;

    if (s[0] == '0' && (s[1]=='x' || s[1]=='X')) {
        return strtoul(s+2, NULL, 16);
    } else {
        return strtoul(s,NULL, 10);
    }
}

#endif
