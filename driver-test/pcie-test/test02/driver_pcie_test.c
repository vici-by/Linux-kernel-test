/*************************************************************************
    > File Name: driver_pcie_test.c
    > Author: baiy
    > Mail: baiyang0223@163.com 
    > Created Time: 2020-10-21-09:33:13
    > Func: 
 ************************************************************************/
#define pr_fmt(fmt)  "[%s:%d] " fmt, __func__, __LINE__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/string.h>

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <linux/iommu.h>
#include <linux/msi.h>
#include <linux/mm.h>
#include <linux/random.h>
#include <linux/spinlock.h>

#include <linux/delay.h>
#include <asm-generic/io.h>



#define DEV_NAME "PCIE_DEMO"



#define OPEN_SRIOV
#define OPEN_MSI


#define IOC_MAGIC  'p'
#define PCIE_DEMO_READ_CONFIG       _IO(IOC_MAGIC, 0)
#define PCIE_DEMO_DUMP_RESOURCE     _IO(IOC_MAGIC, 1)
#define PCIE_DEMO_READ_BAR          _IO(IOC_MAGIC, 2)
#define PCIE_DEMO_WRITE_BAR         _IO(IOC_MAGIC, 3)
#define PCIE_BAR_STABLE_TEST        _IO(IOC_MAGIC, 4)



#define XILINX_PCIE_TEST
#ifdef XILINX_PCIE_TEST
#define PCI_VENDOR_PF_ID_DEMO      0x10EE
#define PCI_DEVICE_PF_ID_DEMO      0x9032
#define PCI_VENDOR_VF_ID_DEMO      0x10DE
#define PCI_DEVICE_VF_ID_DEMO      0xABCD
#endif

#define REQ_ACS_FLAGS   (PCI_ACS_SV | PCI_ACS_RR | PCI_ACS_CR | PCI_ACS_UF)


struct pci_demo_bar {
    resource_size_t base_addr;
    void __iomem *virt_addr;
    resource_size_t size;
    unsigned long flags;
} __packed;

/* priv_flags */
#define DEMO_SRIOV_ENABLED      (1 << 0)
#define DEMO_MSI_ENABLED        (1 << 1)

struct pcie_demo_data {
    u16    num_vfs;
    unsigned int priv_flags;
    struct pci_dev * pcidev;
    struct pci_demo_bar bar[DEVICE_COUNT_RESOURCE];
    struct mutex mlock;
    spinlock_t slock;
#ifdef CONFIG_PCI_MSI
    atomic64_t irq_nums;
    struct irq_domain *dom;
#endif
};

struct pcie_demo_data  * pcie_data;


static ssize_t pcie_demo_read (struct file * filp, char __user * buf, size_t size, loff_t * offset)
{
    return 0;
}
static ssize_t pcie_demo_write (struct file * filp, const char __user * buf, size_t size, loff_t * offset)
{

    return 0;
}



