/*************************************************************************
    > File Name: driver_pcie_test.c
    > Author: baiy
    > Mail: baiyang0223@163.com 
    > Created Time: 2020-10-21-09:33:13
    > Func: 测试使用CBDMA进行数据传输
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

// test dma
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/freezer.h>
#include <linux/kthread.h>
#include <linux/sched/task.h>
#include <linux/moduleparam.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/wait.h>
unsigned long s_vaddr;
unsigned long d_vaddr;
dma_addr_t s_baddr;
dma_addr_t d_baddr;

struct dma_chan * dma_chan;
struct dma_device	*dma_dev;
struct dma_async_tx_descriptor *desc;
struct dma_slave_config dma_config;
dma_cookie_t cookie;
dma_cap_mask_t mask;
struct dma_tx_state state;
enum dma_status st;


#define DEV_NAME "PCIE_DEMO"


#define IOC_MAGIC  'p'
#define PCIE_DEMO_READ_CONFIG       _IO(IOC_MAGIC, 0)
#define PCIE_DEMO_DUMP_RESOURCE     _IO(IOC_MAGIC, 1)
#define PCIE_DEMO_READ_BAR          _IO(IOC_MAGIC, 2)
#define PCIE_DEMO_WRITE_BAR         _IO(IOC_MAGIC, 3)
#define PCIE_DEMO_HDMA_READ_BAR     _IO(IOC_MAGIC, 4) // host dma
#define PCIE_DEMO_HDMA_WRITE_BAR    _IO(IOC_MAGIC, 5)
#define PCIE_DEMO_EDMA_READ_BAR     _IO(IOC_MAGIC, 6) // device dma
#define PCIE_DEMO_EDMA_WRITE_BAR    _IO(IOC_MAGIC, 7)
#define PCIE_CBDMA_DDR_2_DDR        _IO(IOC_MAGIC, 8)
#define PCIE_CBDMA_DDR_2_BAR        _IO(IOC_MAGIC, 9)




#define PCI_VENDOR_PF_ID_DEMO      0x10EE
#define PCI_DEVICE_PF_ID_DEMO      0x9032
#define PCI_VENDOR_VF_ID_DEMO      0x10DE
#define PCI_DEVICE_VF_ID_DEMO      0xABCD

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
#ifdef CONFIG_PCI_MSI
    atomic64_t irq_nums;
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



static void test_dma_cb(void *arg)
{
    pr_info("transmit once cb\n");
}

struct pci_demo_opt {
    unsigned long barnum;
    unsigned long len;
    unsigned long addr;
    unsigned long offset;
};
static long pcie_demo_ioctl (struct file * filp, unsigned int cmd, unsigned long args)
{
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
    struct device * dev;


    pdata = (struct pcie_demo_data *)filp->private_data;

    pdev  = pdata->pcidev;

    dev = &(pdev->dev);
   
    if(args){
        copy_from_user(&user_opt, (void*)args, sizeof(struct pci_demo_opt));
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
        case PCIE_DEMO_HDMA_READ_BAR:
        case PCIE_DEMO_HDMA_WRITE_BAR:
        case PCIE_DEMO_EDMA_READ_BAR:
        case PCIE_DEMO_EDMA_WRITE_BAR:
            break;
        case PCIE_CBDMA_DDR_2_DDR: //  单纯测试CBDMA的DDR到DDR功能

            // 1.申请源Buffer和目标Buffer，获取总线地址
#ifdef DMA_SINGLE
            s_vaddr = __get_free_pages(GFP_KERNEL,get_order(1024));
            d_vaddr = __get_free_pages(GFP_KERNEL,get_order(1024));
            if(s_vaddr == 0 || d_vaddr == 0){
                goto err2;
            }
            s_baddr = dma_map_single(dev,(void*)s_vaddr,1024, DMA_BIDIRECTIONAL);
            d_baddr = dma_map_single(dev,(void*)d_vaddr,1024, DMA_BIDIRECTIONAL);
            pr_info("map dma buffer OK, s_v=%pK,s_b=%llx,d_v=%pK,d_b=%llx\n", (void *)s_vaddr,s_baddr,(void *)d_vaddr,d_baddr);
        
#else
            s_vaddr = (unsigned long)dma_alloc_coherent(dev,1024,&s_baddr, GFP_KERNEL);
            d_vaddr = (unsigned long)dma_alloc_coherent(dev,1024,&d_baddr, GFP_KERNEL);
            if(s_vaddr == 0 || d_vaddr == 0){  
                pr_info("Alloc s_vaddr or d_vaddr buffer failed\n");
                goto err2;
            } 
            pr_info("coherent dma buffer OK, s_v=%pK,s_b=%llx,d_v=%pK,d_b=%llx\n", \
                    (void *)s_vaddr,s_baddr,(void *)d_vaddr,d_baddr);
#endif
            memset((void*)s_vaddr,0x5A,1024);
            memset((void*)d_vaddr,0x0,1024);

            // 2.申请Dma通道,随便哪一个都可以
            dma_cap_zero(mask);
            dma_cap_set(DMA_MEMCPY, mask);
            dma_chan = dma_request_channel(mask, NULL, NULL);
            if(!dma_chan){
                pr_err("dma request failed\n");
                goto err2;
            }  else {
                dma_dev = dma_chan->device;
                pr_info("dma %s,Support %u count ",dma_chan_name(dma_chan),dma_dev->chancnt);
                if (dma_has_cap(DMA_MEMCPY, dma_dev->cap_mask)) {
                    pr_info("\tSupport DMA_MEMCPY");
                }
        
                if (dma_has_cap(DMA_MEMSET, dma_dev->cap_mask)) {
                    pr_info("\tSupport DMA_MEMSET");
                }
                pr_info("dma dev name is %s,config is %pK,tx status %pK\n",\
                    dma_dev->dev->init_name,dma_dev->device_config,dma_dev->device_tx_status);
            }

            
#if 0       // not supported
            pr_info("Start config dma\n");
            dma_config.direction = DMA_MEM_TO_MEM;
            dma_config.src_addr = s_baddr;
            dma_config.dst_addr = d_baddr;
            dma_config.src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
            dma_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
            dma_config.src_maxburst = 1024;
            dma_config.dst_maxburst = 1024;
            dma_config.device_fc = false;
            dmaengine_slave_config(dma_chan, &dma_config);
        
            pr_info("Start prep dma\n");
            desc = dmaengine_prep_dma_cyclic(dma_chan, s_baddr, 0x1024, 0x1024, DMA_MEM_TO_MEM,DMA_PREP_INTERRUPT );
#endif
            desc = dmaengine_prep_dma_memcpy(dma_chan,
                                     d_baddr, s_baddr, 1024, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
        
            pr_info("Start  dma\n");
            desc->callback = test_dma_cb;
            desc->callback_param = NULL;
            cookie = dmaengine_submit(desc);
        
            loop = 3;
            while(loop) {
                st = dmaengine_tx_status(dma_chan,cookie,&state);
                pr_info("st is %d\n", st);
                if(st == DMA_COMPLETE)
                    break;
                msleep(10);
                --loop;
            }
            if(memcmp((void*)s_vaddr,(void*)d_vaddr, 1024)){
                pr_err("dma transmit failed");
            } else {
                pr_err("dma transmit OK");
            }

        	if(!dma_chan){
        		dma_release_channel(dma_chan); 
        		dma_chan = NULL;
            }
err2:
#ifdef DMA_SINGLE
            if(s_vaddr) {
                dma_unmap_single(dev,s_vaddr,1024, DMA_BIDIRECTIONAL);
                free_pages(s_vaddr, get_order(1024));
            }
            if(d_vaddr) {
                dma_unmap_single(dev,d_vaddr,1024, DMA_BIDIRECTIONAL);
                free_pages(d_vaddr, get_order(1024));
            }
#else
            if(s_vaddr)
                dma_free_coherent(dev, 1024, (void *)s_vaddr, s_baddr);
            if(d_vaddr)
                dma_free_coherent(dev, 1024, (void *)d_vaddr, d_baddr);
#endif
        	s_vaddr = 0;
        	d_vaddr = 0;


            break;
        case PCIE_CBDMA_DDR_2_BAR:
            pbar = &(pdata->bar[2]);
            s_vaddr = __get_free_pages(GFP_KERNEL,get_order(1024));
            d_vaddr = (unsigned long)pbar->virt_addr;
            if(s_vaddr == 0 || d_vaddr == 0){
                goto err3;
            }
            s_baddr = dma_map_single(dev,(void*)s_vaddr,1024, DMA_BIDIRECTIONAL);
            d_baddr = dma_map_single(dev,(void*)d_vaddr,1024, DMA_BIDIRECTIONAL);  // ??? pbar->start可以不
            pr_info("map dma buffer OK, s_v=%pK,s_b=%llx,d_v=%pK,d_b=%llx\n", \
                (void *)s_vaddr,s_baddr,(void *)d_vaddr,d_baddr);
            memset((void*)s_vaddr,0x5A,1024);
            memset((void*)d_vaddr,0x0,1024);

            // 2.申请Dma通道,随便哪一个都可以
            dma_cap_zero(mask);
            dma_cap_set(DMA_MEMCPY, mask);
            dma_chan = dma_request_channel(mask, NULL, NULL);
            if(!dma_chan){
                pr_err("dma request failed\n");
                goto err2;
            }  else {
                dma_dev = dma_chan->device;
                pr_info("dma %s,Support %u count ",dma_chan_name(dma_chan),dma_dev->chancnt);
                if (dma_has_cap(DMA_MEMCPY, dma_dev->cap_mask)) {
                    pr_info("\tSupport DMA_MEMCPY");
                }
        
                if (dma_has_cap(DMA_MEMSET, dma_dev->cap_mask)) {
                    pr_info("\tSupport DMA_MEMSET");
                }
                pr_info("dma dev name is %s,config is %pK,tx status %pK\n",\
                    dma_dev->dev->init_name,dma_dev->device_config,dma_dev->device_tx_status);
            }

            
#if 0       // not supported
            pr_info("Start config dma\n");
            dma_config.direction = DMA_MEM_TO_MEM;
            dma_config.src_addr = s_baddr;
            dma_config.dst_addr = d_baddr;
            dma_config.src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
            dma_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
            dma_config.src_maxburst = 1024;
            dma_config.dst_maxburst = 1024;
            dma_config.device_fc = false;
            dmaengine_slave_config(dma_chan, &dma_config);
        
            pr_info("Start prep dma\n");
            desc = dmaengine_prep_dma_cyclic(dma_chan, s_baddr, 0x1024, 0x1024, DMA_MEM_TO_MEM,DMA_PREP_INTERRUPT );
#endif
            desc = dmaengine_prep_dma_memcpy(dma_chan,
                                     d_baddr, s_baddr, 1024, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
        
            pr_info("Start  dma\n");
            desc->callback = test_dma_cb;
            desc->callback_param = NULL;
            cookie = dmaengine_submit(desc);
        
            loop = 3;
            while(loop) {
                st = dmaengine_tx_status(dma_chan,cookie,&state);
                pr_info("st is %d\n", st);
                if(st == DMA_COMPLETE)
                    break;
                msleep(10);
                --loop;
            }
            if(memcmp((void*)s_vaddr,(void*)d_vaddr, 1024)){
                pr_err("dma transmit failed");
            } else {
                pr_err("dma transmit OK");
            }
            if(!dma_chan){
                dma_release_channel(dma_chan); 
                dma_chan = NULL;
            }
err3:

            if(s_vaddr) {
                dma_unmap_single(dev,s_vaddr,1024, DMA_BIDIRECTIONAL);
                free_pages(s_vaddr, get_order(1024));
            }
            if(d_vaddr) {
                dma_unmap_single(dev,d_vaddr,1024, DMA_BIDIRECTIONAL);
            }
            s_vaddr = 0;
            d_vaddr = 0;

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

    pr_info("prbe [E]\n");
    dev = &(pdev->dev);

    pcie_data = kzalloc(sizeof(struct pcie_demo_data), GFP_KERNEL);
    if(!pcie_data){
        pr_err("failed alloc pcie_demo_data\n");
        return (-1);
    }
    pdata = pcie_data;

    dev_set_drvdata(dev, pdata);
    pdata->pcidev = pdev;
    pbar = pdata->bar;

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
        printk("resource: %pR\n",r);
        r++;
    }

    for(loop=0;loop<DEVICE_COUNT_RESOURCE; ++loop){
        pbar[loop].base_addr = pci_resource_start(pdev, loop);
        pbar[loop].size = pci_resource_len(pdev, loop);
        pbar[loop].flags = pci_resource_flags(pdev, loop);
        if(((pbar[loop].flags) & IORESOURCE_IO) || ((pbar[loop].flags) & IORESOURCE_MEM))  {
            pbar[loop].virt_addr = pci_iomap(pdev, loop, pbar[loop].size);
        }
    }

    pr_info("prbe [X]\n");
    return 0;



    pci_release_regions (pdev);
err_regions:
    pci_disable_device (pdev);
err_enable:
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
    dev = &(pdev->dev);

    pr_info("remove [E]\n");

	if(!dma_chan){
		dma_release_channel(dma_chan); 
		dma_chan = NULL;
    }
#ifdef DMA_SINGLE
    if(s_vaddr) {
        dma_unmap_single(dev,s_vaddr,1024, DMA_BIDIRECTIONAL);
        free_pages(s_vaddr, get_order(1024));
    }
    if(d_vaddr) {
        dma_unmap_single(dev,d_vaddr,1024, DMA_BIDIRECTIONAL);
        free_pages(d_vaddr, get_order(1024));
    }
#else
    if(s_vaddr)
        dma_free_coherent(dev, 1024, (void *)s_vaddr, s_baddr);
    if(d_vaddr)
        dma_free_coherent(dev, 1024, (void *)d_vaddr, d_baddr);
#endif
	s_vaddr = 0;
	d_vaddr = 0;

    dev = &(pdev->dev);
    pdata = (struct pcie_demo_data *)dev_get_drvdata(dev);


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




static struct pci_device_id demo_pcie_table[] = {
    {PCI_DEVICE(PCI_VENDOR_PF_ID_DEMO,PCI_DEVICE_PF_ID_DEMO), },
    { 0, }
};
MODULE_DEVICE_TABLE(pci, demo_pcie_table);

static struct pci_driver  demo_pcie_driver  = {
    .name       = "demo-pcie",
    .id_table   = demo_pcie_table,
    .probe      = demo_pcie_probe,
    .remove     = demo_pcie_remove,
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
