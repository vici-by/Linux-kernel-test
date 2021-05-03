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

#include <linux/component.h>
#include <linux/time.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 3, 0)
	#include <asm/switch_to.h>
#else
    #include <asm/system.h>
#endif
#include <linux/delay.h>

#include <linux/platform_device.h>


static int dev_0_ops_bind(struct device *dev, struct device *master,
		    void *master_data)
{
	struct platform_device * pdev = to_platform_device(dev);
	int id = pdev->id;
	pr_info("[E] pdev %p, dev=%p, id=%d, master_data = %p\n",pdev, &pdev->dev, id, master_data);
	return 0;
}

static void dev_0_ops_unbind(struct device *dev, struct device *master,
		       void *master_data)
{
	pr_info("[X]");
}

static int dev_1_ops_bind(struct device *dev, struct device *master,
		    void *master_data)
{
	struct platform_device * pdev = to_platform_device(dev);
	int id = pdev->id;
	pr_info("[E] pdev %p, dev=%p, id=%d, master_data = %p\n",pdev, &pdev->dev, id, master_data);
	return 0;
}

static void dev_1_ops_unbind(struct device *dev, struct device *master,
		       void *master_data)
{
	pr_info("[X]");
}

static int dev_2_ops_bind(struct device *dev, struct device *master,
		    void *master_data)
{
	struct platform_device * pdev = to_platform_device(dev);
	int id = pdev->id;
	pr_info("[E] pdev %p, dev=%p, id=%d, master_data = %p\n",pdev, &pdev->dev, id, master_data);
	return 0;
}

static void dev_2_ops_unbind(struct device *dev, struct device *master,
		       void *master_data)
{
	pr_info("[X]");
}

static int dev_3_ops_bind(struct device *dev, struct device *master,
		    void *master_data)
{
	struct platform_device * pdev = to_platform_device(dev);
	int id = pdev->id;
	pr_info("[E] pdev %p, dev=%p, id=%d, master_data = %p\n",pdev, &pdev->dev, id, master_data);
	return 0;
}

static void dev_3_ops_unbind(struct device *dev, struct device *master,
		       void *master_data)
{
	pr_info("[X]");
}

static int dev_4_ops_bind(struct device *dev, struct device *master,
		    void *master_data)
{
	struct platform_device * pdev = to_platform_device(dev);
	int id = pdev->id;
	pr_info("[E] pdev %p, dev=%p, id=%d, master_data = %p\n",pdev, &pdev->dev, id, master_data);
	return 0;
}

static void dev_4_ops_unbind(struct device *dev, struct device *master,
		       void *master_data)
{
	pr_info("[X]");
}



static int comp_master_bind(struct device *master)
{
	struct platform_device * pdev = to_platform_device(master);
	int id = pdev->id;
	pr_info("[E] pdev %p, dev=%p, id=%d\n",pdev, &pdev->dev, id);
	component_bind_all(master,pdev);	// 这个pdev就是其余设备 master_data的数据来源
	pr_info("[X]\n");
	msleep(1000);
	pr_info("[X] OK\n");
	return 0;
}

static void comp_master_unbind(struct device *master)
{
	struct platform_device * pdev = to_platform_device(master);
	int id = pdev->id;
	pr_info("[E] pdev %p, dev=%p, id=%d\n",pdev, &pdev->dev, id);
	component_unbind_all(master,pdev);
	pr_info("[X]");
	return;
}

static const struct component_ops dev_0_ops = {
	.bind 	= dev_0_ops_bind,
	.unbind = dev_0_ops_unbind,
};
static const struct component_ops dev_1_ops = {
	.bind 	= dev_1_ops_bind,
	.unbind = dev_1_ops_unbind,
};
static const struct component_ops dev_2_ops = {
	.bind 	= dev_2_ops_bind,
	.unbind = dev_2_ops_unbind,
};
static const struct component_ops dev_3_ops = {
	.bind 	= dev_3_ops_bind,
	.unbind = dev_3_ops_unbind,
};
static const struct component_ops dev_4_ops = {
	.bind 	= dev_4_ops_bind,
	.unbind = dev_4_ops_unbind,
};

static const struct component_master_ops comp_master_ops = {
	.bind 	= comp_master_bind,
	.unbind = comp_master_unbind,
};



static int comp_dev_0_probe(struct platform_device *pdev)
{
	pr_info("[E] pdev %p, dev=%p, name=%s\n",pdev, &pdev->dev, dev_name(&pdev->dev));
	pr_info("[X]");
	return component_add(&pdev->dev, &dev_0_ops);
}
static int comp_dev_0_remove(struct platform_device *pdev)
{
	pr_info("[E] pdev %p, dev=%p\n",pdev, &pdev->dev);
	pr_info("[X]");
	component_del(&pdev->dev, &dev_0_ops);
	return 0;
}

static int comp_dev_1_probe(struct platform_device *pdev)
{
	pr_info("[E] pdev %p, dev=%p, name=%s\n",pdev, &pdev->dev, dev_name(&pdev->dev));
	pr_info("[X]");
	return component_add(&pdev->dev, &dev_1_ops);
}
static int comp_dev_1_remove(struct platform_device *pdev)
{
	pr_info("[E] pdev %p, dev=%p\n",pdev, &pdev->dev);
	pr_info("[X]");
	component_del(&pdev->dev, &dev_1_ops);
	return 0;
}

