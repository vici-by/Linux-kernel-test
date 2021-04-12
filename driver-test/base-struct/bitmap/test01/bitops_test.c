/*************************************************************************
    > File Name: bitops_test.c
    > Author: baiy
    > Mail: baiyang0223@163.com
    > Created Time: 2021-04-06-14:34:52
    > Func:  测试Linux 内核位操作
    > Doc:
 ************************************************************************/
#define pr_fmt(fmt) "[%s:%d]: " fmt, __func__, __LINE__
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/notifier.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/version.h>
#include <linux/slab.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 3, 0)
    #include <asm/switch_to.h>
#else
    #include <asm/system.h>
#endif

#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/bitops.h>

#include <linux/log2.h>
#include <linux/bitmap.h>



// 利用模块回调参数进行验证
static int bitops_run_set(const char *val, const struct kernel_param *kp);
static int bitops_run_get(char *val, const struct kernel_param *kp);
static const struct kernel_param_ops run_ops = {
	.set = bitops_run_set,
	.get = bitops_run_get,
};

static ulong bitops_run;
module_param_cb(run, &run_ops, &bitops_run, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(run, "Run the test (default: false)");



#if 1 // test_bit_ops
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
		pr_info("flz is %#x\n", s_flz((u32)(word >> 32)));
		return s_flz((u32)(word >> 32)) + 32;
	}
	return s_flz((u32)(word));

}
#endif

static int bitops_run_get(char *val, const struct kernel_param *kp)
{
	// mutex_lock(&info->lock);
	// mutex_unlock(&info->lock);

	return param_get_ulong(val, kp);
}

static int bitops_run_set(const char *val, const struct kernel_param *kp)
{
	int ret;

	// mutex_lock(&info->lock);
	ret = param_set_ulong(val, kp);
	if (ret) {
		// mutex_unlock(&info->lock);
		return ret;
	}
	pr_info("val: %#lx,roundup_pow_of_two: %#lx,ffs32:%#x, fls32:%#x, ffz32:%#x, flz32:%#x\n",
		bitops_run, roundup_pow_of_two(bitops_run),s_ffs(bitops_run),s_fls(bitops_run),s_ffz(bitops_run), s_flz(bitops_run));

	pr_info("val: %#lx,roundup_pow_of_two: %#lx,ffs64:%#x, fls64:%#x, ffz64:%#x, flz64:%#x\n",
		bitops_run, roundup_pow_of_two(bitops_run),s_ffs64(bitops_run),s_fls64(bitops_run),s_ffz64(bitops_run), s_flz64(bitops_run));

	// mutex_unlock(&info->lock);

	return ret;
}


static int __init bitops_dev_init(void)
{
	pr_info("bitops init[E]");
	pr_info("bitops init[X]");
    return 0;
}
module_init(bitops_dev_init);

static void __exit bitops_dev_exit(void)
{
	pr_info("bitops exit");
}
module_exit(bitops_dev_exit);

MODULE_LICENSE("GPL v2");
MODULE_INFO(supported, "Test driver that simulate serial port over PCI");
