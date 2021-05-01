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
#include <linux/kthread.h>

#include <linux/sched.h>


#include <linux/iommu.h>
#include <linux/msi.h>
#include <linux/mm.h>
#include <linux/random.h>
#include <linux/spinlock.h>

#include <linux/delay.h>
#include <asm-generic/io.h>


#define DEV_NAME "PCIE_DEMO"

#define OPEN_INTX

#define IOC_MAGIC  'p'
#define PCIE_DEMO_READ_CONFIG       _IO(IOC_MAGIC, 0)
#define PCIE_DEMO_DUMP_RESOURCE     _IO(IOC_MAGIC, 1)
#define PCIE_DEMO_READ_BAR          _IO(IOC_MAGIC, 2)
#define PCIE_DEMO_WRITE_BAR         _IO(IOC_MAGIC, 3)
#define PCIE_BAR_STABLE_TEST        _IO(IOC_MAGIC, 4)


#define XILINX_PCIE_TEST
#ifdef  XILINX_PCIE_TEST
#define PCI_VENDOR_PF_ID_DEMO      0x10EE
#define PCI_DEVICE_PF_ID_DEMO      0x9032
#endif

struct pci_demo_bar {
    resource_size_t base_addr;
    void __iomem *virt_addr;
    resource_size_t size;
    unsigned long flags;
} __packed;


/* priv_flags */
#define DEMO_SRIOV_ENABLED      (1 << 0)
#define DEMO_MSI_ENABLED        (1 << 1)
#define DEMO_INTX_ENABLED       (1 << 2)

struct pcie_demo_data {
    u16    num_vfs;
    unsigned int priv_flags;
    struct pci_dev * pcidev;
    struct pci_demo_bar bar[DEVICE_COUNT_RESOURCE];
    struct mutex mlock;
    spinlock_t slock;
#ifdef OPEN_INTX
    u8    intx_line;
    u8    intx_pin;
    atomic64_t irq_nums;
#endif
	wait_queue_head_t  msg_wait;
	struct task_struct *ktask;
	unsigned long msg_cnt;	// 建议换成原子变量
};

struct pcie_demo_data  * pcie_data;

typedef struct VFIO_MSG{
    unsigned int magic;     // 必须是 0x5A5AA5A5，否则无效帧;
    unsigned int cmd;       // 命令帧, bit31==1代表命令ack报文
    unsigned int rvd;
    unsigned int ack;       // 如果是ack，那么返回值。如果cmd需要带参数
}VFIO_MSG;


 VFIO_MSG msg;

struct test_info {
    struct timer_list test_timer;
};
struct test_info tinfo;
static unsigned char c_tmp;
typedef enum vfio_cmd{
    CMD_CHANNEL_STAT,
    CMD_CHANNEL_START,
    CMD_CHANNEL_STOP,
    CMD_CHANNEL_PULSE,
    CMD_CHANNEL_CONTINUE,
}vfio_cmd;


/*
typedef unsigned short __u16;
typedef unsigned int   __u32;
typedef unsigned long  __u64;
#define ___constant_swab16(x) ((__u16)(				\
	(((__u16)(x) & (__u16)0x00ffU) << 8) |			\
	(((__u16)(x) & (__u16)0xff00U) >> 8)))

#define ___constant_swab32(x) ((__u32)(				\
	(((__u32)(x) & (__u32)0x000000ffUL) << 24) |		\
	(((__u32)(x) & (__u32)0x0000ff00UL) <<  8) |		\
	(((__u32)(x) & (__u32)0x00ff0000UL) >>  8) |		\
	(((__u32)(x) & (__u32)0xff000000UL) >> 24)))

#define ___constant_swab64(x) ((__u64)(				\
	(((__u64)(x) & (__u64)0x00000000000000ffULL) << 56) |	\
	(((__u64)(x) & (__u64)0x000000000000ff00ULL) << 40) |	\
	(((__u64)(x) & (__u64)0x0000000000ff0000ULL) << 24) |	\
	(((__u64)(x) & (__u64)0x00000000ff000000ULL) <<  8) |	\
	(((__u64)(x) & (__u64)0x000000ff00000000ULL) >>  8) |	\
	(((__u64)(x) & (__u64)0x0000ff0000000000ULL) >> 24) |	\
	(((__u64)(x) & (__u64)0x00ff000000000000ULL) >> 40) |	\
	(((__u64)(x) & (__u64)0xff00000000000000ULL) >> 56)))
*/


