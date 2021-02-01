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

#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 3, 0)
	#include <asm/switch_to.h>
#else
    #include <asm/system.h>
#endif
#include <linux/timekeeping.h>


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
        {
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
            struct timeval * utv = (struct timeval *)args;
            struct timeval ktv;
            do_gettimeofday(&ktv);
            pr_info("User time is %ld\n", utv->tv_sec*1000000 + utv->tv_usec );
            pr_info("Ker  time is %ld\n", ktv.tv_sec*1000000 + ktv.tv_usec );
#else
            struct timespec uts;
            struct timespec64 kts;
            ktime_t ukm;
            ktime_t kkm;

            copy_from_user(&uts, (struct timespec *)args, sizeof(struct timespec));
            ktime_get_real_ts64(&kts);
            printk("User sec %ld, nsec %ld\n",uts.tv_sec, uts.tv_nsec);
            printk("KERN sec %lld, nsec %ld\n",kts.tv_sec, kts.tv_nsec);
            printk("Current ns is %lld\n", ktime_get_real_ns());
            //ukm = ktime_set(uts.tv_sec, uts.tv_nsec);
            //kkm = timespec64_to_ktimekts();
            //printf("Cost %lldns\n", ktime_to_ns(ktime_sub()));

#endif
        }
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


