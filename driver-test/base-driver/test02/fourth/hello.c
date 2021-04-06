
#define pr_fmt(fmt) "TEST PR_FMT: " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <asm-generic/bug.h>

static int __init hello_init(void)
{
    printk(KERN_INFO"%s,%s,line=%d\n",__FILE__,__func__,__LINE__);
    pr_info("hello init\n");

	// BUG_ON(true); #  会导致内核异常
	WARN_ON(true);

    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_INFO"%s,%s,line=%d\n",__FILE__,__func__,__LINE__);
    pr_info("hello exit\n");

    return;
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("baiy <baiyang0223@163.com>");
MODULE_DESCRIPTION("This is a test driver");
MODULE_VERSION("1.0.0.0");
