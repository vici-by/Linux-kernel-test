#define pr_fmt(fmt) KBUILD_MODNAME " [%s-%d]: " fmt,__func__,__LINE__

#include <linux/init.h>
#include <linux/module.h>

#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <linux/init.h>
#include <linux/aer.h>
#include <linux/dmi.h>


#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/errno.h>
#include <linux/memblock.h>
#include <linux/msi.h>


static void rdump_pci_bus(struct pci_bus * bus)
{
    struct pci_bus *tmpbus;
    struct pci_dev *tmppci;
    struct pci_dev *pdev;
    struct device  *dev;
	int loop = 0;
    int pos;
        
    if(bus && bus->children.next){
        list_for_each_entry(tmpbus, &bus->children, node) {
            printk("pci bus_name is %s:number=%d,primary=%d,%pR\n",\
                bus->name, bus->number,bus->primary,&bus->busn_res);
            rdump_pci_bus(tmpbus);
        }
    }
    return;
}

static void rdump_pci_info(struct pci_bus * bus)
{
    struct pci_bus *tmpbus;
    struct pci_dev *tmppci;
    struct pci_dev *pdev;
    struct device  *dev;
	int loop = 0;
    int pos;

    pr_info("Start dump bus infomation\n");
        
    if(bus && bus->children.next){
        printk("pci bus_name is %s:number=%d,primary=%d\n",bus->name, bus->number,bus->primary);
        list_for_each_entry(tmpbus, &bus->children, node) {
            rdump_pci_info(tmpbus);
        }
    }
    printk("pci bus_name is %s:number=%d,primary=%d\n",bus->name, bus->number,bus->primary);
    list_for_each_entry(pdev, &bus->devices, bus_list) {
        dev = &(pdev->dev);
        printk("devfn = %x, vendor=%x,device=%x,class=%x,subven=%x,sub_dev=%x,irq=%d\n",
            pdev->devfn,pdev->vendor,pdev->device,pdev->class,pdev->subsystem_vendor,pdev->subsystem_device,pdev->irq);
        for(loop=0;loop<DEVICE_COUNT_RESOURCE;++loop){
            if(pdev->resource[loop].flags & IORESOURCE_IO ||  pdev->resource[loop].flags & IORESOURCE_MEM)
                printk("resource %d: %pR\n",loop,&(pdev->resource[loop]));
        }

        #ifdef CONFIG_GENERIC_MSI_IRQ_DOMAIN
		if(dev->msi_domain){
            pr_err("pdve irq_domain name is %s\n",dev->msi_domain->name);
        } else {
            pr_err("pdev irq_domain is NULL\n");
        }
        #endif

#if 0
        #ifdef CONFIG_PCI_IOV
        if (!pci_is_pcie(pdev))
            break;
        pos = pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_SRIOV);
    	if (pos){
			list_for_each_entry(pdev, &pdev->bus->devices, bus_list){
                pr_info("[TMP]pdev is %x,physfn is %d\n",pdev->devfn, pdev->is_physfn);
        		if (pdev->is_physfn){
                    break;       
                }
			}
        	if ((!pdev->is_physfn) && (pci_ari_enabled(pdev->bus))) {
                printk("[TMP]pci bus_name is %s:number=%d,primary=%d\n",bus->name, bus->number,bus->primary);
                pr_info("[TMP]bus self is %pK,ari enable is %d\n",\
                    pdev->bus->self, pdev->bus->self ? bus->self->ari_enabled : 0);
            }
    	}
        #endif
#endif
    }
}

static int pci_cb(struct device * dev, void * data)
{
    struct pci_dev * pdev;
    int loop = 0;

    pr_info("dev is %pK, data is %pK\n",dev,data);
    pdev = to_pci_dev(dev);
    BUG_ON(!pdev);
    printk("devfn = %x, vendor=%x,device=%x,class=%x,subven=%x,sub_dev=%x,irq=%d\n",
        pdev->devfn,pdev->vendor,pdev->device,pdev->class,pdev->subsystem_vendor,pdev->subsystem_device,pdev->irq);
    for(loop=0;loop<DEVICE_COUNT_RESOURCE; ++loop){
		if(pdev->resource[loop].flags == 0)
			continue;
        printk("\t%d:%pR", loop,&pdev->resource[loop]);
	}
    return 0;
}

static void pci_list_bytype(void)
{
    int err;
    pr_info("pci bus type  name is \"%s\"\n",pci_bus_type.name);
    
    err = bus_for_each_dev(&pci_bus_type, NULL, NULL,pci_cb);
    if (err)
		pr_err("dump list failed\n");
    else
        pr_info("dump list OK\n");
}


static int __init pcie_test_init(void)
{
    struct pci_bus *bus;


    // pci_device & pci_bus
    printk("pcie_test_init:%s\n",__FUNCTION__);
#if 0
    printk("\ndump bus_info\n");
    list_for_each_entry(bus, &pci_root_buses, node) {
        printk("ROOT pci bus_name is %s:number=%d,primary=%d,%pR\n",\
            bus->name, bus->number,bus->primary,&bus->busn_res);
        // rdump_pci_bus(bus);
    }
#endif
#if 0
    printk("dump pci_info\n");
    list_for_each_entry(bus, &pci_root_buses, node) {
        rdump_pci_info(bus);
    }

    printk("\ndump pci_info 2:\n");
    // pci_list_bytype();
#endif

#if 1
    
#endif
    return 0;//成功返回0，失败返回错误码
}


static void __exit pcie_test_exit(void)
{
        printk("pcie_test_exit:%s\n",__FUNCTION__);
        return;
}


module_init(pcie_test_init);
module_exit(pcie_test_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("baiy <baiyang0223@163.com>");
MODULE_DESCRIPTION("This is a test driver");
MODULE_VERSION("1.0.0.0");
