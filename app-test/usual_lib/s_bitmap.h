/*************************************************************************
    > File Name: s_bitmap.h
    > Author: baiy
    > Mail: baiyang0223@163.com 
    > Created Time: 2020-12-30-11:45:59
    > Func: 
 ************************************************************************/
#ifndef S_BITMAP_H
#define S_BITMAP_H
#include "s_common.h"

// 向long取整，64位下是64-bits，32位是32-bits




#define BITS_PER_BYTE		8
#define BITS_PER_TYPE(type)	(sizeof(type) * BITS_PER_BYTE)
#define BITS_PER_LONG       __WORDSIZE
#define BITS_TO_LONGS(nr)	DIV_ROUND_UP(nr, BITS_PER_TYPE(long))
#define BITS_TO_U64(nr)		DIV_ROUND_UP(nr, BITS_PER_TYPE(u64))
#define BITS_TO_U32(nr)		DIV_ROUND_UP(nr, BITS_PER_TYPE(u32))
#define BITS_TO_BYTES(nr)	DIV_ROUND_UP(nr, BITS_PER_TYPE(char))


#define DECLARE_BITMAP(name,bits) unsigned long name[BITS_TO_LONGS(bits)]; \
    __bitmap_clear(name, 0, BITS_TO_LONGS(bits) * BITS_PER_LONG );


#define BIT_MASK(nr)		((1UL) << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)


// BITMAP_FIRST_WORD_MASK = 0xffff_ffff_ffff_ffff << start
// BITMAP_LAST_WORD_MASK
#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start) & (BITS_PER_LONG - 1)))
#define BITMAP_LAST_WORD_MASK(nbits) (~0UL >> (-(nbits) & (BITS_PER_LONG - 1)))


#define small_const_nbits(nbits) \
	(__builtin_constant_p(nbits) && (nbits) <= BITS_PER_LONG)



/***********************单个bitmap位操作接口*************************/
// 参考：linux/include/asm-generic/bitops/non-atomic.h
// 非原子操作，可被编译器排序


/**
 * __set_bit - Set a bit in memory
 * @nr:   the bit to set
 * @addr: the address to start counting from
 *
 * Unlike set_bit(), this function is non-atomic and may be reordered.
 * If it's called on the same region of memory simultaneously, the effect
 * may be that only one operation succeeds.
 */
static inline void __set_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

	*p  |= mask;
}


static inline void __clear_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

	*p &= ~mask;
}

/**
 * __change_bit - Toggle a bit in memory
 * @nr: the bit to change
 * @addr: the address to start counting from
 *
 * Unlike change_bit(), this function is non-atomic and may be reordered.
 * If it's called on the same region of memory simultaneously, the effect
 * may be that only one operation succeeds.
 */
static inline void __change_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

	*p ^= mask;
}

/**
 * __test_and_set_bit - Set a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is non-atomic and can be reordered.
 * If two examples of this operation race, one can appear to succeed
 * but actually fail.  You must protect multiple accesses with a lock.
 */
static inline int __test_and_set_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);
	unsigned long old = *p;

	*p = old | mask;
	return (old & mask) != 0;
}

/**
 * __test_and_clear_bit - Clear a bit and return its old value
 * @nr: Bit to clear
 * @addr: Address to count from
 *
 * This operation is non-atomic and can be reordered.
 * If two examples of this operation race, one can appear to succeed
 * but actually fail.  You must protect multiple accesses with a lock.
 */
static inline int __test_and_clear_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);
	unsigned long old = *p;

	*p = old & ~mask;
	return (old & mask) != 0;
}

/* WARNING: non atomic and it can be reordered! */
static inline int __test_and_change_bit(int nr,
					    volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);
	unsigned long old = *p;

	*p = old ^ mask;
	return (old & mask) != 0;
}

/**
 * test_bit - Determine whether a bit is set
 * @nr: bit number to test
 * @addr: Address to start counting from
 */
static inline int test_bit(int nr, const volatile unsigned long *addr)
{
	return 1UL & (addr[BIT_WORD(nr)] >> (nr & (BITS_PER_LONG-1)));
}


/***********************bitmap操作接口*************************/
// 参考：
//      linux/include/linux/bitops/bitmap.h
//      linux/lib/bitmap.c



extern unsigned long *bitmap_alloc(unsigned int nbits);
extern void bitmap_free(unsigned long *bitmap);
extern void __bitmap_set(unsigned long *map, unsigned int start, int len);
extern void __bitmap_clear(unsigned long *map, unsigned int start, int len);
extern int __bitmap_and(unsigned long *dst, const unsigned long *bitmap1,
				const unsigned long *bitmap2, unsigned int bits);
extern void __bitmap_or(unsigned long *dst, const unsigned long *bitmap1,
				const unsigned long *bitmap2, unsigned int bits);
extern void __bitmap_xor(unsigned long *dst, const unsigned long *bitmap1,
				const unsigned long *bitmap2, unsigned int bits);
extern int __bitmap_andnot(unsigned long *dst, const unsigned long *bitmap1,
				const unsigned long *bitmap2, unsigned int bits);
extern void __bitmap_complement(unsigned long *dst, const unsigned long *src, 
                unsigned int bits);


extern unsigned long find_first_bit(const unsigned long *addr, \
                unsigned long size);
extern unsigned long find_first_zero_bit(const unsigned long *addr, \
                unsigned long size);
extern unsigned long find_last_bit(const unsigned long *addr, unsigned long size);

extern unsigned long find_last_bit(const unsigned long *addr, unsigned long size);