static unsigned int count = 0;
static unsigned long long stm = 0;  // 开始时间
static unsigned long long etm = 0;  // 结束时间
static unsigned long long ctm = 0;  // 结束时间
static unsigned long long ttm = 0;  // 总共花费时间


// static unsigned long long stm = (unsigned long long )ktime_to_us(ktime_get_real());
// static unsigned long long etm = (unsigned long long )ktime_to_us(ktime_get_real());



ssize_t pcie_transmit_msg(unsigned int cmd)
{
    unsigned long loop = 0;
    unsigned int tmp_cnt = 0;

#if 1
    msg.magic = 0xFF88A55A;
    msg.cmd   = cmd;
    msg.rvd   = (int)((stm >> 32) & 0xFFFFFFFF);
    msg.ack   = (int)(stm & 0xFFFFFFFF);
    iowrite32(msg.magic, pcie_data->bar[0].virt_addr + 0);
    iowrite32(msg.cmd, pcie_data->bar[0].virt_addr + 4);
	pr_info("write %#x\n", msg.magic);
    pr_info("write %#x\n", msg.cmd);

    // TBD ??? 为啥不能分一次发送？？？ 对面一次最多接收八个字节？？？
    iowrite32(msg.rvd, pcie_data->bar[0].virt_addr + 0);
    iowrite32(msg.ack, pcie_data->bar[0].virt_addr + 0x04);
    pr_info("write %#x\n", msg.rvd);
    pr_info("write %#x\n", msg.ack);
#else
    stm = (unsigned long long )ktime_to_ns(ktime_get_real());
    iowrite32(count, pcie_data->bar[0].virt_addr + 0);
    tmp_cnt = ioread32(pcie_data->bar[0].virt_addr + 0);
    etm = (unsigned long long )ktime_to_ns(ktime_get_real());
    pr_info("write cnt is %d, read is %d, cost %lld ns\n", count,tmp_cnt, etm-stm);
    tmp_cnt = 0;
    ++count;
#endif
    return loop;
}
EXPORT_SYMBOL(pcie_transmit_msg);



static ssize_t pcie_demo_read (struct file * filp, char __user * buf, size_t size, loff_t * offset)
{
    return 0;
}

static ssize_t pcie_demo_write (struct file * filp, const char __user * buf, size_t size, loff_t * offset)
{

    return 0;
}

