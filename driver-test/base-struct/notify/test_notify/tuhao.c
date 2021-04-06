/*************************************************************************
    > File Name: tuhao.c
    > Author: baiy
    > Mail: yang01.bai@horizon.ai 
    > Created Time: 2020-11-30-15:14:20
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
/*Tuhao.c*/

extern int register_godness_notifier(struct notifier_block*);
extern int unregister_godness_notifier(struct notifier_block*);

static int baby_need_money(struct notifier_block *this, unsigned long event, void *ptr)
{
        if(event != 0)  //不是金钱需求关我鸟事
        {
            return NOTIFY_DONE; //Don't care
        }
        printk("[Tuhao]Hi Baby,$$$$$$$$ 么么哒 \n");
        return NOTIFY_OK;
}

static struct notifier_block cash_notifier =
{
        .notifier_call = baby_need_money,
        .priority = 2,
};

static int __init tuhao_register(void)
{
        int err;
        printk("[Tuhao]Tuhao register cash_requirment response to Godness...");

        err = register_godness_notifier(&cash_notifier);
        if (err)
        {
                printk("Refused!\n");
                return -1;
        }
        printk("Accepted!\n");

        return err;
}

static void __exit tuhao_unregister(void)
{
        unregister_godness_notifier(&cash_notifier);
        printk("[Tuhao]Tuhao is giving up Godness!(Son of bitch)\n");
}

module_init(tuhao_register);
module_exit(tuhao_unregister);
