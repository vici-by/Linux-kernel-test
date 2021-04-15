/*************************************************************************
    > File Name: s_compiler.h
    > Author: baiy
    > Mail: baiyang0223@163.com
    > Created Time: 2020-12-11-17:46:07
    > Func: 编译器配置选项
 ************************************************************************/

#ifndef S_COMPILER_H
#define S_COMPILER_H

#ifndef __WORDSIZE
#define __WORDSIZE (__SIZEOF_LONG__ * 8)
#endif


// __always_inline强制内联，修饰的函数在被调用的时候不会被编译成函数调用,而是直接扩展到调用函数体内.
#ifndef __always_inline
# define __always_inline	inline __attribute__((always_inline))
#endif


/*
 * 编译器分支预测优化（将可能的分支提前），likely表示更大可能成立，unlikely表示更大可能不成立
 */
#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)   // x==1 很有可能发生
#endif
#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0) // x==0基本不发生
#endif




/*
 * 此函数为GNU扩展，用来判断两个类型是否相同，如果type_a与 type_b相同的话，就会返回1，否则的话，返回0
 */
#ifndef __same_type
# define __same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
#endif
// 如果e为True，强制编译器出错。 但这个表达式也会返回产生一个结果（值为0饼键入 int），因此可用在表达式中
#define BUILD_BUG_ON_ZERO(e) ((int)(sizeof(struct { int:(-!!(e)); })))
#define __must_be_array(a)	BUILD_BUG_ON_ZERO(__same_type((a), &(a)[0])) // 区分指针和数组



#ifndef __must_check
#define __must_check
#endif




#endif