struct pci_demo_opt {
    unsigned long barnum;
    unsigned long len;
    unsigned long addr; // count
    unsigned long offset;
};
static long pcie_demo_ioctl (struct file * filp, unsigned int cmd, unsigned long args)
{
    int    ret;
    char * tmpbuf;
    unsigned long loop;
    unsigned long val;
    int barnum;
    struct pci_demo_opt user_opt;
    struct pci_demo_opt * opt;
    struct pci_demo_bar * pbar;
    struct resource * r;
    struct pci_dev *pdev;
    struct pcie_demo_data *pdata;


    pdata = (struct pcie_demo_data *)filp->private_data;

    pdev  = pdata->pcidev;

    if(args) {
        opt = (struct pci_demo_opt * )args;
        ret = copy_from_user(&user_opt, (void*)args, sizeof(struct pci_demo_opt));
        if(ret != 0){
            pr_err("copy from user ret %d failed\n",ret);
            return -EINVAL;
        }
        opt = &user_opt;
    }
    switch(cmd){
        case PCIE_DEMO_READ_CONFIG:
            tmpbuf = kmalloc(0x40, GFP_KERNEL);
            if(!tmpbuf){
                pr_err("short of memory");
                return -ENOMEM;
            }
            memset(tmpbuf,0,0x40);
            for(loop=0; loop<0x40; ++loop){
                pci_read_config_byte(pdev,loop,tmpbuf+loop);
            }
            if(copy_to_user(( void*)opt->addr, tmpbuf,0x40) != 0){
                pr_err("copy to user failed\n");
                kfree(tmpbuf);
                return -1;
            }
            kfree(tmpbuf);
            break;
        case PCIE_DEMO_DUMP_RESOURCE:
            r = pdev->resource;
            for(loop=0;loop<DEVICE_COUNT_RESOURCE; ++loop){
                printk("resource: start:%#llx,end:%#llx,name:%s,flags:%#lx,desc:%#lx\n",
                                r->start,r->end,r->name,r->flags,r->desc);
                r++;
            }
            break;
        case PCIE_DEMO_READ_BAR:
            barnum = opt->barnum;
            pbar = &(pdata->bar[opt->barnum]);
            if(pbar->virt_addr == NULL) {
                pr_err("Is not a IO/MEMORY BAR");
                return -EINVAL;
            }
            tmpbuf = kmalloc(opt->len, GFP_KERNEL);
            if(!tmpbuf){
                pr_err("short of memory");
                return -ENOMEM;
            }
            memset(tmpbuf,0,opt->len);

            if(((pbar->flags) & IORESOURCE_IO)) {
                for(loop=0; loop<opt->len; loop+=4){
                    ((unsigned int*)tmpbuf)[loop] = ioread32(pbar[loop].virt_addr + opt->offset + loop);
                }
                val = copy_to_user((void*)opt->addr, tmpbuf,opt->len*4);
                if(val != 0){
                    pr_err("copy to user failed\n");
                    kfree(tmpbuf);
                    return -1;
                }
            }else if(((pbar->flags) & IORESOURCE_MEM)) {
                for(loop=0; loop<opt->len; ++loop){
                    tmpbuf[loop] = ioread8(pbar->virt_addr + opt->offset + loop);
                }
                val = copy_to_user((void *)opt->addr, tmpbuf,opt->len);
                if(val != 0){
                    pr_err("copy to user failed\n");
                    kfree(tmpbuf);
                    return -1;
                }
            }
            break;
        case PCIE_DEMO_WRITE_BAR:
            pbar = &(pdata->bar[opt->barnum]);
            if(pbar->virt_addr == NULL) {
                pr_err("Is not a IO/MEMORY BAR");
                return -EINVAL;
            }
            tmpbuf = kmalloc(opt->len, GFP_KERNEL);
            if(!tmpbuf){
                pr_err("short of memory");
                return -ENOMEM;
            }
            memset(tmpbuf,0,opt->len);
            val = copy_from_user(tmpbuf, (void *)opt->addr, opt->len);
            if(val != 0){
                pr_err("copy to user failed\n");
                kfree(tmpbuf);
                return -1;
            }
            if(((pbar->flags) & IORESOURCE_IO)) {
                for(loop=0; loop<opt->len; loop+=4){
                    iowrite32(((unsigned int*)tmpbuf)[loop] , pbar[loop].virt_addr + opt->offset + loop);
                }
            }else if(((pbar->flags) & IORESOURCE_MEM)) {
                for(loop=0; loop<opt->len; ++loop){
                    iowrite8((tmpbuf)[loop] & 0xff, pbar->virt_addr + opt->offset + loop);
                }
            }
            break;
        case PCIE_BAR_STABLE_TEST:
            {
            int * pb1;
            int * pb2;
            char * tmpbuf2;
            unsigned long cnt;
            unsigned long i;

            pbar = &(pdata->bar[opt->barnum]);
            if(pbar->virt_addr == NULL) {
                pr_err("Is not a IO/MEMORY BAR");
                return -EINVAL;
            }
            opt->len = round_up(opt->len, PAGE_SIZE);
            pr_info("offset is %ld, len is %ld\n",opt->offset,opt->len);
            tmpbuf = (void *)__get_free_page(GFP_KERNEL);
            tmpbuf2 = (void *)__get_free_page(GFP_KERNEL);
            if(!tmpbuf || !tmpbuf2){
                pr_err("short of memory");
                return -ENOMEM;
            }
            for(cnt=0;cnt<opt->addr;++cnt){
                for(loop = 0; loop < opt->len; loop+=PAGE_SIZE){
                    memset(tmpbuf2,0,PAGE_SIZE);
                    get_random_bytes_wait(tmpbuf,PAGE_SIZE);
                    #if 0
                    memcpy_toio(pbar->virt_addr +  opt->offset + loop,tmpbuf,PAGE_SIZE);
                    memcpy_fromio(tmpbuf2,pbar->virt_addr +  opt->offset + loop,PAGE_SIZE);
                    #else
                    pb1 = (int *)tmpbuf;
                    pb2 = (int *)tmpbuf2;
                    for(i=0;i<PAGE_SIZE;i+=4){
                        iowrite32(pb1[i/4], pbar->virt_addr +  opt->offset + loop + i);
                    }
                    for(i=0;i<PAGE_SIZE;i+=4){
                        pb2[i/4] = ioread32(pbar->virt_addr +  opt->offset + loop + i);
                    }
                    #endif
                    if(memcmp(tmpbuf,tmpbuf2,PAGE_SIZE)){
                        pr_info("[bar %ld offset %lu count %ld] test FAILED\n",opt->barnum,loop+opt->offset,cnt);
                        pr_info("%#x-%#x\n",((int *)tmpbuf)[0], ((int *)tmpbuf2)[0]);
                        break;
                    } else {
                        pr_info("[bar %ld offset %lu count %ld] test OK\n",opt->barnum,loop+opt->offset,cnt);
                    }
                }
                if(loop < opt->len)
                    break;
            }
            free_page((unsigned long)tmpbuf);
            free_page((unsigned long)tmpbuf2);
            }
            break;
        default:
            pr_err("Not support command");
            return -1;
    }
    return 0;
}
static int pcie_demo_open (struct inode * inode, struct file * filp)
{
    struct pcie_demo_data *pdata = pcie_data;
    filp->private_data = pdata;
    return 0;
}
static int pcie_demo_release (struct inode * inode, struct file * filp)
{
    filp->private_data = NULL;
    return 0;
}


