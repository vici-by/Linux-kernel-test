/*************************************************************************
    > File Name: first.c
    > Author: baiy
    > Mail: yang01.bai@hobot.cc 
    > Created Time: 2018年07月11日 星期三 09时54分24秒
	> Func: 测试驱动模块的符号导出和模块参数
 ************************************************************************/
 
#include <linux/init.h>
#include <linux/module.h>
 

int extern_num = 0x5A5A5A5A;
EXPORT_SYMBOL_GPL(extern_num);

void print_hello(void)
{
	printk("hello this is a module_extern func\n");
}
EXPORT_SYMBOL_GPL(print_hello);
void print_bye(void)
{
	printk("bye this is a module_extern func\n");
}
EXPORT_SYMBOL_GPL(print_bye);

void print_extern(void)
{
	printk("Extern num is %#x\n", extern_num);
}
EXPORT_SYMBOL_GPL(print_extern);

static int __init first_init(void)
{
    printk("%s-%d,test driver init - symbol is %#x\n",__func__,__LINE__, extern_num);
    return 0;
}
static void __exit first_exit(void)
{
    printk("%s-%d,test driver exit - symbol is %#x\n",__func__,__LINE__, extern_num);
}


		
module_init(first_init);
module_exit(first_exit); //修饰入口和出口函数
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("test driver first: symbol export");
MODULE_AUTHOR("baiyang <baiyang0223@163.com>");
MODULE_VERSION("1.0.0.0");
