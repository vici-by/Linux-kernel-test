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



static void rdump_pci_info(struct pci_bus * bus)
{
    struct pci_bus *tmpbus;
    struct pci_dev *tmppci;
	int loop = 0;

    if(bus && bus->children.next){
        list_for_each_entry(tmpbus, &bus->children, node) {
            rdump_pci_info(tmpbus);
        }
    }
    printk("pci bus_name is %s:number=%d,primary=%d\n",bus->name, bus->number,bus->primary);
    list_for_each_entry(tmppci, &bus->devices, bus_list) {
        printk("devfn = %x, vendor=%x,device=%x,class=%x,subven=%x,sub_dev=%x\n",
            tmppci->devfn,tmppci->vendor,tmppci->device,tmppci->class,tmppci->subsystem_vendor,tmppci->subsystem_device);
		
	    for(loop=0;loop<DEVICE_COUNT_RESOURCE; ++loop){
			if(tmppci->resource[loop].flags == 0)
				continue;
	        printk("\t%d:%pR", loop,&tmppci->resource[loop]);
	    }
    }
}


static int __init pcie_test_init(void)
{
    struct pci_bus *bus;


    // pci_device & pci_bus
    printk("pcie_test_init:%s\n",__FUNCTION__);
    printk("\ndump pci_info\n");
    list_for_each_entry(bus, &pci_root_buses, node) {
        rdump_pci_info(bus);
    }

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