static long pcie_demo_ioctl (struct file * filp, unsigned int cmd, unsigned long args)
{

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


#ifdef OPEN_INTX
static irqreturn_t demo_pci_intx_isr(int irq, void * data)
{

    unsigned long flags;
    struct pcie_demo_data *pdata = (struct pcie_demo_data *)data;
    spin_lock_irqsave(&(pdata->slock), flags);
    atomic64_inc(&(pdata->irq_nums));
    pr_info("recv irq-%lu", atomic64_read(&(pdata->irq_nums)));
	++pcie_data->msg_cnt;
	wake_up_interruptible(&pcie_data->msg_wait);
	pr_info("recv irq-wake");
    spin_unlock_irqrestore(&(pdata->slock), flags);


    return IRQ_HANDLED;
}
#endif




static void test_timer(struct timer_list *t)
{
	struct test_info *ts = from_timer(ts, t, test_timer);

#if 1
    if(c_tmp == 0){
        pcie_transmit_msg(CMD_CHANNEL_START);
        c_tmp = 1;
    } else {
        pcie_transmit_msg(CMD_CHANNEL_STOP);
        c_tmp = 0;
    }
#else
	++pcie_data->msg_cnt;
	wake_up_interruptible(&pcie_data->msg_wait);
#endif
	mod_timer(&tinfo.test_timer, jiffies+msecs_to_jiffies(5000));
}

static int test_task(void * data)
{
	while(1) {
		if(wait_event_interruptible(pcie_data->msg_wait,pcie_data->msg_cnt != 0)) {
			pr_info("Recv Invaild  signal\n");
        	return -ERESTARTSYS; // receive signal
    	}
		pr_info("Recv msg cnt is %ld\n", pcie_data->msg_cnt);
		--pcie_data->msg_cnt;
	}
}


int  demo_pcie_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
    struct pci_bus *bus;
    struct pci_ops *ops;
    struct resource * r;
    int loop = 0;
    int err = -EINVAL;
    struct pci_demo_bar * pbar;
    struct device * dev;
    unsigned short cfg_cmd;
    unsigned short cfg_stat;

    pr_info("prbe [E] 20210120\n");
    dev = &(pdev->dev);
    pcie_data = kzalloc(sizeof(struct pcie_demo_data), GFP_KERNEL);
    if(!pcie_data){
        pr_err("failed alloc pcie_demo_data\n");
        return (-1);
    }

    mutex_init(&(pcie_data->mlock));
    spin_lock_init(&(pcie_data->slock));
    dev_set_drvdata(dev, pcie_data);
    pcie_data->pcidev = pdev;
    pbar = pcie_data->bar;

#ifdef OPEN_INTX
    pci_read_config_byte(pdev,0x3c, &pcie_data->intx_line);
    pci_read_config_byte(pdev,0x3e, &pcie_data->intx_pin);
    atomic64_set(&(pcie_data->irq_nums), 0);
#endif
    pci_read_config_word(pdev,0x04,&cfg_cmd);
    pci_read_config_word(pdev,0x04,&cfg_stat);

    pr_info("before enable:cfg cmd is %#x, cfg_stat is %#x\n",cfg_cmd, cfg_stat);
    /* Enable PCIE device */
    if (pci_enable_device(pdev)){
        pr_err("pci_enable_device failed\n");
        err = -EIO;
        goto err_enable;
    }
    pr_info("end enable:cfg cmd is %#x, cfg_stat is %#x\n",cfg_cmd, cfg_stat);
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

#ifdef OPEN_INTX
    if(!(pcie_data->priv_flags & DEMO_INTX_ENABLED)) {
    	err = devm_request_irq(dev, pdev->irq, demo_pci_intx_isr,
		       IRQF_SHARED ,
		       "vfio-pcie", pcie_data);
    	if(err){
            pr_info("Request irq failed\n");
        } else {
            pcie_data->priv_flags |= DEMO_INTX_ENABLED;
            pr_info("Request irq OK\n");
        }
    }
#endif

    timer_setup(&tinfo.test_timer, test_timer, 0);
	mod_timer(&tinfo.test_timer, jiffies+msecs_to_jiffies(5000));

	init_waitqueue_head(&pcie_data->msg_wait);
	pcie_data->ktask = kthread_run(test_task,NULL,"test_task");

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

    pr_info("remove [E]\n");
    del_timer_sync(&tinfo.test_timer);

    dev = &(pdev->dev);
    pdata = (struct pcie_demo_data *)dev_get_drvdata(dev);

#ifdef OPEN_INTX
    if(pcie_data->priv_flags & DEMO_INTX_ENABLED) {
        // void devm_free_irq(struct device *dev, unsigned int irq, void *dev_id)
        devm_free_irq(dev, pdata->intx_line, pdata);
    }
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
    pr_info("exit [X]\n");
}

module_init(pcie_demo_init);
module_exit(pcie_demo_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("baiy <baiyang0223@163.com>");
MODULE_DESCRIPTION("TEST PCIE_DRIVER for GPU");
MODULE_VERSION("1.0.0.0");
