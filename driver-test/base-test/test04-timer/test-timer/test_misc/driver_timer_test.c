/*************************************************************************
    > File Name: driver_misc_test.c
    > Author: baiy
    > Mail: baiyang0223@163.com 
    > Created Time: 2020-10-21-09:33:13
    > Func: 
 ************************************************************************/

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#include <linux/miscdevice.h>
#include <linux/time.h>
#include <linux/timer.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 3, 0)
	#include <asm/switch_to.h>
#else
    #include <asm/system.h>
#endif
#include <linux/timekeeping.h>

struct test_info {
    struct timer_list test_timer;
};
struct test_info tinfo;

static void test_timer(struct timer_list *t)
{
	struct test_info *ts = from_timer(ts, t, test_timer);
	pr_info("test_timer  ts is %p\n",ts);
	mod_timer(&tinfo.test_timer, jiffies+msecs_to_jiffies(1000));
}

static int __init misc_demo_init(void)
{
	pr_info("init [E],tinfo is %pk\n",&tinfo);
    timer_setup(&tinfo.test_timer, test_timer, 0);
	mod_timer(&tinfo.test_timer, jiffies+msecs_to_jiffies(1000));

	return 0;
}

static void __exit misc_demo_exit(void)
{
	pr_info("exit [E]\n");
    del_timer_sync(&tinfo.test_timer);
}
module_init(misc_demo_init);
module_exit(misc_demo_exit);

MODULE_LICENSE("GPL v2");


