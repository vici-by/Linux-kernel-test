/*
 * a simple char device driver: testioctl without mutex
 *
 * Copyright (C) 2014 Barry Song  (baohua@kernel.org)
 *
 * Licensed under GPLv2 or later.
 */

#define pr_fmt(fmt) "IOCTL_MEM_DEBUG: " fmt

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <asm-generic/fcntl.h>
#include <asm-generic/ioctl.h>
#include <linux/ioctl.h>

#define MAGIC_NUM 'G'
#define TEST_01 _IO(MAGIC_NUM,1)
#define TEST_02 _IOR(MAGIC_NUM,2,int)
#define TEST_03 _IOW(MAGIC_NUM,3,int) //读写一个int数据,size大小可自己填写。
#define TEST_04 _IOWR(MAGIC_NUM,4,100)
#define TEST_MAX _IO(MAGIC_NUM,5)


static int testioctl_major = 0;
module_param(testioctl_major, int, S_IRUGO);
MODULE_PARM_DESC(testioctl_major,"testioctl_major  num");


struct testioctl_dev {
	struct cdev cdev;
	struct class * cls;
	struct device * dev;
};
struct testioctl_dev *testioctl_devp;


static int testioctl_open(struct inode *inode, struct file *filp)
{
	struct testioctl_dev * devp = container_of(inode->i_cdev, struct testioctl_dev, cdev);
	filp->private_data = devp;
	pr_info("open testioctl\n");
	return 0;
}

static int testioctl_release(struct inode *inode, struct file *filp)
{
	pr_info("close testioctl\n");
	return 0;
}

static long testioctl_ioctl(struct file *filp, unsigned int cmd,
			    unsigned long args)
{

	struct testioctl_dev *dev = filp->private_data;
	char buf[100] = {0};
	int  ioarg;
	int  retval;
	int  err;

	/*检查类型，幻数是否正确*/
	if(_IOC_TYPE(cmd)!=MAGIC_NUM)
		return -EINVAL;

	/*检测命令序号是否大于允许的最大序号*/
	if(_IOC_NR(cmd)> TEST_MAX)
		return -EINVAL;	

	/*检测读写权限属性*/
	/*
	if(_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_READ,(void *)args,_IOC_SIZE(cmd));
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_WRITE,(void *)args,_IOC_SIZE(cmd));
	// 这里测试方向，所以暂时不反悔
	if(err)
		return -EFAULT;
	*/

	switch (cmd) {
	case TEST_01:
		pr_info("%s-%d: cmd is test01\n", __func__, __LINE__);
        pr_info("tgid=%lu, pgid=%lu\n",(unsigned long)(current->pid), (unsigned long)(current->tgid));
		break;

	case TEST_02:
		if(_IOC_DIR(cmd) & _IOC_READ)
			err = !access_ok(VERIFY_READ,(void *)args,_IOC_SIZE(cmd));
		if(err)
			return -EFAULT;
		ioarg = 0xA5A5A5A5;
		retval =   __put_user(ioarg,(int *)args);
		pr_info("%s-%d: cmd is test02-put %#x,_IOC_SIZE(cmd) is %d\n", \
			__func__, __LINE__,ioarg,_IOC_SIZE(cmd));
		break;

	case TEST_03:
		if(_IOC_DIR(cmd) & _IOC_WRITE)
			err = !access_ok(VERIFY_WRITE,(void *)args,_IOC_SIZE(cmd));
		if(err)
			return -EFAULT;

		retval =   __get_user(ioarg,(int *)args);
		pr_info("%s-%d: cmd is test03-get %#x\n", __func__, __LINE__,ioarg);
		break;

	/*				
	case TEST_04:
		if(_IOC_DIR(cmd) & _IOC_READ)
			err = !access_ok(VERIFY_READ,(void *)args,_IOC_SIZE(cmd));
		if(err)
			return -EFAULT;
		if(_IOC_DIR(cmd) & _IOC_WRITE)
			err = !access_ok(_IOC_WRITE,(void *)args,_IOC_SIZE(cmd));
		if(err)
			return -EFAULT;
		
		retval = copy_from_user(buf,(void *)args, 100);
		pr_info("%s-%d: cmd is test04，read %s, write %s\n", 
			__func__, __LINE__,buf,"nice to meet you,too.");
	
		memset(buf,0,100);
		strcpy(buf,"nice to meet you,too.");
		retval = copy_to_user((void *)args, 100,buf);
		
		break;
	*/
	default:
		return -EINVAL;
	}

	return 0;
}

static const struct file_operations testioctl_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = testioctl_ioctl,
	.open = testioctl_open,
	.release = testioctl_release,
};

static int __init testioctl_init(void)
{
	int ret;
	dev_t devno;

	testioctl_devp = kzalloc(sizeof(struct testioctl_dev), GFP_KERNEL);
	if (!testioctl_devp) {
		pr_err("alloc testioctl_devp failed\n");
		return -ENOMEM;
	}

	if (testioctl_major) {
		devno = MKDEV(testioctl_major, 0);
		ret = register_chrdev_region(devno, 1, "testioctl");
	} else {
		ret = alloc_chrdev_region(&devno, 0, 1, "testioctl");
		testioctl_major = MAJOR(devno);
	}
	if (ret < 0){
		pr_err("reg/alloc char dev failed\n");
		goto fail_reg_chrdev;
	}

	cdev_init(&testioctl_devp->cdev, &testioctl_fops);
	testioctl_devp->cdev.owner = THIS_MODULE;
	ret = cdev_add(&testioctl_devp->cdev, devno, 1);
	if (ret){
		pr_err("Error %d adding testioctl\n", ret);
		goto fail_cdev;
	}


	testioctl_devp->cls = class_create(THIS_MODULE,"testioctl");
	if(testioctl_devp->cls == NULL){
		pr_err("class create failed\n");
		ret = PTR_ERR(testioctl_devp->cls);
		goto fail_cls;	
	}

	testioctl_devp->dev = device_create(testioctl_devp->cls, NULL, devno, NULL, "testioctl");
	if (IS_ERR(testioctl_devp->dev))
	{
		pr_err("device create failed\n");
		ret = PTR_ERR(testioctl_devp->dev);
		goto fail_device;
	}
	pr_info("Regist char device success\n");
	return 0;

fail_device:
	class_destroy(testioctl_devp->cls);
fail_cls:
	cdev_del(&testioctl_devp->cdev);
fail_cdev:
	unregister_chrdev_region(devno, 1);
fail_reg_chrdev:
	kfree(testioctl_devp);
	return ret;
}


static void __exit testioctl_exit(void)
{
	dev_t devno  = MKDEV(testioctl_major,0);
	device_destroy(testioctl_devp->cls,devno);
	class_destroy(testioctl_devp->cls);
	cdev_del(&testioctl_devp->cdev);
	unregister_chrdev_region(devno, 1);
	kfree(testioctl_devp);
	pr_info("Unregist char device success\n");
}


module_init(testioctl_init);
module_exit(testioctl_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("baiy <baiyang0223@163.com>");
MODULE_DESCRIPTION("This is a test driver");
MODULE_VERSION("1.0.0.0");
