#include <linux/init.h>
#include <linux/module.h>
 
static int __init hello_init(void)
{
    printk("hello_init:%s\n",__FUNCTION__);
    return 0;// 成功返回0，失败返回错误码
}

static void __exit hello_exit(void)
{
    printk("hello_exit:%s\n",__FUNCTION__);
    return;
}

// 修饰入口和出口函数
module_init(hello_init);
module_exit(hello_exit); 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("baiy <baiyang0223@163.com>"); // 作者信息，选填
MODULE_DESCRIPTION("This is a test driver"); // 描述信息，选填
MODULE_VERSION("1.0.0.0");	// 版本描述，选填