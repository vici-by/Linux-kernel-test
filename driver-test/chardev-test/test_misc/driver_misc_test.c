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

#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 3, 0)
	#include <asm/switch_to.h>
#else
    #include <asm/system.h>
#endif



#define DEV_NAME "MISC_DEMO"

/* 定义设备类型 */
#define IOC_MAGIC  'm'
#define MISC_DEMO_TEST   	_IO(IOC_MAGIC, 0)

struct misc_demo_data {
	int num;
};


static ssize_t misc_demo_read (struct file * filp, char __user * buf, size_t size, loff_t * offset)
{
	return 0;
}
static ssize_t misc_demo_write (struct file * filp, const char __user * buf, size_t size, loff_t * offset)
{

	return 0;
}
static long misc_demo_ioctl (struct file * filp, unsigned int cmd, unsigned long args)
{
    switch(cmd){
    case MISC_DEMO_TEST:
        break;
    default:
        break;
    }
	return 0;
}

static int misc_demo_open (struct inode * inode, struct file * filp)
{
	return 0;
}
static int misc_demo_release (struct inode * inode, struct file * filp)
{
	return 0;
}




static struct file_operations misc_demo_fops = {
    .owner = THIS_MODULE,
	.open  = misc_demo_open,
	.release = misc_demo_release,
	.read  = misc_demo_read,
	.write = misc_demo_write,
	.unlocked_ioctl = misc_demo_ioctl,
	.compat_ioctl   = misc_demo_ioctl,
};

static struct miscdevice misc_demo_drv = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = DEV_NAME,
	.fops  = &misc_demo_fops,
};

static int __init misc_demo_init(void)
{
	pr_info("init [E]\n");
	misc_register(&misc_demo_drv);
	return 0;
}

static void __exit misc_demo_exit(void)
{
	pr_info("exit [E]\n");
	misc_deregister(&misc_demo_drv);
}
module_init(misc_demo_init);
module_exit(misc_demo_exit);

MODULE_LICENSE("GPL v2");


