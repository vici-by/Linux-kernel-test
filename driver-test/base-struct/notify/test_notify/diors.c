/*************************************************************************
    > File Name: diors.c
    > Author: baiy
    > Mail: yang01.bai@horizon.ai 
    > Created Time: 2020-11-30-15:14:40
    > Func: 
 ************************************************************************/

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
/*Diors.c*/

extern int register_godness_notifier(struct notifier_block*);
extern int unregister_godness_notifier(struct notifier_block*);

static int godness_need_music(struct notifier_block *this, unsigned long event, void *ptr)
{
        if(event != 1) //我又没钱，给不了你大房子、气派的车子...
        {
            return NOTIFY_DONE; //Don't care
        }
        printk("[Diors]Hi girl,This is a classic Music disk,take it. \n");
        return NOTIFY_OK;
}

static struct notifier_block music_notifier =
{
        .notifier_call = godness_need_music,
        .priority = 2,
};

static int __init diors_register(void)
{
        int err;
        printk("[Diors]Diors register music_requirment response to Godness...");

        err = register_godness_notifier(&music_notifier);
        if (err)
        {
                printk("Refused!\n");
                return -1;
        }
        printk("Accepted!\n");

        return err;
}

static void __exit diors_unregister(void)
{
        unregister_godness_notifier(&music_notifier);
        printk("[Diors]Tuhao is giving up Godness!(What a pity)\n");
}

module_init(diors_register);
module_exit(diors_unregister);
