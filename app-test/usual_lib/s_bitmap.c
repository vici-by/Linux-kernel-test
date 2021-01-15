#include "s_bitmap.h"


/*********************** bitmap 申请和释放 ***********************/
unsigned long *bitmap_alloc(unsigned int nbits)
{
    unsigned long * bitmap;
    bitmap = malloc(BITS_TO_LONGS(nbits) * sizeof(unsigned long));

    if(unlikely(bitmap == NULL)){
        fprintf(stderr,"[%s:%s %d]Short of Memory\n",__FILE__,__func__,__LINE__);
        return NULL;
    }
    memset(bitmap, 0, BITS_TO_LONGS(nbits) * sizeof(unsigned long));
    return bitmap;
}

void bitmap_free(unsigned long *bitmap)
{
	free(bitmap);
}





/*********************** bitmap 查找函数 ***********************/


/*
 * 根据invert参数，查找下一个符合要求的位置
 * This is a common helper function for find_next_bit, find_next_zero_bit, and
 * find_next_and_bit. The differences are:
 *  - The "invert" argument, which is XORed with each fetched word before
 *    searching it for one bits.
 *  - The optional "addr2", which is anded with "addr1" if present.
 */
static inline unsigned long _find_next_bit(const unsigned long *addr1,
		const unsigned long *addr2, unsigned long nbits,
		unsigned long start, unsigned long invert)
{
	unsigned long tmp;

	if (unlikely(start >= nbits))
		return nbits;

	tmp = addr1[start / BITS_PER_LONG];
	if (addr2)
		tmp &= addr2[start / BITS_PER_LONG];
	tmp ^= invert;

	/* Handle 1st word. */
	tmp &= BITMAP_FIRST_WORD_MASK(start);
	start = round_down(start, BITS_PER_LONG);

	while (!tmp) {
		start += BITS_PER_LONG;
		if (start >= nbits)
			return nbits;

		tmp = addr1[start / BITS_PER_LONG];
		if (addr2)
			tmp &= addr2[start / BITS_PER_LONG];
		tmp ^= invert;
	}

	return min(start + __ffs(tmp), nbits);
}

/*
 * Find the next set bit in a memory region.
 */
unsigned long find_next_bit(const unsigned long *addr, unsigned long size,
			    unsigned long offset)
{
	return _find_next_bit(addr, NULL, size, offset, 0UL);
}
                

/*
 * bitmap 查找函数，返回第一个为1的bit所在位置，或者返回size
 */
unsigned long find_first_bit(const unsigned long *addr, unsigned long size)
{
	unsigned long idx;

	for (idx = 0; idx * BITS_PER_LONG < size; idx++) {
		if (addr[idx]) // ffs未定义addr[idx]为0的情况，必须手动检查
			return min(idx * BITS_PER_LONG + __ffs(addr[idx]), size);
	}

	return size;
}

unsigned long find_last_bit(const unsigned long *addr, unsigned long size)
{
	if (size) {
		unsigned long val = BITMAP_LAST_WORD_MASK(size);
		unsigned long idx = (size-1) / BITS_PER_LONG;

		do {
			val &= addr[idx];
			if (val)
				return idx * BITS_PER_LONG + __fls(val);

			val = ~0ul;
		} while (idx--);
	}
	return size;
}

/*
 * Find the first cleared bit in a memory region.
 */
unsigned long find_first_zero_bit(const unsigned long *addr, unsigned long size)
{
	unsigned long idx;

	for (idx = 0; idx * BITS_PER_LONG < size; idx++) {
		if (addr[idx] != ~0UL)
			return min(idx * BITS_PER_LONG + ffz(addr[idx]), size);
	}

	return size;
}

unsigned long find_next_zero_bit(const unsigned long *addr, unsigned long size,
				 unsigned long offset)
{
	return _find_next_bit(addr, NULL, size, offset, ~0UL);
}

unsigned long find_next_and_bit(const unsigned long *addr1,
		const unsigned long *addr2, unsigned long size,
		unsigned long offset)
{
	return _find_next_bit(addr1, addr2, size, offset, 0UL);
}