static int comp_dev_2_probe(struct platform_device *pdev)
{
	pr_info("[E] pdev %p, dev=%p, name=%s\n",pdev, &pdev->dev, dev_name(&pdev->dev));
	pr_info("[X]");
	return component_add(&pdev->dev, &dev_2_ops);
}
static int comp_dev_2_remove(struct platform_device *pdev)
{
	pr_info("[E] pdev %p, dev=%p\n",pdev, &pdev->dev);
	pr_info("[X]");
	component_del(&pdev->dev, &dev_2_ops);
	return 0;

}

static int comp_dev_3_probe(struct platform_device *pdev)
{
	pr_info("[E] pdev %p, dev=%p, name=%s\n",pdev, &pdev->dev, dev_name(&pdev->dev));
	pr_info("[X]");
	return component_add(&pdev->dev, &dev_3_ops);
}
static int comp_dev_3_remove(struct platform_device *pdev)
{
	pr_info("[E] pdev %p, dev=%p\n",pdev, &pdev->dev);
	pr_info("[X]");
	component_del(&pdev->dev, &dev_3_ops);
	return 0;

}

static int comp_dev_4_probe(struct platform_device *pdev)
{
	pr_info("[E] pdev %p, dev=%p, name=%s\n",pdev, &pdev->dev, dev_name(&pdev->dev));
	pr_info("[X]");
	return component_add(&pdev->dev, &dev_4_ops);
}
static int comp_dev_4_remove(struct platform_device *pdev)
{
	pr_info("[E] pdev %p, dev=%p\n",pdev, &pdev->dev);
	pr_info("[X]");
	component_del(&pdev->dev, &dev_4_ops);
	return 0;

}


static int compare_dev_name(struct device *dev, void *data)
{
	const char *name = data;
	return !strcmp(dev_name(dev), name);
}
static int compare_dev(struct device *dev, void *data)
{
	return (dev == data);
}


struct platform_driver comp_drivers[] = {
	[0] = {.probe = comp_dev_0_probe, .remove = comp_dev_0_remove, .driver = {.name = "COMP_DEV_0"} },
	[1] = {.probe = comp_dev_1_probe, .remove = comp_dev_1_remove, .driver = {.name = "COMP_DEV_1"} },
	[2] = {.probe = comp_dev_2_probe, .remove = comp_dev_2_remove, .driver = {.name = "COMP_DEV_2"} },
	[3] = {.probe = comp_dev_3_probe, .remove = comp_dev_3_remove, .driver = {.name = "COMP_DEV_3"} },
	[4] = {.probe = comp_dev_4_probe, .remove = comp_dev_4_remove, .driver = {.name = "COMP_DEV_4"} },
};


#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 3, 0))
int comp_dev_match_name(struct device *dev, const void *data)
#else
int comp_dev_match_name(struct device *dev, void *data)
#endif

{
	struct platform_device *pdev = to_platform_device(dev);
	return (strcmp(pdev->name, data) == 0);
}

static int comp_master_probe(struct platform_device *pdev)
{
	int	i;
	char buf[128] = {};
	struct component_match	*match = NULL;
	struct platform_driver	*pdrv  = NULL;
	struct device			*dev = &pdev->dev;

	pr_info("[E] pdev %p, dev=%p, name=%s\n",pdev, &pdev->dev, dev_name(&pdev->dev));
	for (i=0; i<ARRAY_SIZE(comp_drivers); ++i) {
		struct device *d;
		pdrv = &comp_drivers[i];
		// bus_find_device_by_name 适合不带id的设备，因为 platform_device_add在有id时会将id追加到设备名上
		// d = bus_find_device_by_name(&platform_bus_type, NULL, comp_drivers[i].driver.name);
		// d = bus_find_device(&platform_bus_type, NULL, &pdrv->driver, (void *)platform_bus_type.match);
		d = bus_find_device(&platform_bus_type, NULL,  pdrv->driver.name, comp_dev_match_name);
		if (!d) {
			pr_info("Can't get device by name : %s\n", pdrv->driver.name);
			continue;
		}
		pr_info("add component %s, initname is %s\n", pdrv->driver.name, d->init_name);
		// 因为dev->initname都为NULL， 所以不适合用 compare_dev_name
		// component_match_add(dev, &match, compare_dev_name, comp_drivers[i].driver.name);
		component_match_add(dev, &match, compare_dev, d);
		put_device(d);
	}

	return component_master_add_with_match(&pdev->dev, &comp_master_ops, match);
}
static int comp_master_remove(struct platform_device *pdev)
{
	pr_info("[E] pdev %p, dev=%p\n",pdev, &pdev->dev);
	component_master_del(&pdev->dev, &comp_master_ops);
	return 0;
}





struct platform_driver comp_master_drivers = {
	.probe = comp_master_probe,
	.remove = comp_master_remove,
	.driver = {.name = "COMP_MASTER"},
};




static int __init comp_master_init(void)
{
	int i = 0;

	pr_info("init [E]\n");
	for (i=0; i<ARRAY_SIZE(comp_drivers); ++i) {
		platform_driver_register(&comp_drivers[i]);
	}
	platform_driver_register(&comp_master_drivers);
	return 0;
}
module_init(comp_master_init);

static void __exit comp_master_exit(void)
{

	int i = 0;

	pr_info("exit [E]\n");
	platform_driver_unregister(&comp_master_drivers);
	for (i=0; i<ARRAY_SIZE(comp_drivers); ++i) {
		platform_driver_unregister(&comp_drivers[i]);
	}
}
module_exit(comp_master_exit);

MODULE_LICENSE("GPL v2");




