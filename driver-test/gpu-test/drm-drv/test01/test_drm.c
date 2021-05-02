/*************************************************************************
	> File Name: test_drm.c
	> Author: baiy
	> Mail: baiyang0223@163.com 
	> Created Time: 2021-05-01-23:24:35
	> Func: 
 ************************************************************************/
#define pr_fmt(fmt)  "[%s:%d] " fmt, __func__, __LINE__

#include <drm/drmP.h>
#include <drm/drm_drv.h>
#include <drm/drm_gem.h>
#include <drm/drm_vblank.h>

static struct drm_device *vkms_dev;

static struct drm_driver vkms_drv = {
	.name			= "vkms",
	.desc			= "Virtual Kernel Mode Setting",
	.date			= "20180514",
};

static int __init vkms_init(void)
{
	pr_info("[E]");
	vkms_dev = drm_dev_alloc(&vkms_drv, NULL);
	if (!vkms_dev) {
		pr_err("[vkms] dev alloc failed");
		return -ENOMEM;
	}

	pr_info("[vkms] dev register");
	drm_dev_register(vkms_dev, 0);
	pr_info("[X]\n");

	return 0;
}


static void __exit vkms_exit(void)
{
	pr_info("[E]");
	if (vkms_dev) {
		drm_dev_unregister(vkms_dev);
		drm_dev_put(vkms_dev);
	}
	pr_info("[X]\n");
}

module_init(vkms_init);
module_exit(vkms_exit);

MODULE_LICENSE("GPL v2");
MODULE_INFO(supported, "Test drm driver");
