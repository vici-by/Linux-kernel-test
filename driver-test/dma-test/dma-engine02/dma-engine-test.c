/*************************************************************************
    > File Name: driver_misc_test.c
    > Author: baiy
    > Mail: baiyang0223@163.com 
    > Created Time: 2020-10-21-09:33:13
    > Func: 
 ************************************************************************/

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>

 #include <linux/miscdevice.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 3, 0)
	#include <asm/switch_to.h>
#else
    #include <asm/system.h>
#endif


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



#define DEV_NAME "MISC_DEMO"

/* 定义设备类型 */
#define IOC_MAGIC  'm'
#define MISC_DEMO_TEST   	_IO(IOC_MAGIC, 0)

struct misc_demo_data {
	int num;
};


unsigned long s_vaddr;
unsigned long d_vaddr;
dma_addr_t s_baddr;
dma_addr_t d_baddr;

struct dma_chan * dma_chan;
struct dma_async_tx_descriptor *desc;

struct dma_slave_config dma_config;



static ssize_t misc_demo_read (struct file * filp, char __user * buf, size_t size, loff_t * offset)
{
	return 0;
}
static ssize_t misc_demo_write (struct file * filp, const char __user * buf, size_t size, loff_t * offset)
{

	return 0;
}
static long misc_demo_ioctl (struct file * filp, unsigned int cmd, unsigned long args)
{
    switch(cmd){
    case MISC_DEMO_TEST:
        break;
    default:
        break;
    }
	return 0;
}

static int misc_demo_open (struct inode * inode, struct file * filp)
{
    

	return 0;
}
static int misc_demo_release (struct inode * inode, struct file * filp)
{
	return 0;
}


static struct file_operations misc_demo_fops = {
    .owner = THIS_MODULE,
	.open  = misc_demo_open,
	.release = misc_demo_release,
	.read  = misc_demo_read,
	.write = misc_demo_write,
	.unlocked_ioctl = misc_demo_ioctl,
	.compat_ioctl   = misc_demo_ioctl,
};

static struct miscdevice misc_demo_drv = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = DEV_NAME,
	.fops  = &misc_demo_fops,
};

static void test_dma_cb(void *arg)
{
    pr_info("transmit once\n");
}

static int __init misc_demo_init(void)
{

#if 1
    dma_cookie_t cookie;
    dma_cap_mask_t mask;
    dma_cap_zero(mask);
    dma_cap_set(DMA_MEMCPY, mask);
    dma_chan = dma_request_channel(mask, NULL, NULL);
	pr_info("%s: %s (%s)\n",
		 __func__,
		 dma_chan ? "success" : "fail",
		 dma_chan ? dma_chan_name(dma_chan) : NULL);

    if(!dma_chan){
        pr_err("dma request failed\n");
        return -EINVAL;
    }
#endif
#ifdef DMA_SINGLE
    s_vaddr = __get_free_pages(GFP_KERNEL,get_order(0x1024));
    d_vaddr = __get_free_pages(GFP_KERNEL,get_order(0x1024));
    if(s_vaddr == 0 || d_vaddr == 0){
        goto err2;
    }
    s_baddr = dma_map_single(NULL,s_vaddr,0x1024, DMA_BIDIRECTIONAL);
    d_baddr = dma_map_single(NULL,d_vaddr,0x1024, DMA_BIDIRECTIONAL);
    pr_info("map dma buffer OK, s_v=%pK,s_b=%pK,d_v=%pK,d_b=%pK\n", s_vaddr,s_baddr,d_vaddr,d_baddr);

#else
    s_vaddr = (unsigned long)dma_alloc_coherent(NULL,0x1024,&s_baddr, GFP_KERNEL);
    d_vaddr = (unsigned long)dma_alloc_coherent(NULL,0x1024,&d_baddr, GFP_KERNEL);
    if(s_vaddr == 0 || d_vaddr == 0){  
        goto err2;
    } else {
        pr_info("coherent dma buffer OK, s_v=%pK,s_b=%pK,d_v=%pK,d_b=%pK\n", s_vaddr,s_baddr,d_vaddr,d_baddr);
    }
#endif

#if 0
        
    dma_config.direction = DMA_MEM_TO_MEM;
    dma_config.src_addr = s_baddr;
    dma_config.dst_addr = d_baddr;
    dma_config.src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
    dma_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
    dma_config.src_maxburst = 1024;
    dma_config.dst_maxburst = 1024;
    dma_config.device_fc = false;


    dmaengine_slave_config(dma_chan, &dma_config);
    desc = dmaengine_prep_dma_cyclic(dma_chan, s_baddr, 0x1024, 0x1024, DMA_MEM_TO_MEM,DMA_PREP_INTERRUPT );

	desc->callback = test_dma_cb;
	desc->callback_param = NULL;
	cookie = dmaengine_submit(desc);
#endif
	pr_info("init [E]\n");

	// misc_register(&misc_demo_drv);

    return 0;
err2:
#ifdef DMA_SINGLE
    if(s_vaddr)
        free_pages(s_vaddr, get_order(0x1024));
    if(d_vaddr)
        free_pages(d_vaddr, get_order(0x1024));
#else
    if(s_vaddr)
        dma_free_coherent(NULL, 0x1024, (void *)s_vaddr, s_baddr);
    if(d_vaddr)
        dma_free_coherent(NULL, 0x1024, (void *)d_vaddr, d_baddr);
#endif
    if(!dma_chan){
       dma_release_channel(dma_chan); 
    }
    pr_err("INIT Failed");
	return -1;
}

static void __exit misc_demo_exit(void)
{
#ifdef DMA_SINGLE
    if(s_vaddr)
        free_pages(s_vaddr, get_order(0x1024));
    if(d_vaddr)
        free_pages(d_vaddr, get_order(0x1024));
#else
    if(s_vaddr)
        dma_free_coherent(NULL, 0x1024, (void *)s_vaddr, s_baddr);
    if(d_vaddr)
        dma_free_coherent(NULL, 0x1024, (void *)d_vaddr, d_baddr);
#endif

    if(!dma_chan){
       dma_release_channel(dma_chan); 
    }


	pr_info("exit [E]\n");
	// misc_deregister(&misc_demo_drv);
}
module_init(misc_demo_init);
module_exit(misc_demo_exit);

MODULE_LICENSE("GPL v2");




