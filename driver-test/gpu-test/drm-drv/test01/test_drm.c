/*************************************************************************
    > File Name: test_drm.c
    > Author: baiy
    > Mail: baiyang0223@163.com 
    > Created Time: 2021-05-01-23:24:35
    > Func: 
 ************************************************************************/

#include <drm/drmP.h>

static struct drm_device *vkms_dev;

static struct drm_driver vkms_drv = {
	.name			= "vkms",
	.desc			= "Virtual Kernel Mode Setting",
	.date			= "20180514",
	.major			= 1,
	.minor			= 0,
};

static int __init vkms_init(void)
{
	drm_dev_alloc(&vkms_drv, NULL);
	drm_dev_register(vkms_dev, 0);

	return 0;
}


static void __exit vkms_exit(void)
{
	if (vkms_dev) {
		drm_dev_unregister(vkms_dev);
		drm_dev_put(vkms_dev);
	}
}

module_init(vkms_init);
module_exit(vkms_exit);

MODULE_LICENSE("GPL v2");
MODULE_INFO(supported, "Test drm driver");
