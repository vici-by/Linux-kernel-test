/*************************************************************************
    > File Name: mychar.c
    > Author: baiy
    > Mail: yang01.bai@hobot.cc 
    > Created Time: 2018-07-27-16:56:21
	> Func: 
 ************************************************************************/

#define pr_fmt(fmt) "CHAR_DEBUG: " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>


#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <asm/pgtable.h>
#include <asm/io.h>


//文件操作相关结构体：
//struct file :描述一个已打开的文件 linux/fs.h
//struct file_operations: vfs与设备的接口函数，linux/fs.h
//struct cdev: 描述一个字符设备
//struct inode: 描述一个文件节点


static int major = 0;
module_param(major,int,0644);
MODULE_PARM_DESC(major,"major num");

static int __init mychar_init(void)
{
    int ret = 0;
    dev_t devno;
    //1.注册字符驱动
    if(major){
        devno = MKDEV(major,0);
        ret = register_chrdev_region(devno, 3,"mychar");
    }else{
        ret = alloc_chrdev_region(&devno,0, 3,"mychar");
        major = MAJOR(devno);
    }
    if(ret < 0){
        printk(KERN_ERR"failed region chrdev\n");
        goto failed_register;
    }
    pr_info("%s,%s-%d\n",__FILE__,__func__,__LINE__);

    return 0;//成功返回0，失败返回错误码

    // unregister_chrdev_region(devno,1);
failed_register:

    return -EINVAL;
}


static void __exit mychar_exit(void)
{
    dev_t devno;
    devno = MKDEV(major,0);
    unregister_chrdev_region(devno,3);
    pr_info("%s,%s-%d\n",__FILE__,__func__,__LINE__);
    return;
}

module_init(mychar_init);
module_exit(mychar_exit); //修饰入口和出口函数
MODULE_LICENSE("GPL");
MODULE_AUTHOR("baiy <baiyang0223@163.com>"); //作者信息，选填
MODULE_DESCRIPTION("This is a test driver"); //描述信息，选填
MODULE_VERSION("1.0.0.0");	//版本描述，选填
//modinfo 可查看这些信息