static struct file_operations pcie_demo_fops = {
    .owner = THIS_MODULE,
    .open  = pcie_demo_open,
    .release = pcie_demo_release,
    .read  = pcie_demo_read,
    .write = pcie_demo_write,
    .unlocked_ioctl = pcie_demo_ioctl,
    .compat_ioctl   = pcie_demo_ioctl,
};

static struct miscdevice pcie_demo_drv = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = DEV_NAME,
    .fops  = &pcie_demo_fops,
};

#ifdef OPEN_MSI
#ifdef CONFIG_PCI_MSI

static irqreturn_t demo_pci_msi_isr(int irq, void * data)
{
    unsigned long flags;
    struct pcie_demo_data *pdata = (struct pcie_demo_data *)data;
    spin_lock_irqsave(&(pdata->slock), flags);
    atomic64_inc(&(pdata->irq_nums));
    pr_info("recv irq-%lu", atomic64_read(&(pdata->irq_nums)));
    spin_unlock_irqrestore(&(pdata->slock), flags);

    return IRQ_HANDLED;
}

#endif
#endif

int  demo_pcie_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
    struct pci_bus *bus;
    struct pci_ops *ops;
    struct resource * r;
    struct pcie_demo_data *pdata;
    int loop = 0;
    int err = -EINVAL;
    struct pci_demo_bar * pbar;
    struct device * dev;



    struct iommu_group *group = NULL;
#ifdef OPEN_SRIOV
#ifdef CONFIG_PCI_IOV
    int pos = 0;
    u16 num_vfs = 0;
#endif
#endif
    pr_info("prbe [E] 20201225 1109\n");
    dev = &(pdev->dev);

    pcie_data = kzalloc(sizeof(struct pcie_demo_data), GFP_KERNEL);
    if(!pcie_data){
        pr_err("failed alloc pcie_demo_data\n");
        return (-1);
    }
    pdata = pcie_data;

    mutex_init(&(pdata->mlock));
    spin_lock_init(&(pdata->slock));
    dev_set_drvdata(dev, pdata);
    pdata->pcidev = pdev;
    pbar = pdata->bar;

#ifdef OPEN_SRIOV
#ifdef CONFIG_PCI_IOV 
    /* Get number of subvnics */
    pos = pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_SRIOV);
    if (pos) {
        pci_read_config_word(pdev, pos + PCI_SRIOV_TOTAL_VF, &num_vfs);
        if (num_vfs) {
            err = pci_enable_sriov(pdev, num_vfs);
            if (err) {
                pr_err("SRIOV enable failed, aborting."
                    " pci_enable_sriov() returned %d\n",
                    err);
                err = -1;
                goto err_sriov;
            }
            pr_err("num_vfs is %d\n",num_vfs);
            pdata->num_vfs = num_vfs;
            pdata->priv_flags |= DEMO_SRIOV_ENABLED;
        }
    }
