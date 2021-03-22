#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>

static int demo_range_probe(struct platform_device *pdev)
{
    struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

    printk(KERN_INFO "%s start: 0x%x, end: 0x%x\n",
        res->name, res->start, res->end + 1);

    return 0;
}

static int demo_range_remove(struct platform_device *pdev)
{
    return 0;
}

static const struct of_device_id demo_range_of_match[]  = {
    { .compatible = "range"},
    {},
};

static struct platform_driver demo_range_driver = {
    .driver = {
        .name = "demo_range",
        .owner = THIS_MODULE,
        .of_match_table = demo_range_of_match,
    },
    .probe = demo_range_probe,
    .remove = demo_range_remove,
};
module_platform_driver(demo_range_driver);

MODULE_LICENSE("GPL v2");
