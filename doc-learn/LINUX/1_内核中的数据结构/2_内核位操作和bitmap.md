# 2_内核位操作和bitmap







## 内核基础位操作

### ffs和fls

<font color=red>ffs用来查找二进制数中第一个为1的位</font>

<font color=red>fls用来查找二进制数中最后一个为1的位</font>

<font color=red>ffz用来查找二进制数中第一个为0的位</font>



问题：如果自己实现该怎么实现？

- 最无聊的方法就是 从 低->高/高->低  每1bit进行判断。

- 比较好的方法就是 二分查找。
- X86系统还支持BSR和BSF指令快速查找







#### 用户态ffs

其实用户态也实现了ffs相关函数

```
#include <strings.h>

int ffs(int i);
int ffsl(long int i);
int ffsll(long long int i);
```



#### 内核态ffs和fls

##### 位扫描指令 (了解）

这里值得一提的是：<font color=red>X86内部集成了 BSR和BSF位扫描指令</font>，可以快速检测，参考：[gcc 嵌入式汇编(asm)实现bsr(位扫描)指令](https://blog.csdn.net/10km/article/details/48997303)。

注：这里我们暂时只需要知道有这个功能就好



> BSR-逆向位扫描指令，BSF - 正向位扫描 (386以上CPU可用)
>
> 
>
> 格式: BSF dest, src
>
> 影响标志位: ZF
>
> 功能：从源操作数的的最低位向高位搜索，将遇到的第一个“1”所在的位序号存入目标[寄存器](https://baike.baidu.com/item/)寄存器中，
>
> 若所有位都是0，则ZF=1，否则ZF=0。
>
> 
>
> 格式: BSR dest, src
>
> 影响标志位: ZF
>
> 功能：从源操作数的的最高位向低位搜索，将遇到的第一个“1”所在的位序号存入目标[寄存器](https://baike.baidu.com/item/寄存器)中，
>
> 若所有位都是0，则ZF=1，否则ZF=0。



内核实现参考：[arch\x86\include\asm\bitops.h X86下汇编进行位扫描](https://github.com/torvalds/linux/blob/master/arch/x86/include/asm/bitops.h)

```
static __always_inline unsigned long ffz(unsigned long word); // find first zero bit in word
static __always_inline int ffs(int x); // find first set bit in word - ffs=find first set
static __always_inline int fls(int x); // find last set bit in word  - fls=find last set
static __always_inline int fls64(__u64 x); // find last set bit in a 64-bit word

// 注：这里边带_ffs和_fls都是不进行异常检测的，用户必须在传参时判断参数为0的情况
// 那么如何实现ffs64呢？？？ 用ffs组合下即可
static inline unsigned long __ffs64(u64 word)
{
#if BITS_PER_LONG == 32
	if (((u32)word) == 0UL)
		return __ffs((u32)(word >> 32)) + 32;
#elif BITS_PER_LONG != 64
#error BITS_PER_LONG not 32 or 64
#endif
	return __ffs((unsigned long)word);
}
```





##### 软件实现ffs和fls

```
/**
 * ffs - find first bit set，参数x为0返回0，其余返回1-32
 */
static __always_inline int ffs(unsigned int x)
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

static __always_inline int fls(unsigned int x)  // x=0返回0，其余返回1-32
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

#define ffz(x)  __ffs(~(x))  // ffz实现很机智


// 64位操作使用32位组合即可
static __always_inline  unsigned int ffs64(u64 word)
{
	if (((u32)word) == 0UL)
		return ffs((u32)(word >> 32)) + 32;
	return ffs((unsigned int)word);
}

// 64位操作使用32位组合即可
static __always_inline  unsigned int fls64(u64 word)
{
	if(!word)
		return 0;
	if ( ((u32)(word>>32)) == 0UL) // 如果高32bit为0，则直接查找低32bit
		return ffs((u32)(word)) + 32;

	return ffs((u32)(word>>32));   // 否则直接查找高32bit哪个为1
}

```











### roundup_pow_of_two

最<font color=red>接近的（向上取整） 最大2的指数次幂roundup_pow_of_two</font> 分析与实现：

思想： 2^n有个特点，就是    二进制数中最后一个为1， 其他位都为0.  那么实现非常简单了

首先判断这个数是不是2^n (只要判断与自己的mask相与，为0则是2^n,非0则不是2^n)，如果是，则就是它自己。 如果不是，那么 1<<fls64(x)即可

```
roundup_pow_of_two(x) ( ((x) & (x-1)) ?  BIT(fls64(x)) : (x) )
```

内核实现思想一样：

```

static inline __attribute__((const))
unsigned long __roundup_pow_of_two(unsigned long n)
{
	return 1UL << fls_long(n - 1);
}

#define roundup_pow_of_two(n)			\
(						\
	__builtin_constant_p(n) ? (		\
		(n == 1) ? 1 :			\
		(1UL << (ilog2((n) - 1) + 1))	\
				   ) :		\
	__roundup_pow_of_two(n)			\
 )
```





### 相关测试











## 内核的bitmap