#endif
#endif

    /* Enable PCIE device */
    if (pci_enable_device(pdev)){
        pr_err("pci_enable_device failed\n");
        err = -EIO;
        goto err_enable;
    }

    err = pci_request_regions(pdev, "demo_pci_test");
    if(err < 0){
        printk("ERROR: pci_enable_device\n");
        goto err_regions;
    }
    pci_set_master(pdev);

    bus = pdev->bus;
    ops = bus->ops;

    r = pdev->resource;

    for(loop=0;loop<DEVICE_COUNT_RESOURCE; ++loop){
        printk("res:%d  %pR\n",loop,r);
        r++;
    }

    for(loop=0;loop<DEVICE_COUNT_RESOURCE; ++loop){
        pbar[loop].base_addr = pci_resource_start(pdev, loop);
        pbar[loop].size = pci_resource_len(pdev, loop);
        pbar[loop].flags = pci_resource_flags(pdev, loop);
        if(((pbar[loop].flags) & IORESOURCE_IO) || ((pbar[loop].flags) & IORESOURCE_MEM))  {
            pbar[loop].virt_addr = pci_iomap(pdev, loop, pbar[loop].size);
            pr_info("loop:%d, base=%#llx,size=%#llx,vaddr=%pK\n",loop, pbar[loop].base_addr, \
				pbar[loop].size, pbar[loop].virt_addr);
        }
    }
#ifdef OPEN_MSI
#ifdef CONFIG_PCI_MSI
    do{
        if(pdata->priv_flags & DEMO_MSI_ENABLED){
            pr_info("MSI is enabled\n");
            break;
        }

        //Start MSI method interrupt
        dev_info(dev,"befor enable msi IRQ = %d\n",pdev->irq);
        atomic64_set(&(pdata->irq_nums), 0);
        /* Enable MSI/MSI-X */
        err = pci_alloc_irq_vectors(pdev, 1,1, PCI_IRQ_MSI | PCI_IRQ_MSIX);
        if (err < 0) {
            pr_err( "Request for msi vectors failed,not support msi\n");
            break;
        }

        // pdata->dom = pci_msi_get_device_domain(pdev);
        pdata->dom = dev->msi_domain;
        if(pdata->dom){
            pr_err("pdve irq_domain name is %s\n",pdata->dom->name);
        } else {
            pr_err("pdev irq_domain is NULL\n");
        }
        
        /* Register mailbox interrupt handlers */
        err = request_irq(pci_irq_vector(pdev, 0),
                  demo_pci_msi_isr, 0, DEV_NAME, pdata);
        if (err){
            pr_err("Not Support MSI interrupt\n");
            pci_free_irq_vectors(pdev);
        }
        pdata->priv_flags |= DEMO_MSI_ENABLED;
        pr_info("MSI  enabled\n");
        dev_info(dev,"after enable msi IRQ = %d\n",pdev->irq);
        break;
    }while(1);
#endif
#endif

#if 0
#define TEST_IOMMU_GROUP 
#ifdef TEST_IOMMU_GROUP

    for (bus = pdev->bus; !pci_is_root_bus(bus); bus = bus->parent) {
        pr_info("Current bus num is %d\n", bus->number);
        if (!bus->self)
            continue;


        pdev = bus->self;

        group = iommu_group_get(&pdev->dev);
        if (group) {
            pr_info("Find iommu groub \n");
            break;
        }
        
    }
#endif
#endif
    pr_info("prbe [X]\n");
    return 0;

    pci_release_regions (pdev);
err_regions:
    pci_disable_device (pdev);
err_enable:
#ifdef OPEN_SRIOV
#ifdef CONFIG_PCI_IOV
    if (pdata->priv_flags & DEMO_SRIOV_ENABLED ) {
        pci_disable_sriov(pdev);
        pdata->priv_flags &= ~DEMO_SRIOV_ENABLED;
    }
#endif
#endif
err_sriov:
    dev_set_drvdata(dev, NULL);
    kfree(pcie_data);
    pcie_data = NULL;

    return err;
}


