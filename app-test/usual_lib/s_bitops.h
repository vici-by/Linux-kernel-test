/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_BITOPS_H
#define _LINUX_BITOPS_H



#include "s_types.h"


#define BIT(x)	(1 << (x))



/**
 * rol64 - rotate a 64-bit value left
 * @word: value to rotate
 * @shift: bits to roll
 */
static inline __u64 rol64(__u64 word, unsigned int shift)
{
	return (word << shift) | (word >> (64 - shift));
}

/**
 * ror64 - rotate a 64-bit value right
 * @word: value to rotate
 * @shift: bits to roll
 */
static inline __u64 ror64(__u64 word, unsigned int shift)
{
	return (word >> shift) | (word << (64 - shift));
}

/**
 * rol32 - rotate a 32-bit value left
 * @word: value to rotate
 * @shift: bits to roll
 */
static inline __u32 rol32(__u32 word, unsigned int shift)
{
	return (word << shift) | (word >> ((-shift) & 31));
}

/**
 * ror32 - rotate a 32-bit value right
 * @word: value to rotate
 * @shift: bits to roll
 */
static inline __u32 ror32(__u32 word, unsigned int shift)
{
	return (word >> shift) | (word << (32 - shift));
}

/**
 * rol16 - rotate a 16-bit value left
 * @word: value to rotate
 * @shift: bits to roll
 */
static inline __u16 rol16(__u16 word, unsigned int shift)
{
	return (word << shift) | (word >> (16 - shift));
}

/**
 * ror16 - rotate a 16-bit value right
 * @word: value to rotate
 * @shift: bits to roll
 */
static inline __u16 ror16(__u16 word, unsigned int shift)
{
	return (word >> shift) | (word << (16 - shift));
}

/**
 * rol8 - rotate an 8-bit value left
 * @word: value to rotate
 * @shift: bits to roll
 */
static inline __u8 rol8(__u8 word, unsigned int shift)
{
	return (word << shift) | (word >> (8 - shift));
}

/**
 * ror8 - rotate an 8-bit value right
 * @word: value to rotate
 * @shift: bits to roll
 */
static inline __u8 ror8(__u8 word, unsigned int shift)
{
	return (word >> shift) | (word << (8 - shift));
}




/**
 * s_ffs - find first bit set，参数x为0返回0，其余返回1-32
 */
static __always_inline int s_ffs(unsigned int x)
{
	int r = 1;		// r为查找位置

	if (!x)			// 如果传入是0，则直接返回0
		return 0;
	if (!(x & 0xffff)) {	// 首次二分查找：查找低16-bit是否为0; 如果为0，则索引r+=16; 如果不为0，则索引不变，在低16-bit查找
		x >>= 16;
		r += 16;
	}
	if (!(x & 0xff)) {		// 再次二分查找：查找新数据的低8-bit是否为0；
		x >>= 8;
		r += 8;
	}
	if (!(x & 0xf)) {		// 再次二分查找：查找新数据的低4-bit是否为0
		x >>= 4;
		r += 4;
	}
	if (!(x & 3)) {			// 继续二分查找
		x >>= 2;
		r += 2;
	}
	if (!(x & 1)) {			// 最后一次二分查找
		x >>= 1;
		r += 1;
	}
	return r;
}

static __always_inline int s_fls(unsigned int x)  // x=0返回0，其余返回1-32
{
	int r = 32;					//

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {	// 同上，查询最高
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}


#define s_ffz(x)  s_ffs(~(x))  // ffz实现很机智
#define s_flz(x)  s_fls(~(x))



// 64位操作使用32位组合即可
static __always_inline  unsigned int s_ffs64(u64 word)
{
	if (((u32)word) == 0UL)
		return s_ffs((u32)(word >> 32)) + 32;
	return s_ffs((unsigned int)word);
}

// 64位操作使用32位组合即可
static __always_inline  unsigned int s_fls64(u64 word)
{
	if(!word)
		return 0;
	if ( ((u32)(word>>32)) == 0UL) // 如果高32bit为0，则直接查找低32bit
		return s_fls((u32)(word));

	return s_fls((u32)(word >> 32)) + 32;   // 否则直接查找高32bit哪个为1
}



static __always_inline  unsigned int s_ffz64(u64 word)
{
	if( word == ~0UL )
		return 0;
	if( ((u32)(word)) != 0xFFFFFFFF) // ffz用来查找二进制数中第一个为0的位， 优先判断低32-bit
		return s_ffz((u32)(word));
	return s_ffz((u32)(word>>32)) + 32;
}

static __always_inline  unsigned int s_flz64(u64 word)
{
	if( word == ~0UL )
		return 0;
	if( ((u32)(word>>32)) != 0xFFFFFFFF){ // flz用来查找二进制数中最后一个为0的位，优先判断高32-bit
		return s_flz((u32)(word >> 32)) + 32;
	}
	return s_flz((u32)(word));
}


#define roundup_pow_of_two(x) ( ((x) & (x-1)) ?  BIT(s_fls64(x)) : (x) )


#endif





