/*************************************************************************
    > File Name: s_util.h
    > Author: baiy
    > Mail: baiyang0223@163.com
    > Created Time: 2020-12-11-17:46:07
    > Func: 通用Linux常用接口
 ************************************************************************/

#ifndef __S_UTIL_H
#define __S_UTIL_H



/*
 * Some bits are stolen from perf tool :)
 */

#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/types.h>
#include <linux/types.h>
#include <ctype.h>
#include <linux/magic.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/statfs.h>

#include "s_compiler.h"


#ifdef __GNUC__
#define NORETURN __attribute__((__noreturn__))
#else
#define NORETURN
#ifndef __attribute__
#define __attribute__(x)
#endif
#endif


extern bool do_debug_print;

#define PROT_RW (PROT_READ|PROT_WRITE)
#define MAP_ANON_NORESERVE (MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE)

extern void print_die(const char *err, ...) NORETURN __attribute__((format (printf, 1, 2)));
extern void die_perror(const char *s) NORETURN;
extern int print_err(const char *err, ...) __attribute__((format (printf, 1, 2)));
extern void print_warning(const char *err, ...) __attribute__((format (printf, 1, 2)));
extern void print_info(const char *err, ...) __attribute__((format (printf, 1, 2)));

#define pr_debug(fmt, ...)						\
	do {								\
		if (do_debug_print)					\
			print_info("(%s) %s:%d: " fmt, __FILE__,		\
				__func__, __LINE__, ##__VA_ARGS__);	\
	} while (0)
#define pr_info(fmt, ...)						\
        do {                                \
            print_info("(%s) %s:%d: " fmt, __FILE__,       \
                __func__, __LINE__, ##__VA_ARGS__); \
        } while (0)
#define pr_err(fmt, ...)						\
        do {                                \
            print_err("(%s) %s:%d: " fmt, __FILE__,       \
                __func__, __LINE__, ##__VA_ARGS__); \
        } while (0)
#define die(fmt, ...)						\
        do {                                \
            print_die("(%s) %s:%d: " fmt, __FILE__,       \
                __func__, __LINE__, ##__VA_ARGS__); \
        } while (0)




#define BUILD_BUG_ON(condition)	((void)sizeof(char[1 - 2*!!(condition)]))

#ifndef BUG_ON_HANDLER
# define BUG_ON_HANDLER(condition)					\
	do {								\
		if ((condition)) {					\
			pr_err("BUG at %s:%d", __FILE__, __LINE__);	\
			raise(SIGABRT);					\
		}							\
	} while (0)
#endif

#define BUG_ON(condition)	BUG_ON_HANDLER((condition))

#define DIE_IF(cnd)						\
do {								\
	if (cnd)						\
	die(" at (" __FILE__ ":" __stringify(__LINE__) "): "	\
		__stringify(cnd) "\n");				\
} while (0)

#define WARN_ON(condition) ({					\
	int __ret_warn_on = !!(condition);			\
	if (__ret_warn_on)					\
		pr_warning("(%s) %s:%d: failed condition: %s",	\
				__FILE__, __func__, __LINE__,	\
				__stringify(condition));	\
	__ret_warn_on;						\
})

#define MSECS_TO_USECS(s) ((s) * 1000)

/* Millisecond sleep */
static inline void msleep(unsigned int msecs)
{
	usleep(MSECS_TO_USECS(msecs));
}










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



#define MB_SHIFT (20)
#define KB_SHIFT (10)
#define GB_SHIFT (30)





/**********************************自定义代码*******************************************/
struct host_env{
    unsigned int nr_online_cpus;
    unsigned long page_size;
	unsigned long nr_pages;
    unsigned long mem_size;
};
extern void host_env(struct host_env * henv);

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
