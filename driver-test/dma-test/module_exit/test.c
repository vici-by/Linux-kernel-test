/*************************************************************************
    > File Name: test.c
    > Author: baiy
    > Mail: baiyang0223@163.com 
    > Created Time: 2020-12-15-15:00:47
    > Func:  cat/proc/kallsyms | grep module 
 ************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/completion.h>


static int __init force_demo_init(void)
{
	complete((struct completions * )());
	return 0;
}

static void __exit force_demo_exit(void)
{
    pr_info("exit [E]\n");
}
module_init(force_demo_init);
module_exit(force_demo_exit);

MODULE_LICENSE("GPL v2");
