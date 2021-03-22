/*************************************************************************
    > File Name: first.c
    > Author: baiy
    > Mail: yang01.bai@hobot.cc 
    > Created Time: 2018年07月11日 星期三 09时54分24秒
	> Func: 测试驱动模块的符号导出和模块参数
 ************************************************************************/
 
#include <linux/init.h>
#include <linux/module.h>
 
static int myint = 1;
static short myshort = 2;
static char * mystr = "hello";
static int myarr[3] = {4,5,6};
 
module_param(myint,int, 0644);
MODULE_PARM_DESC(myint,"test param int");
 
module_param(myshort,short,0);
MODULE_PARM_DESC(myshort,"test param short");
 
module_param(mystr,charp,0644);
MODULE_PARM_DESC(mystr,"test param str");
 
module_param_array(myarr,int, NULL, 0644);
MODULE_PARM_DESC(myarr,"test param arr");

static int __init first_init(void)
{
    printk("%s-%d,test driver init\n",__func__,__LINE__);
    printk("myint = %d, myshort =%d, mystr = %s, myarr[0]=%d\n",
            myint,myshort,mystr,myarr[0]);
    return 0;
}
static void __exit first_exit(void)
{
    printk("myint = %d, myshort =%d, mystr = %s, myarr[0]=%d\n",
            myint,myshort,mystr,myarr[0]);
    printk("%s-%d,test driver exit\n",__func__,__LINE__);
}


		
module_init(first_init);
module_exit(first_exit); //修饰入口和出口函数
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("test driver first");
MODULE_AUTHOR("baiyang <baiyang0223@163.com>");
MODULE_VERSION("1.0.0.0");
