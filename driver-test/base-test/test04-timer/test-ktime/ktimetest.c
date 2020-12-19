/*************************************************************************
    > File Name: ktimetest.c
    > Author: baiy
    > Mail: baiyang0223@163.com 
    > Created Time: 2020-12-16-11:12:25
    > Func: 学习dmatest，利用模块参数，启动一个线程，然后随机测试ktime
 ************************************************************************/
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/ktime.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/freezer.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/sched/task.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/wait.h>


static int paramtest_run_get(char *val, const struct kernel_param *kp);
static int paramtest_run_set(const char *val, const struct kernel_param *kp);

static const struct kernel_param_ops run_ops = {
    .set = paramtest_run_set,
    .get = paramtest_run_get,
};
static bool paramtest_run;
module_param_cb(run, &run_ops, &paramtest_run, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(run, "Run the test (default: false)");



static int ktime_test_thread(void *data)
{
    int i = 10;
    struct completion cmpl;
    unsigned int requirment_type = 0;
    ktime_t         start, diff;
    s64         runtime = 0;

    printk("thread starting...\n");
    while((i--) > 0){
        init_completion(&cmpl);
        get_random_bytes(&requirment_type,sizeof(requirment_type));  //生成一个内核随机数
        requirment_type %= 3;  // 随机数范围0-2s
        ++requirment_type;

        start = ktime_get();
        wait_for_completion_timeout(&cmpl, requirment_type * HZ);
        diff = ktime_sub(ktime_get(), start);
        runtime = ktime_to_us(diff);
        pr_info("thread sleep %d,runtime is %lld\n", requirment_type, runtime);
    }
    printk("thread ended!\n");
    paramtest_run=0;

    return 0;
}

static int paramtest_run_get(char *val, const struct kernel_param *kp)
{
    return param_get_bool(val, kp);
}

static int paramtest_run_set(const char *val, const struct kernel_param *kp)
{
    int ret;

    ret = param_set_bool(val, kp);
    pr_info("pr_set_param is %d,val is %d\n",ret,*((bool *)kp->arg));

    kthread_run(ktime_test_thread,NULL,"Ktime_test");
    return ret;
}

static int __init ktimetest_init(void)
{
    return 0;
}
module_init(ktimetest_init);

static void __exit ktimetest_exit(void)
{
}
module_exit(ktimetest_exit);


MODULE_LICENSE("GPL v2");