extern unsigned long find_next_and_bit(const unsigned long *addr1,
		const unsigned long *addr2, unsigned long size,
		unsigned long offset);

extern unsigned long find_next_bit(const unsigned long *addr, unsigned long size,
			    unsigned long offset);


extern unsigned long find_next_zero_bit(const unsigned long *addr, unsigned long size,
				 unsigned long offset);


static inline void bitmap_zero(unsigned long *dst, unsigned int nbits)
{
	unsigned int len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
	memset(dst, 0, len);
}

static inline void bitmap_fill(unsigned long *dst, unsigned int nbits)
{
	unsigned int len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
	memset(dst, 0xff, len);
}

static inline void bitmap_copy(unsigned long *dst, const unsigned long *src,
			unsigned int nbits)
{
	unsigned int len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
	memcpy(dst, src, len);
}

/*
 * Copy bitmap and clear tail bits in last word.
 */
static inline void bitmap_copy_clear_tail(unsigned long *dst,
		const unsigned long *src, unsigned int nbits)
{
	bitmap_copy(dst, src, nbits);
	if (nbits % BITS_PER_LONG)
		dst[nbits / BITS_PER_LONG] &= BITMAP_LAST_WORD_MASK(nbits);
}




// bitmap 位操作
static inline int bitmap_and(unsigned long *dst, const unsigned long *src1,
			const unsigned long *src2, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		return (*dst = *src1 & *src2 & BITMAP_LAST_WORD_MASK(nbits)) != 0;
	return __bitmap_and(dst, src1, src2, nbits);
}

static inline void bitmap_or(unsigned long *dst, const unsigned long *src1,
			const unsigned long *src2, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		*dst = *src1 | *src2;
	else
		__bitmap_or(dst, src1, src2, nbits);
}

static inline void bitmap_xor(unsigned long *dst, const unsigned long *src1,
			const unsigned long *src2, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		*dst = *src1 ^ *src2;
	else
		__bitmap_xor(dst, src1, src2, nbits);
}

static inline int bitmap_andnot(unsigned long *dst, const unsigned long *src1,
			const unsigned long *src2, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		return (*dst = *src1 & ~(*src2) & BITMAP_LAST_WORD_MASK(nbits)) != 0;
	return __bitmap_andnot(dst, src1, src2, nbits);
}

static inline void bitmap_complement(unsigned long *dst, const unsigned long *src,
			unsigned int nbits)
{
	if (small_const_nbits(nbits))
		*dst = ~(*src);
	else
		__bitmap_complement(dst, src, nbits);
}



/**
 * 用二分查找法 从 unsigned long 类型中找到第一个为1的bit
 * 这里未定义 word是0的情况，所以使用前必须自己检测word是否为0
 * 成功返回[64位：0-63] 或 [32位：0-31]
 */
static __always_inline unsigned long __ffs(unsigned long word)
{
	int num = 0;

#if BITS_PER_LONG == 64
	if ((word & 0xffffffff) == 0) {
		num += 32;
		word >>= 32;
	}
#endif
	if ((word & 0xffff) == 0) {
		num += 16;
		word >>= 16;
	}
	if ((word & 0xff) == 0) {
		num += 8;
		word >>= 8;
	}
	if ((word & 0xf) == 0) {
		num += 4;
		word >>= 4;
	}
	if ((word & 0x3) == 0) {
		num += 2;
		word >>= 2;
	}
	if ((word & 0x1) == 0)
		num += 1;
	return num;
}

static __always_inline unsigned long __fls(unsigned long word)
{
	int num = BITS_PER_LONG - 1;

#if BITS_PER_LONG == 64
	if (!(word & (~0ul << 32))) {
		num -= 32;
		word <<= 32;
	}
#endif
	if (!(word & (~0ul << (BITS_PER_LONG-16)))) {
		num -= 16;
		word <<= 16;
	}
	if (!(word & (~0ul << (BITS_PER_LONG-8)))) {
		num -= 8;
		word <<= 8;
	}
	if (!(word & (~0ul << (BITS_PER_LONG-4)))) {
		num -= 4;
		word <<= 4;
	}
	if (!(word & (~0ul << (BITS_PER_LONG-2)))) {
		num -= 2;
		word <<= 2;
	}
	if (!(word & (~0ul << (BITS_PER_LONG-1))))
		num -= 1;
	return num;
}



/*
 * ffz - find first zero in word.
 * @word: The word to search
 *
 * Undefined if no zero exists, so code should check against ~0UL first.
 */
#define ffz(x)  __ffs(~(x))



#define for_each_set_bit(bit, addr, size) \
	for ((bit) = find_first_bit((addr), (size));		\
	     (bit) < (size);					\
	     (bit) = find_next_bit((addr), (size), (bit) + 1))

/* same as for_each_set_bit() but use bit as value to start with */
#define for_each_set_bit_from(bit, addr, size) \
	for ((bit) = find_next_bit((addr), (size), (bit));	\
	     (bit) < (size);					\
	     (bit) = find_next_bit((addr), (size), (bit) + 1))

#define for_each_clear_bit(bit, addr, size) \
	for ((bit) = find_first_zero_bit((addr), (size));	\
	     (bit) < (size);					\
	     (bit) = find_next_zero_bit((addr), (size), (bit) + 1))

/* same as for_each_clear_bit() but use bit as value to start with */
#define for_each_clear_bit_from(bit, addr, size) \
	for ((bit) = find_next_zero_bit((addr), (size), (bit));	\
	     (bit) < (size);					\
	     (bit) = find_next_zero_bit((addr), (size), (bit) + 1))


#endif//S_BITMAP_H
