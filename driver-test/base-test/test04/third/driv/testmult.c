/*
 * a simple char device driver: globalmem without mutex
 *
 * Copyright (C) 2014 Barry Song  (baohua@kernel.org)
 *
 * Licensed under GPLv2 or later.
 */

#define pr_fmt(fmt) "GLOBAL_MEM_DEBUG: " fmt

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <asm-generic/fcntl.h>
#include <linux/ioctl.h>
#include <asm-generic/ioctl.h>

#define GLOBALMEM_SIZE	0x1000
#define DEVICE_NUM        4

#define MAGIC_NUM 'G'
#define MEM_CLEAR _IO(MAGIC_NUM,1)

static int globalmem_major = 0;
module_param(globalmem_major, int, S_IRUGO);
MODULE_PARM_DESC(globalmem_major,"globalmem_major  num");


struct globalmem_dev {
	struct cdev cdev;
	struct class * cls;
	struct device * dev;
	unsigned char mem[GLOBALMEM_SIZE];
};

struct globalmem_dev *globalmem_devp;


static int globalmem_open(struct inode *inode, struct file *filp)
{
	struct globalmem_dev * devp = container_of(inode->i_cdev, struct globalmem_dev, cdev);
	filp->private_data = devp;
	pr_info("open globalmem\n");
	return 0;
}

static int globalmem_release(struct inode *inode, struct file *filp)
{
	pr_info("close globalmem\n");

	return 0;
}


static const struct file_operations globalmem_fops = {
	.owner = THIS_MODULE,
	.open = globalmem_open,
	.release = globalmem_release,
};





static int __init globalmem_init(void)
{
	int ret;
	dev_t devno;


	if (globalmem_major) {
		devno  = MKDEV(globalmem_major, 0);
		ret = register_chrdev_region(devno, DEVICE_NUM, "globalmem");
	} else {
		ret = alloc_chrdev_region(&devno, 0, DEVICE_NUM, "globalmem");
		globalmem_major = MAJOR(devno);
	}
	if (ret < 0){
		pr_err("reg/alloc char dev failed\n");
		goto fail_reg_chrdev;
	}
	
	cdev_init(&globalmem_devp->cdev, &globalmem_fops);
	globalmem_devp->cdev.owner = THIS_MODULE;
	ret = cdev_add(&globalmem_devp->cdev, devno, DEVICE_NUM);
	if (ret){
		pr_err("Error %d adding globalmem\n", ret);
		goto fail_cdev;
	}
	return 0;
fail_device:
	cdev_del(&globalmem_devp->cdev);
fail_cdev:
	class_destroy(globalmem_devp->cls);
fail_cls:
	unregister_chrdev_region(devno, DEVICE_NUM);
fail_reg_chrdev:
	return ret;
}


static void __exit globalmem_exit(void)
{
	dev_t devno;
	int index;
	
	devno  = MKDEV(globalmem_major, 0);
	cdev_del(&globalmem_devp->cdev);
	class_destroy(globalmem_devp->cls);
	unregister_chrdev_region(devno, DEVICE_NUM);
}


module_init(globalmem_init);
module_exit(globalmem_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("baiy <baiyang0223@163.com>");
MODULE_DESCRIPTION("This is a test driver");
MODULE_VERSION("1.0.0.0");
