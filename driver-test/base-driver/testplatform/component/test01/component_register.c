/*************************************************************************
    > File Name: component_register.c
    > Author: baiy
    > Mail: baiyang0223@163.com
    > Created Time: 2020-10-21-09:33:13
    > Func: 模拟注册5个设备
 ************************************************************************/
#define pr_fmt(fmt)  "[%s:%d] " fmt, __func__, __LINE__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#include <linux/miscdevice.h>
#include <linux/time.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 3, 0)
	#include <asm/switch_to.h>
#else
    #include <asm/system.h>
#endif
#include <linux/timekeeping.h>

#include <linux/platform_device.h>


struct platform_device  comp_devices[] = {
	[0] = {.name = "COMP_DEV_0", .id = 0},
	[1] = {.name = "COMP_DEV_1", .id = 1},
	[2] = {.name = "COMP_DEV_2", .id = 2},
	[3] = {.name = "COMP_DEV_3", .id = 3},
	[4] = {.name = "COMP_DEV_4", .id = 4},
	[5] = {.name = "COMP_MASTER", .id = 5},
};


static int __init comp_reg_init(void)
{
	int i = 0;

	pr_info("init [E]\n");
	for (i=0; i<ARRAY_SIZE(comp_devices); ++i) {
		platform_device_register(&comp_devices[i]);
	}
	return 0;
}
module_init(comp_reg_init);

static void __exit comp_reg_exit(void)
{
	int i = 0;

	pr_info("exit [E]\n");
	for (i=0; i<ARRAY_SIZE(comp_devices); ++i) {
		platform_device_unregister(&comp_devices[i]);
	}
}
module_exit(comp_reg_exit);

MODULE_LICENSE("GPL v2");



