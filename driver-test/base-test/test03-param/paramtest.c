/*
 * DMA Engine test module
 *
 * Copyright (C) 2007 Atmel Corporation
 * Copyright (C) 2013 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#define pr_fmt(fmt) KBUILD_MODNAME " [%s %d]: " fmt, __func__, __LINE__

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

static unsigned int test_buf_size = 16384;
module_param(test_buf_size, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(test_buf_size, "Size of the memcpy test buffer");

static char test_channel[20];
module_param_string(channel, test_channel, sizeof(test_channel),
        S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(channel, "Bus ID of the channel to test (default: any)");

static char test_device[32];
module_param_string(device, test_device, sizeof(test_device),
        S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(device, "Bus ID of the DMA Engine to test (default: any)");

static unsigned int max_channels;
module_param(max_channels, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(max_channels,
        "Maximum number of channels to use (default: all)");

static bool verbose;
module_param(verbose, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(verbose, "Enable \"success\" result messages (default: off)");


static int paramtest_run_set(const char *val, const struct kernel_param *kp);
static int paramtest_run_get(char *val, const struct kernel_param *kp);
static const struct kernel_param_ops run_ops = {
    .set = paramtest_run_set,
    .get = paramtest_run_get,
};
static bool paramtest_run;
module_param_cb(run, &run_ops, &paramtest_run, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(run, "Run the test (default: false)");



/*
 * baiy@inno-MS-7B89:test03-param$ cat /sys/module/paramtest/parameters/run 
 * N
 * root@inno-MS-7B89:test03-param# echo 1 >  /sys/module/paramtest/parameters/run
 * root@inno-MS-7B89:test03-param# cat /sys/module/paramtest/parameters/run
 * Y
 * ......
 *
 */
static int paramtest_run_get(char *val, const struct kernel_param *kp)
{
    return param_get_bool(val, kp);
}

static void paramtest_dump(void)
{

}
static int paramtest_run_set(const char *val, const struct kernel_param *kp)
{
    int ret;

    ret = param_set_bool(val, kp);
    pr_info("pr_set_param is %d,val is %d\n",ret,*((bool *)kp->arg));
    paramtest_dump();
    return ret;
}

static int __init paramtest_init(void)
{
    pr_info("");
    return 0;
}
/* when compiled-in wait for drivers to load first */
late_initcall(paramtest_init);

static void __exit paramtest_exit(void)
{
    pr_info("");
}
module_exit(paramtest_exit);

MODULE_AUTHOR("Yang.bai");
MODULE_LICENSE("GPL v2");