// 批量设置bitmap，从某个位开始设置多少字节
// 比如start=3，len=80；
// *p = map
// bits_to_set = 64-(3 % 8) == 61
// mask_to_set = 0xffff_ffff_ffff_ffff << 3 = FFFFFFFFFFFFFFF8
// len - bits_to_set
//      循环1：80 - 61 == 19, *p |= FFFFFFFFFFFFFFF8 // 一下将[3,63] 都置1
//                len -= bits_to_set, len = 19; 
//                bits_to_set = 64
//      循环2：条件不符，退出循环，此时，len=19
//  此时，len > 0,start+len = size = 83
//    *p |= mask_to_set ; // 将多出得bit都置1
void __bitmap_set(unsigned long *map, unsigned int start, int len)
{
	unsigned long *p = map + BIT_WORD(start);   // p是start所在得 unsigned long 变量里边
	const unsigned int size = start + len;      // 
	int bits_to_set = BITS_PER_LONG - (start % BITS_PER_LONG);  // 
	unsigned long mask_to_set = BITMAP_FIRST_WORD_MASK(start);  // 

	while (len - bits_to_set >= 0) {
		*p |= mask_to_set;
		len -= bits_to_set;
		bits_to_set = BITS_PER_LONG;
		mask_to_set = ~0UL;
		p++;
	}
	if (len) {
		mask_to_set &= BITMAP_LAST_WORD_MASK(size);
		*p |= mask_to_set;
	}
}


void __bitmap_clear(unsigned long *map, unsigned int start, int len)
{
	unsigned long *p = map + BIT_WORD(start);
	const unsigned int size = start + len;
	int bits_to_clear = BITS_PER_LONG - (start % BITS_PER_LONG);
	unsigned long mask_to_clear = BITMAP_FIRST_WORD_MASK(start);

	while (len - bits_to_clear >= 0) {
		*p &= ~mask_to_clear;
		len -= bits_to_clear;
		bits_to_clear = BITS_PER_LONG;
		mask_to_clear = ~0UL;
		p++;
	}
	if (len) {
		mask_to_clear &= BITMAP_LAST_WORD_MASK(size);
		*p &= ~mask_to_clear;
	}
}


int __bitmap_and(unsigned long *dst, const unsigned long *bitmap1,
				const unsigned long *bitmap2, unsigned int bits)
{
	unsigned int k;
	unsigned int lim = bits/BITS_PER_LONG;
	unsigned long result = 0;

	for (k = 0; k < lim; k++)
		result |= (dst[k] = bitmap1[k] & bitmap2[k]);
	if (bits % BITS_PER_LONG)
		result |= (dst[k] = bitmap1[k] & bitmap2[k] &
			   BITMAP_LAST_WORD_MASK(bits));
	return result != 0;
}


void __bitmap_or(unsigned long *dst, const unsigned long *bitmap1,
				const unsigned long *bitmap2, unsigned int bits)
{
	unsigned int k;
	unsigned int nr = BITS_TO_LONGS(bits);

	for (k = 0; k < nr; k++)
		dst[k] = bitmap1[k] | bitmap2[k];
}


void __bitmap_xor(unsigned long *dst, const unsigned long *bitmap1,
				const unsigned long *bitmap2, unsigned int bits)
{
	unsigned int k;
	unsigned int nr = BITS_TO_LONGS(bits);

	for (k = 0; k < nr; k++)
		dst[k] = bitmap1[k] ^ bitmap2[k];
}


int __bitmap_andnot(unsigned long *dst, const unsigned long *bitmap1,
				const unsigned long *bitmap2, unsigned int bits)
{
	unsigned int k;
	unsigned int lim = bits/BITS_PER_LONG;
	unsigned long result = 0;

	for (k = 0; k < lim; k++)
		result |= (dst[k] = bitmap1[k] & ~bitmap2[k]);
	if (bits % BITS_PER_LONG)
		result |= (dst[k] = bitmap1[k] & ~bitmap2[k] &
			   BITMAP_LAST_WORD_MASK(bits));
	return result != 0;
}

void __bitmap_complement(unsigned long *dst, const unsigned long *src, 
                unsigned int bits)
{
	unsigned int k, lim = BITS_TO_LONGS(bits);
	for (k = 0; k < lim; ++k)
		dst[k] = ~src[k];
}