void demo_pcie_remove(struct pci_dev *pdev)
{
    struct pci_demo_bar * pbar;
    struct device * dev;
    int loop = 0;
    struct pcie_demo_data *pdata;

    pr_info("remove [E]\n");

    dev = &(pdev->dev);
    pdata = (struct pcie_demo_data *)dev_get_drvdata(dev);

#ifdef OPEN_MSI
#ifdef CONFIG_PCI_MSI
    do{
        if(!(pdata->priv_flags & DEMO_MSI_ENABLED)) {
            pr_info("MSI is not enabled\n");
            break;
        }

        free_irq(pci_irq_vector(pdev, 0), pdata);
        pci_free_irq_vectors(pdev);
        pdata->priv_flags &= ~DEMO_MSI_ENABLED;
        atomic64_set(&(pdata->irq_nums), 0);
        break;
    }while(1);
#endif
#endif

#ifdef OPEN_SRIOV
#ifdef CONFIG_PCI_IOV
    if (pdata->priv_flags & DEMO_SRIOV_ENABLED ) {
        pci_disable_sriov(pdev);
        pdata->priv_flags &= ~DEMO_SRIOV_ENABLED;
    }
#endif
#endif
    pbar = pdata->bar;
    for(loop=0;loop<DEVICE_COUNT_RESOURCE; ++loop){
        if(pbar[loop].virt_addr)  {
            pci_iounmap(pdev,pbar[loop].virt_addr);
        }
    }

    pci_release_regions (pdev);
    pci_disable_device (pdev);

    dev_set_drvdata(dev, NULL);
    kfree(pcie_data);
    pcie_data = NULL;

    pr_info("remove [X]\n");
}

#ifdef OPEN_SRIOV
#ifdef CONFIG_PCI_IOV
static int demo_pcie_sriov_configure(struct pci_dev *pdev, int numvfs)
{
    struct device * dev;
    struct pcie_demo_data *pdata;
    u16 num_vfs = 0;
    int pos = 0;
    int err = 0;

    pr_info("sriov config [E]\n");

    dev = &(pdev->dev);
    pdata = (struct pcie_demo_data *)dev_get_drvdata(dev);

    if(numvfs > 0) {
        /* Get number of subvnics */
        pos = pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_SRIOV);
        if (pos) {
            pci_read_config_word(pdev, pos + PCI_SRIOV_TOTAL_VF, &num_vfs);
            if (num_vfs) {
                err = pci_enable_sriov(pdev, num_vfs);
                if (err) {
                    pr_err("SRIOV enable failed, aborting."
                        " pci_enable_sriov() returned %d\n",
                        err);
                    return -1;
                }
                pr_err("num_vfs is %d\n",num_vfs);
                pdata->num_vfs = num_vfs;
                pdata->priv_flags |= DEMO_SRIOV_ENABLED;
            }
        }

    } else {
        if (pdata->priv_flags & DEMO_SRIOV_ENABLED ) {
            pci_disable_sriov(pdev);
            pdata->priv_flags &= ~DEMO_SRIOV_ENABLED;
        }
    }
    pr_info("sriov config [X]\n");

    return 0;
}
#endif
#endif


static struct pci_device_id demo_pcie_table[] = {
    {PCI_DEVICE(PCI_VENDOR_PF_ID_DEMO,PCI_DEVICE_PF_ID_DEMO), },
    {PCI_DEVICE(PCI_VENDOR_VF_ID_DEMO,PCI_DEVICE_VF_ID_DEMO), },
    { 0, }
};
MODULE_DEVICE_TABLE(pci, demo_pcie_table);

static struct pci_driver  demo_pcie_driver  = {
    .name       = "demo-pcie",
    .id_table   = demo_pcie_table,
    .probe      = demo_pcie_probe,
    .remove     = demo_pcie_remove,
#ifdef OPEN_SRIOV
#ifdef CONFIG_PCI_IOV
    .sriov_configure = demo_pcie_sriov_configure,
#endif
#endif
};


static int __init pcie_demo_init(void)
{
    pr_info("init [E]\n");
    misc_register(&pcie_demo_drv);
    return pci_register_driver(&demo_pcie_driver);
}

static void __exit pcie_demo_exit(void)
{
    pr_info("exit [E]\n");
    misc_deregister(&pcie_demo_drv);
    pci_unregister_driver(&demo_pcie_driver);
}

module_init(pcie_demo_init);
module_exit(pcie_demo_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("baiy <baiyang0223@163.com>");
MODULE_DESCRIPTION("TEST PCIE_DRIVER for GPU");
MODULE_VERSION("1.0.0.0");
