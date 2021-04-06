/*************************************************************************
    > File Name: gfs.c
    > Author: baiy
    > Mail: baiyang0223@163.com 
    > Created Time: 2020-11-30-15:22:44
    > Func: 
 ************************************************************************/

/*GFS.c*/

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
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 3, 0)
	#include <asm/switch_to.h>
#else
    #include <asm/system.h>
#endif
MODULE_LICENSE("GPL");
/*
* 注册通知链
*/

extern int register_godness_notifier(struct notifier_block*);
extern int unregister_godness_notifier(struct notifier_block*);

static int sweet_heart_requirments(struct notifier_block *this, unsigned long event, void *ptr)
{
        switch(event)
        {
                case 0:
                        printk("[GFS]Hi honey,the VISA card is ready for you! \n");
                        break;
                case 1:
                        printk("[GFS]Hi honey,let me play the piano for you! \n");
                        break;
                default:
                        break;
        }
        return 0;
}

static struct notifier_block honey_notifier =
{
        .notifier_call = sweet_heart_requirments,
        .priority = 5,
};

static int __init GFS_register(void)
{
        int err;
        printk("[GFS]GFS register honey_requirment response to Godness...");

        err = register_godness_notifier(&honey_notifier);
        if (err)
        {
                printk("Refused!\n");
                return -1;
        }
        printk("Accepted!\n");

        return err;
}

/*
* 卸载刚刚注册了的通知链
*/
static void __exit GFS_unregister(void)
{
        unregister_godness_notifier(&honey_notifier);
        printk("[GFS]GFS broke up with Godness!(How sadness)\n");
}
module_init(GFS_register);
module_exit(GFS_unregister);
