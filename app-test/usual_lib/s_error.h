/*************************************************************************
    > File Name: s_error.h
    > Author: baiy
    > Mail: baiyang0223@163.com 
    > Created Time: 2020-12-30-11:42:12
    > Func: 
 ************************************************************************/
#ifndef S_ERROR_H
#define S_ERROR_H
typedef enum S_ERROR
{
#define S_ERRNO(x) x,
    #include "s_errno.h"
#undef  S_ERRNO
}S_ERROR;

#endif//__S_ERRNO_H
