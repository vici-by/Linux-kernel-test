/*************************************************************************
    > File Name: hello.c
    > Author: baiy
    > Mail: yang01.bai@hobot.cc 
    > Created Time: 2018-07-27-16:56:21
	> Func: 
 ************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/kerf.h>
#include <linux/refcount.h>

/*
struct kobj_type {                                                                                                                                                             
    void (*release)(struct kobject *kobj);                                                                                                                                     
    const struct sysfs_ops *sysfs_ops;                                                                                                                                         
    struct attribute **default_attrs;                                                                                                                                          
    const struct kobj_ns_type_operations *(*child_ns_type)(struct kobject *kobj);                                                                                              
    const void *(*namespace)(struct kobject *kobj);                                                                                                                            
};
*/
void test_kobj_release(struct kobject * kojb)
{
    
}
struct kobj_type test_type = {
    .release = test_kobj_release;
};

struct test_koject {
    struct koject kobj;
};

struct test_kobject test;

static int __init xxx_init(void)
{
    printk("xx_init:%s\n",__FUNCTION__);
    kobject_set_name(&test.kobj,"test-kojb");

    return 0;//成功返回0，失败返回错误码
}


static void __exit xxx_exit(void)
{
    printk("xxx_exit:%s\n",__FUNCTION__);
    return;
}

module_init(xxx_init);
module_exit(xxx_exit); //修饰入口和出口函数
MODULE_LICENSE("GPL");
MODULE_AUTHOR("baiy <baiyang0223@163.com>"); //作者信息，选填
MODULE_DESCRIPTION("This is a test driver"); //描述信息，选填
MODULE_VERSION("1.0.0.0");	//版本描述，选填


