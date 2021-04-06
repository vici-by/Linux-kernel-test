/*************************************************************************
    > File Name: driver_pcie_test.c
    > Author: baiy
    > Mail: baiyang0223@163.com 
    > Created Time: 2020-10-21-09:33:13
    > Func: 
 ************************************************************************/
#define pr_fmt(fmt)  "[%s:%d] " fmt, __func__, __LINE__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/string.h>

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <linux/iommu.h>
#include <linux/msi.h>
#include <linux/mm.h>
#include <linux/random.h>



#define DEV_NAME "RANDOM_DEMO"





#define IOC_MAGIC  'r'
#define PCIE_DEMO_GET_RANDOM       _IO(IOC_MAGIC, 0)







static ssize_t random_demo_read (struct file * filp, char __user * buf, size_t size, loff_t * offset)
{
    return 0;
}
static ssize_t random_demo_write (struct file * filp, const char __user * buf, size_t size, loff_t * offset)
{

    return 0;
}



struct random_demo_opt {
    unsigned long len;
    unsigned long addr;
};
static long random_demo_ioctl (struct file * filp, unsigned int cmd, unsigned long args)
{
    char * tmpbuf;
    struct random_demo_opt user_opt;
    struct random_demo_opt * opt;

    copy_from_user(&user_opt, (void*)args, sizeof(struct random_demo_opt));
    opt = &user_opt;
    switch(cmd){
        case PCIE_DEMO_GET_RANDOM:
		{
            tmpbuf = kmalloc(opt->len, GFP_KERNEL);
            if(!tmpbuf){
                pr_err("short of memory");
                return -ENOMEM;
            }
			memset(tmpbuf,0,opt->len);
			get_random_bytes_wait(tmpbuf,opt->len);
			copy_to_user((void *)opt->addr, tmpbuf,opt->len);	
		}
        break;
        default:
            pr_err("Not support command");
            return -1;
    }
    return 0;
}
static int random_demo_open (struct inode * inode, struct file * filp)
{
    filp->private_data = NULL;
    return 0;
}
static int random_demo_release (struct inode * inode, struct file * filp)
{
    filp->private_data = NULL;
    return 0;
}


static struct file_operations random_demo_fops = {
    .owner = THIS_MODULE,
    .open  = random_demo_open,
    .release = random_demo_release,
    .read  = random_demo_read,
    .write = random_demo_write,
    .unlocked_ioctl = random_demo_ioctl,
    .compat_ioctl   = random_demo_ioctl,
};

static struct miscdevice random_demo_drv = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = DEV_NAME,
    .fops  = &random_demo_fops,
};

static int __init random_demo_init(void)
{
    pr_info("init [E]\n");
    misc_register(&random_demo_drv);
    return 0;
}

static void __exit random_demo_exit(void)
{
    pr_info("exit [E]\n");
    misc_deregister(&random_demo_drv);
}

module_init(random_demo_init);
module_exit(random_demo_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("baiy <baiyang0223@163.com>");
MODULE_DESCRIPTION("TEST PCIE_DRIVER for GPU");
MODULE_VERSION("1.0.0.0");
