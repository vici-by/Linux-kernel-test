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


#define XILINX_PCIE_TEST
#ifdef XILINX_PCIE_TEST
#define PCI_VENDOR_PF_ID_DEMO      0x10EE
#define PCI_DEVICE_PF_ID_DEMO      0x9032
#define PCI_VENDOR_VF_ID_DEMO      0x10DE
#define PCI_DEVICE_VF_ID_DEMO      0xABCD
#endif


static struct pci_device_id demo_pcie_table[] = {
    {PCI_DEVICE(PCI_VENDOR_PF_ID_DEMO,PCI_DEVICE_PF_ID_DEMO), },
    { 0, }
};


static int __init pcie_test_init(void)
{
    struct pci_bus *bus;
	struct device *dev;
	struct pci_dev *pdev = NULL;
    int pos;

    pr_info("\n");
    

    pdev = pci_get_subsys(PCI_VENDOR_PF_ID_DEMO,PCI_DEVICE_PF_ID_DEMO, PCI_ANY_ID, PCI_ANY_ID,NULL);
    if( !pdev ){
        pr_info("Not found device\n");
        return 0;
    } else {
        pr_info("devfn = %x, vendor=%x,device=%x,class=%x,subven=%x,sub_dev=%x,irq=%d\n",
            pdev->devfn,pdev->vendor,pdev->device,pdev->class,pdev->subsystem_vendor,pdev->subsystem_device,pdev->irq);
        bus = pdev->bus;
        if(bus){
            pr_info("pci bus_name is %s:number=%d,primary=%d\n",bus->name, bus->number,bus->primary);
        } else {
            pr_err("No bus ???\n");
            return 0;
               
        }
#ifdef CONFIG_PCI_IOV
        if (!pci_is_pcie(pdev)){
            return -1;
        }
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

    }

    return 0;//成功返回0，失败返回错误码
}


static void __exit pcie_test_exit(void)
{
        pr_info("\n");
        return;
}


module_init(pcie_test_init);
module_exit(pcie_test_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("baiy <baiyang0223@163.com>");
MODULE_DESCRIPTION("This is a test driver");
MODULE_VERSION("1.0.0.0");
