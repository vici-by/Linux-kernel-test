/*
 * Mediated virtual PCI serial host device driver
 *
 * Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.
 *     Author: Neo Jia <cjia@nvidia.com>
 *             Kirti Wankhede <kwankhede@nvidia.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Sample driver that creates mdev device that simulates serial port over PCI
 * card.
 *
 */
#define pr_fmt(fmt)  "[%s:%d] " fmt, __func__, __LINE__
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/uuid.h>
#include <linux/vfio.h>
#include <linux/iommu.h>
#include <linux/sysfs.h>
#include <linux/ctype.h>
#include <linux/file.h>
#include <linux/mdev.h>
#include <linux/pci.h>
#include <linux/serial.h>
#include <uapi/linux/serial_reg.h>
#include <linux/eventfd.h>
#include <linux/version.h>
#include "vfio_msg.h"
#include <linux/timekeeping.h>

/*
 * #defines
 */

#define VERSION_STRING  "0.1"
#define DRIVER_AUTHOR   "NVIDIA Corporation"

#define MTTY_CLASS_NAME "mtty"

#define MTTY_NAME       "mtty"

#define MTTY_STRING_LEN		16

#define MTTY_CONFIG_SPACE_SIZE  0xff
#define MTTY_IO_BAR_SIZE        0x8
#define MTTY_MMIO_BAR_SIZE      0x400

#define STORE_LE16(addr, val)   (*(u16 *)addr = val)
#define STORE_LE32(addr, val)   (*(u32 *)addr = val)

#define MAX_FIFO_SIZE   16

#define CIRCULAR_BUF_INC_IDX(idx)    (idx = (idx + 1) & (MAX_FIFO_SIZE - 1))

#define MTTY_VFIO_PCI_OFFSET_SHIFT   40

#define MTTY_VFIO_PCI_OFFSET_TO_INDEX(off)   (off >> MTTY_VFIO_PCI_OFFSET_SHIFT)
#define MTTY_VFIO_PCI_INDEX_TO_OFFSET(index) \
				((u64)(index) << MTTY_VFIO_PCI_OFFSET_SHIFT)
#define MTTY_VFIO_PCI_OFFSET_MASK    \
				(((u64)(1) << MTTY_VFIO_PCI_OFFSET_SHIFT) - 1)
#define MAX_MTTYS	24



//#define DEBUG       1
//#define DEBUG_REGS  1
//#define DEBUG_INTR  1
static unsigned long long total_ns = 0;
static int test_msg_cnt = 0;

// extern void vfio_add_msg(unsigned int channel, struct VFIO_MSG_LIST * msgl);


/*
 * Global Structures
 */
static struct mtty_dev {
	dev_t		vd_devt;
	struct class	*vd_class;
	struct cdev	vd_cdev;
	struct idr	vd_idr;
	struct device	dev;
} mtty_dev;

struct mdev_region_info {
	u64 start;
	u64 phys_start;
	u32 size;
	u64 vfio_offset;
};

/* loop back buffer */
struct rxtx {
    VFIO_MSG_LIST * rxfifo;
    VFIO_MSG_LIST * txfifo;
    u8 rxcnt;
    u8 txcnt;

	u8 fifo[MAX_FIFO_SIZE];
	u8 head, tail;
	u8 count;
};

struct serial_port {
	u8 uart_reg[8];         /* 8 registers */
	struct rxtx rxtx;       /* loop back buffer */
	bool dlab;
	bool overrun;
	u16 divisor;
	u8 fcr;                 /* FIFO control register */
	u8 max_fifo_size;
	u8 intr_trigger_level;  /* interrupt trigger level */
};

/* State of each mdev device */
struct mdev_state {
	int irq_fd;
    int channel;
	struct eventfd_ctx *intx_evtfd;
	struct eventfd_ctx *msi_evtfd;
	int irq_index;
	u8 *vconfig;
	struct mutex ops_lock;
	struct mdev_device *mdev;
	struct mdev_region_info region_info[VFIO_PCI_NUM_REGIONS];
	u32 bar_mask[VFIO_PCI_NUM_REGIONS];
	struct list_head next;
	struct serial_port s[2];
	struct mutex rxtx_lock;
	struct vfio_device_info dev_info;
	int nr_ports;
};

static struct mutex mdev_list_lock;
static struct list_head mdev_devices_list;

static const struct file_operations vd_fops = {
	.owner          = THIS_MODULE,
};

/* function prototypes */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0))
static int mtty_trigger_interrupt(uuid_le uuid);

/* Helper functions */
static struct mdev_state *find_mdev_state_by_uuid(uuid_le uuid)
{
	struct mdev_state *mds;

	list_for_each_entry(mds, &mdev_devices_list, next) {
		if (uuid_le_cmp(mdev_uuid(mds->mdev), uuid) == 0)
			return mds;
	}

	return NULL;
}
#else
static int mtty_trigger_interrupt(struct mdev_state *mdev_state);
#endif
static void dump_buffer(u8 *buf, uint32_t count)
{
#if defined(DEBUG)
	int i;

	pr_info("Buffer:\n");
	for (i = 0; i < count; i++) {
		pr_info("%2x ", *(buf + i));
		if ((i + 1) % 16 == 0)
			pr_info("\n");
	}
#endif
}

static void mtty_create_config_space(struct mdev_state *mdev_state)
{
	/* PCI dev ID */
	STORE_LE32((u32 *) &mdev_state->vconfig[0x0], 0x903210ee);

	/* Control: I/O+, Mem-, BusMaster- */
	STORE_LE16((u16 *) &mdev_state->vconfig[0x4], 0x0003);

	/* Status: capabilities list absent */
	STORE_LE16((u16 *) &mdev_state->vconfig[0x6], 0x0200);

	/* Rev ID */
	mdev_state->vconfig[0x8] =  0x10;

	/* programming interface class : 16550-compatible serial controller */
    mdev_state->vconfig[0x9] =  0x00;
	/* Sub class : 00 */
	mdev_state->vconfig[0xa] =  0x80;

	/* Base class : Simple Communication controllers */
	mdev_state->vconfig[0xb] =  0x05;

	/* base address registers */
#if 0
	/* BAR0: IO space */
	STORE_LE32((u32 *) &mdev_state->vconfig[0x10], 0x000001);
	mdev_state->bar_mask[0] = ~(MTTY_IO_BAR_SIZE) + 1;

	if (mdev_state->nr_ports == 2) {
		/* BAR1: IO space */
		STORE_LE32((u32 *) &mdev_state->vconfig[0x14], 0x000001);
		mdev_state->bar_mask[1] = ~(MTTY_IO_BAR_SIZE) + 1;
	}
#else
	STORE_LE32((u32 *) &mdev_state->vconfig[0x10], 0x000000);  // Memory空间
	mdev_state->bar_mask[0] = ~(MTTY_MMIO_BAR_SIZE) + 1;       // 长度掩码空间
    pr_info("bar 0 mask is %#x\n",mdev_state->bar_mask[0]);

	if (mdev_state->nr_ports == 2) {
		/* BAR1: IO space */
		STORE_LE32((u32 *) &mdev_state->vconfig[0x14], 0x000000);
		mdev_state->bar_mask[1] = ~(MTTY_IO_BAR_SIZE) + 1;
	}
#endif
	/* Subsystem ID */
	// STORE_LE32((u32 *) &mdev_state->vconfig[0x2c], 0x32534348);
	STORE_LE32((u32 *) &mdev_state->vconfig[0x2c], 0x00000000);

	mdev_state->vconfig[0x34] =  0x00;   /* Cap Ptr */
	mdev_state->vconfig[0x3d] =  0x01;   /* interrupt pin (INTA#) */

	/* Vendor specific data */
#if 0
	mdev_state->vconfig[0x40] =  0x23;
	mdev_state->vconfig[0x43] =  0x80;
	mdev_state->vconfig[0x44] =  0x23;
	mdev_state->vconfig[0x48] =  0x23;
	mdev_state->vconfig[0x4c] =  0x23;

	mdev_state->vconfig[0x60] =  0x50;
	mdev_state->vconfig[0x61] =  0x43;
	mdev_state->vconfig[0x62] =  0x49;
	mdev_state->vconfig[0x63] =  0x20;
	mdev_state->vconfig[0x64] =  0x53;
	mdev_state->vconfig[0x65] =  0x65;
	mdev_state->vconfig[0x66] =  0x72;
	mdev_state->vconfig[0x67] =  0x69;
	mdev_state->vconfig[0x68] =  0x61;
	mdev_state->vconfig[0x69] =  0x6c;
	mdev_state->vconfig[0x6a] =  0x2f;
	mdev_state->vconfig[0x6b] =  0x55;
	mdev_state->vconfig[0x6c] =  0x41;
	mdev_state->vconfig[0x6d] =  0x52;
	mdev_state->vconfig[0x6e] =  0x54;
#endif
}

static void handle_pci_cfg_write(struct mdev_state *mdev_state, u16 offset,
				 u8 *buf, u32 count)
{
	u32 cfg_addr, bar_mask, bar_index = 0;

	switch (offset) {
	case 0x04: /* device control */
	case 0x06: /* device status */
		/* do nothing */
		break;
	case 0x3c:  /* interrupt line */
		mdev_state->vconfig[0x3c] = buf[0];
		break;
	case 0x3d:
		/*
		 * Interrupt Pin is hardwired to INTA.
		 * This field is write protected by hardware
		 */
		break;
	case 0x10:  /* BAR0 */
	case 0x14:  /* BAR1 */
		if (offset == 0x10)
			bar_index = 0;
		else if (offset == 0x14)
			bar_index = 1;

		if ((mdev_state->nr_ports == 1) && (bar_index == 1)) {
			STORE_LE32(&mdev_state->vconfig[offset], 0);
			break;
		}

		cfg_addr = *(u32 *)buf;
		pr_info("BAR%d addr 0x%x\n", bar_index, cfg_addr);

		if (cfg_addr == 0xffffffff) {
			bar_mask = mdev_state->bar_mask[bar_index];  // 和长度掩码 相与可以计算出长度
			cfg_addr = (cfg_addr & bar_mask);
            pr_info("!!!bar 0 mask is %#x,len is %#x\n",bar_mask, cfg_addr);
		}

		cfg_addr |= (mdev_state->vconfig[offset] & 0x3ul);  // 判断是IO空间还是内存空间
		pr_info("BAR%d  write %#x\n",bar_index,cfg_addr);
		STORE_LE32(&mdev_state->vconfig[offset], cfg_addr);

		break;
	case 0x18:  /* BAR2 */
	case 0x1c:  /* BAR3 */
	case 0x20:  /* BAR4 */
		STORE_LE32(&mdev_state->vconfig[offset], 0);
		break;
	default:
		pr_info("PCI config write @0x%x of %d bytes not handled\n",
			offset, count);
		break;
	}
}

static void handle_bar_write(unsigned int index, struct mdev_state *mdev_state,
				u16 offset, u8 *buf, u32 count)
{
    struct rxtx   *prxtx;
    VFIO_MSG_LIST *msgl;
    char      *msg;
	u8 data = *buf;

    mutex_lock(&mdev_state->rxtx_lock);
	/* Handle data written by guest */
    prxtx = &(mdev_state->s[index].rxtx);

    if(prxtx->rxcnt == 16 && prxtx->rxfifo){
        //vfio_add_msg(mdev_state->channel,( struct VFIO_MSG_LIST * )prxtx->rxfifo);
        prxtx->rxfifo = NULL;
        prxtx->rxcnt  = 0;
    }
#if defined(DEBUG)
    pr_info("count is %d, rxcnt is %d\n",count, prxtx->rxcnt);
#endif


    if(prxtx->rxcnt == 0 && buf[0] != 0x5A){  //  不是一个有效的数据
        pr_err("Invaild magic[%#x], drop it\n",buf[0]);
        mutex_unlock(&mdev_state->rxtx_lock);
        return;
    }
    if( prxtx->rxcnt  && !prxtx->rxfifo ) {  //   本地数据异常
        pr_err("rxcnt is %d, but fifo is NULL, may be abort\n",prxtx->rxcnt);
        prxtx->rxcnt   = 0;
    }

    if(offset == 0 && buf[0] == 0x5A){  // 说明是一个合法头部
        if(prxtx->rxfifo){
            prxtx->rxfifo = NULL;
        }
        prxtx->rxcnt = 0;
        prxtx->rxfifo = kzalloc(sizeof(struct VFIO_MSG_LIST),GFP_KERNEL);
        if(prxtx->rxfifo == NULL){
            pr_err("kzalloc failed. short of memory\n");
            mutex_unlock(&mdev_state->rxtx_lock);
            return;
        }
        #if defined(DEBUG)
        pr_info("alloc fifo is %#lx\n",(unsigned long)prxtx->rxfifo);
        #endif
        INIT_LIST_HEAD(&(prxtx->rxfifo->list));
    }

    if(count + prxtx->rxcnt  > 16) {
        pr_err("recv count  greater than 16, is %d, cur is %d error\n",\
            count, prxtx->rxcnt);
        count = 16 - prxtx->rxcnt;
    }


#if defined(DEBUG)
    pr_info("count is %d, rxcnt is %d\n",count, prxtx->rxcnt);
#endif

    msg = (char *)&(prxtx->rxfifo->msg);
    memcpy(msg + prxtx->rxcnt, buf, count );
    prxtx->rxcnt += count;
#if defined(DEBUG)
    pr_info("recv count is %d, total count is %d\n",count,prxtx->rxcnt);
#endif
    if(prxtx->rxcnt == 16){
        int cnt =prxtx->rxfifo->msg.cmd;
        unsigned long long ttm = 0;
        unsigned long long etm = (unsigned long long )ktime_to_us(ktime_get_real());
        unsigned long long stm = ((prxtx->rxfifo->msg.rvd << 32)& 0xFFFFFFFF00000000 ) | prxtx->rxfifo->msg.ack;
        ttm = (prxtx->rxfifo->msg.rvd);
        stm = ((ttm << 32)& 0xFFFFFFFF00000000 ) | (prxtx->rxfifo->msg.ack & 0xFFFFFFFF);
        test_msg_cnt++;
        unsigned long long ctm = etm - stm;
        total_ns += ctm;
        pr_info("recv cnt %d,cur cnt %d, stm %lld, etm %lld, cost %lld, per %lld\n",
            cnt, test_msg_cnt, stm,etm,etm-stm, (total_ns / test_msg_cnt));


        #if defined(DEBUG)
        pr_info("Get a package:\n");
        #endif

        dump_buffer((char *)&(prxtx->rxfifo->msg), 16);
        #if 0
        //vfio_add_msg(mdev_state->channel,prxtx->rxfifo);
        #else
        kfree(prxtx->rxfifo);
        #endif
        prxtx->rxfifo = NULL;
        prxtx->rxcnt  = 0;
    }
	mutex_unlock(&mdev_state->rxtx_lock);
}

static void handle_bar_read(unsigned int index, struct mdev_state *mdev_state,
			    u16 offset, u8 *buf, u32 count)
{
	/* Handle read requests by guest */
}

static void mdev_read_base(struct mdev_state *mdev_state)
{
	int index, pos;
	u32 start_lo, start_hi;
	u32 mem_type;

	pos = PCI_BASE_ADDRESS_0;

	for (index = 0; index <= VFIO_PCI_BAR5_REGION_INDEX; index++) {

		if (!mdev_state->region_info[index].size)
			continue;

		start_lo = (*(u32 *)(mdev_state->vconfig + pos)) &
			PCI_BASE_ADDRESS_MEM_MASK;
		mem_type = (*(u32 *)(mdev_state->vconfig + pos)) &
			PCI_BASE_ADDRESS_MEM_TYPE_MASK;

		switch (mem_type) {
		case PCI_BASE_ADDRESS_MEM_TYPE_64:
			start_hi = (*(u32 *)(mdev_state->vconfig + pos + 4));
			pos += 4;
			break;
		case PCI_BASE_ADDRESS_MEM_TYPE_32:
		case PCI_BASE_ADDRESS_MEM_TYPE_1M:
			/* 1M mem BAR treated as 32-bit BAR */
		default:
			/* mem unknown type treated as 32-bit BAR */
			start_hi = 0;
			break;
		}
		pos += 4;
		mdev_state->region_info[index].start = ((u64)start_hi << 32) |
							start_lo;
	}
}

static ssize_t mdev_access(struct mdev_device *mdev, u8 *buf, size_t count,
			   loff_t pos, bool is_write)
{
	struct mdev_state *mdev_state;
	unsigned int index;
	loff_t offset;
	int ret = 0;

	if (!mdev || !buf)
		return -EINVAL;

	mdev_state = mdev_get_drvdata(mdev);
	if (!mdev_state) {
		pr_err("%s mdev_state not found\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&mdev_state->ops_lock);

	index = MTTY_VFIO_PCI_OFFSET_TO_INDEX(pos);
	offset = pos & MTTY_VFIO_PCI_OFFSET_MASK;
    /*

    enum {
        VFIO_PCI_BAR0_REGION_INDEX,
        VFIO_PCI_BAR1_REGION_INDEX,
        VFIO_PCI_BAR2_REGION_INDEX,
        VFIO_PCI_BAR3_REGION_INDEX,
        VFIO_PCI_BAR4_REGION_INDEX,
        VFIO_PCI_BAR5_REGION_INDEX,
        VFIO_PCI_ROM_REGION_INDEX,
        VFIO_PCI_CONFIG_REGION_INDEX,
        VFIO_PCI_VGA_REGION_INDEX,
        VFIO_PCI_NUM_REGIONS = 9
    };
    */

	switch (index) {
	case VFIO_PCI_CONFIG_REGION_INDEX:

#if defined(DEBUG)
		pr_info("%s: PCI config space %s at offset 0x%llx\n",
			 __func__, is_write ? "write" : "read", offset);
#endif
		if (is_write) {
			dump_buffer(buf, count);
			handle_pci_cfg_write(mdev_state, offset, buf, count);
		} else {
			memcpy(buf, (mdev_state->vconfig + offset), count);
			dump_buffer(buf, count);
		}
		break;

	case VFIO_PCI_BAR0_REGION_INDEX ... VFIO_PCI_BAR5_REGION_INDEX:
		if (!mdev_state->region_info[index].start)
			mdev_read_base(mdev_state);

		if (is_write) {
            dump_buffer(buf, count);
#if defined(DEBUG_REGS)
            pr_info("%s: BAR%d  WR offset @0x%llx  val:0x%02x dlab:%d\n",
                __func__, index, offset,
                *buf, mdev_state->s[index].dlab);
#endif
			handle_bar_write(index, mdev_state, offset, buf, count);
		} else {
			handle_bar_read(index, mdev_state, offset, buf, count);
			dump_buffer(buf, count);
#if defined(DEBUG_REGS)
            pr_info("%s: BAR%d  RD offset @0x%llx val:0x%02x dlab:%d\n",
                __func__, index, offset,
                *buf, mdev_state->s[index].dlab);
#endif
		}
		break;

	default:
		ret = -1;
		goto accessfailed;
	}

	ret = count;


accessfailed:
	mutex_unlock(&mdev_state->ops_lock);

	return ret;
}


struct mdev_device {
	struct device dev;
	struct mdev_parent *parent;
	guid_t uuid;
	void *driver_data;
	struct list_head next;
	struct kobject *type_kobj;
	struct device *iommu_device;
	bool active;
};

static int vfio_get_channel(struct mdev_device *mdev)
{
    int i = 0;
    char * p;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0))
    guid_t uuid;

    if(!mdev){
        pr_err("Invaild mdev device\n");
        return -ENOENT;
    }
    uuid = (guid_t)mdev_uuid(mdev);
#else
    guid_t uuid;
    const guid_t *muuid;
    if(!mdev){
        pr_err("Invaild mdev device\n");
        return -ENOENT;
    }
    muuid = mdev_uuid(mdev);
    guid_copy( &uuid, muuid);
#endif
    p = uuid.b;
    pr_cont("mdev id is :");
    for(i=0; i<16; ++i){
        pr_cont("%#.2x ", uuid.b[i] & 0xff);
    }
    pr_cont("\n");
    pr_info("channel is %#x\n",uuid.b[15] & 0xff );
    return uuid.b[15] & 0xff;
}

static int mtty_create(struct kobject *kobj, struct mdev_device *mdev)
{
	struct mdev_state *mdev_state;
	char name[MTTY_STRING_LEN];
	int nr_ports = 0, i;

	if (!mdev)
		return -EINVAL;

	for (i = 0; i < 1; i++) {
		snprintf(name, MTTY_STRING_LEN, "%s-%d",
			dev_driver_string(mdev_parent_dev(mdev)), i + 1);
		if (!strcmp(kobj->name, name)) {
            pr_info("port[%d] name is %s\n",i, name);
			nr_ports = i + 1;
			break;
		}
	}
    pr_info("nr_port(BAR) num is %d\n",nr_ports);
	if (!nr_ports)
		return -EINVAL;

	mdev_state = kzalloc(sizeof(struct mdev_state), GFP_KERNEL);
	if (mdev_state == NULL)
		return -ENOMEM;

	mdev_state->nr_ports = nr_ports;
	mdev_state->irq_index = -1;
	mdev_state->s[0].max_fifo_size = MAX_FIFO_SIZE;
	mdev_state->s[1].max_fifo_size = MAX_FIFO_SIZE;
    mdev_state->channel = vfio_get_channel(mdev);
	mutex_init(&mdev_state->rxtx_lock);
	mdev_state->vconfig = kzalloc(MTTY_CONFIG_SPACE_SIZE, GFP_KERNEL);
	if (mdev_state->vconfig == NULL) {
		kfree(mdev_state);
		return -ENOMEM;
	}

	mutex_init(&mdev_state->ops_lock);
	mdev_state->mdev = mdev;
	mdev_set_drvdata(mdev, mdev_state);

	mtty_create_config_space(mdev_state);

	mutex_lock(&mdev_list_lock);
	list_add(&mdev_state->next, &mdev_devices_list);
	mutex_unlock(&mdev_list_lock);

	return 0;
}

static int mtty_remove(struct mdev_device *mdev)
{
	struct mdev_state *mds, *tmp_mds;
	struct mdev_state *mdev_state = mdev_get_drvdata(mdev);
	int ret = -EINVAL;

	mutex_lock(&mdev_list_lock);
	list_for_each_entry_safe(mds, tmp_mds, &mdev_devices_list, next) {
		if (mdev_state == mds) {
			list_del(&mdev_state->next);
			mdev_set_drvdata(mdev, NULL);
			kfree(mdev_state->vconfig);
			kfree(mdev_state);
			ret = 0;
			break;
		}
	}
	mutex_unlock(&mdev_list_lock);

	return ret;
}

static int mtty_reset(struct mdev_device *mdev)
{
	struct mdev_state *mdev_state;

	if (!mdev)
		return -EINVAL;

	mdev_state = mdev_get_drvdata(mdev);
	if (!mdev_state)
		return -EINVAL;

	pr_info("%s: called\n", __func__);

	return 0;
}


ssize_t mtty_read(struct mdev_device *mdev, char __user *buf, size_t count,
		  loff_t *ppos)
{
	unsigned int done = 0;
	int ret;

#if defined(DEBUG)
    pr_info("\n\n\n");
    pr_info("===============[Start Read]=======================\n");
    pr_info("Read count is %ld,offset [%lld-%#llx]\n",count, *ppos,*ppos);
#endif

	while (count) {
		size_t filled;

		if (count >= 4 && !(*ppos % 4)) {
			u32 val;

			ret =  mdev_access(mdev, (char *)&val, sizeof(val),
					   *ppos, false);
			if (ret <= 0)
				goto read_err;

			if (copy_to_user(buf, &val, sizeof(val)))
				goto read_err;

			filled = 4;
		} else if (count >= 2 && !(*ppos % 2)) {
			u16 val;

			ret = mdev_access(mdev, (char *)&val, sizeof(val),
					  *ppos, false);
			if (ret <= 0)
				goto read_err;

			if (copy_to_user(buf, &val, sizeof(val)))
				goto read_err;

			filled = 2;
		} else {
			u8 val;

			ret = mdev_access(mdev, (char *)&val, sizeof(val),
					  *ppos, false);
			if (ret <= 0)
				goto read_err;

			if (copy_to_user(buf, &val, sizeof(val)))
				goto read_err;

			filled = 1;
		}

		count -= filled;
		done += filled;
		*ppos += filled;
		buf += filled;
	}

#if defined(DEBUG)
    pr_info("===============[End Read]=======================\n");
#endif

	return done;

read_err:
#if defined(DEBUG)
    pr_info("===============[Read Error]=======================\n");
#endif

	return -EFAULT;
}

ssize_t mtty_write(struct mdev_device *mdev, const char __user *buf,
		   size_t count, loff_t *ppos)
{
	unsigned int done = 0;
	int ret;
#if defined(DEBUG)
    pr_info("\n\n\n");
    pr_info("===============[Start Write]=======================\n");
    pr_info("Write count is %ld,offset [%lld-%#llx]\n",count, *ppos,*ppos);
#endif

	while (count) {
		size_t filled;

		if (count >= 4 && !(*ppos % 4)) {
			u32 val;

			if (copy_from_user(&val, buf, sizeof(val)))
				goto write_err;

			ret = mdev_access(mdev, (char *)&val, sizeof(val),
					  *ppos, true);
			if (ret <= 0)
				goto write_err;

			filled = 4;
		} else if (count >= 2 && !(*ppos % 2)) {
			u16 val;

			if (copy_from_user(&val, buf, sizeof(val)))
				goto write_err;

			ret = mdev_access(mdev, (char *)&val, sizeof(val),
					  *ppos, true);
			if (ret <= 0)
				goto write_err;

			filled = 2;
		} else {
			u8 val;

			if (copy_from_user(&val, buf, sizeof(val)))
				goto write_err;

			ret = mdev_access(mdev, (char *)&val, sizeof(val),
					  *ppos, true);
			if (ret <= 0)
				goto write_err;

			filled = 1;
		}
		count -= filled;
		done += filled;
		*ppos += filled;
		buf += filled;
	}
#if defined(DEBUG)
    pr_info("===============[End Write]=======================\n");
#endif
	return done;
write_err:
#if defined(DEBUG)
    pr_info("===============[Write Error]=======================\n");
#endif

	return -EFAULT;
}



static int mtty_set_irqs(struct mdev_device *mdev, uint32_t flags,
			 unsigned int index, unsigned int start,
			 unsigned int count, void *data)
{
	int ret = 0;
	struct mdev_state *mdev_state;

	if (!mdev)
		return -EINVAL;

	mdev_state = mdev_get_drvdata(mdev);
	if (!mdev_state)
		return -EINVAL;

	mutex_lock(&mdev_state->ops_lock);
	switch (index) {
	case VFIO_PCI_INTX_IRQ_INDEX:
		switch (flags & VFIO_IRQ_SET_ACTION_TYPE_MASK) {
		case VFIO_IRQ_SET_ACTION_MASK:
		case VFIO_IRQ_SET_ACTION_UNMASK:
			break;
		case VFIO_IRQ_SET_ACTION_TRIGGER:
		{
			if (flags & VFIO_IRQ_SET_DATA_NONE) {
				pr_info("%s: disable INTx\n", __func__);
				if (mdev_state->intx_evtfd)
					eventfd_ctx_put(mdev_state->intx_evtfd);
				break;
			}

			if (flags & VFIO_IRQ_SET_DATA_EVENTFD) {
				int fd = *(int *)data;

				if (fd > 0) {
					struct eventfd_ctx *evt;

					evt = eventfd_ctx_fdget(fd);
					if (IS_ERR(evt)) {
						ret = PTR_ERR(evt);
						break;
					}
					mdev_state->intx_evtfd = evt;
					mdev_state->irq_fd = fd;
					mdev_state->irq_index = index;
					break;
				}
			}
			break;
		}
		}
		break;
	case VFIO_PCI_MSI_IRQ_INDEX:
		switch (flags & VFIO_IRQ_SET_ACTION_TYPE_MASK) {
		case VFIO_IRQ_SET_ACTION_MASK:
		case VFIO_IRQ_SET_ACTION_UNMASK:
			break;
		case VFIO_IRQ_SET_ACTION_TRIGGER:
			if (flags & VFIO_IRQ_SET_DATA_NONE) {
				if (mdev_state->msi_evtfd)
					eventfd_ctx_put(mdev_state->msi_evtfd);
				pr_info("%s: disable MSI\n", __func__);
				mdev_state->irq_index = VFIO_PCI_INTX_IRQ_INDEX;
				break;
			}
			if (flags & VFIO_IRQ_SET_DATA_EVENTFD) {
				int fd = *(int *)data;
				struct eventfd_ctx *evt;

				if (fd <= 0)
					break;

				if (mdev_state->msi_evtfd)
					break;

				evt = eventfd_ctx_fdget(fd);
				if (IS_ERR(evt)) {
					ret = PTR_ERR(evt);
					break;
				}
				mdev_state->msi_evtfd = evt;
				mdev_state->irq_fd = fd;
				mdev_state->irq_index = index;
			}
			break;
	}
	break;
	case VFIO_PCI_MSIX_IRQ_INDEX:
		pr_info("%s: MSIX_IRQ\n", __func__);
		break;
	case VFIO_PCI_ERR_IRQ_INDEX:
		pr_info("%s: ERR_IRQ\n", __func__);
		break;
	case VFIO_PCI_REQ_IRQ_INDEX:
		pr_info("%s: REQ_IRQ\n", __func__);
		break;
	}

	mutex_unlock(&mdev_state->ops_lock);
	return ret;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0)
static int mtty_trigger_interrupt(uuid_le uuid)
{
	int ret = -1;
	struct mdev_state *mdev_state;

	mdev_state = find_mdev_state_by_uuid(uuid);

	if (!mdev_state) {
		pr_info("%s: mdev not found\n", __func__);
		return -EINVAL;
	}
#else
static int mtty_trigger_interrupt(struct mdev_state *mdev_state)
{
	int ret = -1;
#endif
	if ((mdev_state->irq_index == VFIO_PCI_MSI_IRQ_INDEX) &&
	    (!mdev_state->msi_evtfd))
		return -EINVAL;
	else if ((mdev_state->irq_index == VFIO_PCI_INTX_IRQ_INDEX) &&
		 (!mdev_state->intx_evtfd)) {
		pr_info("%s: Intr eventfd not found\n", __func__);
		return -EINVAL;
	}

	if (mdev_state->irq_index == VFIO_PCI_MSI_IRQ_INDEX)
		ret = eventfd_signal(mdev_state->msi_evtfd, 1);
	else
		ret = eventfd_signal(mdev_state->intx_evtfd, 1);

#if defined(DEBUG_INTR)
	pr_info("Intx triggered\n");
#endif
	if (ret != 1)
		pr_err("%s: eventfd signal failed (%d)\n", __func__, ret);

	return ret;
}

static int mtty_get_region_info(struct mdev_device *mdev,
			 struct vfio_region_info *region_info,
			 u16 *cap_type_id, void **cap_type)
{
	unsigned int size = 0;
	struct mdev_state *mdev_state;
	u32 bar_index;

	if (!mdev)
		return -EINVAL;

	mdev_state = mdev_get_drvdata(mdev);
	if (!mdev_state)
		return -EINVAL;

	bar_index = region_info->index;
	if (bar_index >= VFIO_PCI_NUM_REGIONS)
		return -EINVAL;

	mutex_lock(&mdev_state->ops_lock);

	switch (bar_index) {
	case VFIO_PCI_CONFIG_REGION_INDEX:
		size = MTTY_CONFIG_SPACE_SIZE;
		break;
	case VFIO_PCI_BAR0_REGION_INDEX:
		size = MTTY_IO_BAR_SIZE;
		break;
	case VFIO_PCI_BAR1_REGION_INDEX:
		if (mdev_state->nr_ports == 2)
			size = MTTY_IO_BAR_SIZE;
		break;
	default:
		size = 0;
		break;
	}

	mdev_state->region_info[bar_index].size = size;
	mdev_state->region_info[bar_index].vfio_offset =
		MTTY_VFIO_PCI_INDEX_TO_OFFSET(bar_index);

	region_info->size = size;
	region_info->offset = MTTY_VFIO_PCI_INDEX_TO_OFFSET(bar_index);
	region_info->flags = VFIO_REGION_INFO_FLAG_READ |
		VFIO_REGION_INFO_FLAG_WRITE;
	mutex_unlock(&mdev_state->ops_lock);
	return 0;
}

static int mtty_get_irq_info(struct mdev_device *mdev,
			     struct vfio_irq_info *irq_info)
{
	switch (irq_info->index) {
	case VFIO_PCI_INTX_IRQ_INDEX:
	case VFIO_PCI_MSI_IRQ_INDEX:
	case VFIO_PCI_REQ_IRQ_INDEX:
		break;

	default:
		return -EINVAL;
	}

	irq_info->flags = VFIO_IRQ_INFO_EVENTFD;
	irq_info->count = 1;

	if (irq_info->index == VFIO_PCI_INTX_IRQ_INDEX)
		irq_info->flags |= (VFIO_IRQ_INFO_MASKABLE |
				VFIO_IRQ_INFO_AUTOMASKED);
	else
		irq_info->flags |= VFIO_IRQ_INFO_NORESIZE;

	return 0;
}

static int mtty_get_device_info(struct mdev_device *mdev,
			 struct vfio_device_info *dev_info)
{
	dev_info->flags = VFIO_DEVICE_FLAGS_PCI;
	dev_info->num_regions = VFIO_PCI_NUM_REGIONS;
	dev_info->num_irqs = VFIO_PCI_NUM_IRQS;

	return 0;
}

static long mtty_ioctl(struct mdev_device *mdev, unsigned int cmd,
			unsigned long arg)
{
	int ret = 0;
	unsigned long minsz;
	struct mdev_state *mdev_state;

	if (!mdev)
		return -EINVAL;

	mdev_state = mdev_get_drvdata(mdev);
	if (!mdev_state)
		return -ENODEV;

	switch (cmd) {
	case VFIO_DEVICE_GET_INFO:
	{
		struct vfio_device_info info;

		minsz = offsetofend(struct vfio_device_info, num_irqs);

		if (copy_from_user(&info, (void __user *)arg, minsz))
			return -EFAULT;

		if (info.argsz < minsz)
			return -EINVAL;

		ret = mtty_get_device_info(mdev, &info);
		if (ret)
			return ret;

		memcpy(&mdev_state->dev_info, &info, sizeof(info));

		if (copy_to_user((void __user *)arg, &info, minsz))
			return -EFAULT;

		return 0;
	}
	case VFIO_DEVICE_GET_REGION_INFO:
	{
		struct vfio_region_info info;
		u16 cap_type_id = 0;
		void *cap_type = NULL;

		minsz = offsetofend(struct vfio_region_info, offset);

		if (copy_from_user(&info, (void __user *)arg, minsz))
			return -EFAULT;

		if (info.argsz < minsz)
			return -EINVAL;

		ret = mtty_get_region_info(mdev, &info, &cap_type_id,
					   &cap_type);
		if (ret)
			return ret;

		if (copy_to_user((void __user *)arg, &info, minsz))
			return -EFAULT;

		return 0;
	}

	case VFIO_DEVICE_GET_IRQ_INFO:
	{
		struct vfio_irq_info info;

		minsz = offsetofend(struct vfio_irq_info, count);

		if (copy_from_user(&info, (void __user *)arg, minsz))
			return -EFAULT;

		if ((info.argsz < minsz) ||
		    (info.index >= mdev_state->dev_info.num_irqs))
			return -EINVAL;

		ret = mtty_get_irq_info(mdev, &info);
		if (ret)
			return ret;

		if (copy_to_user((void __user *)arg, &info, minsz))
			return -EFAULT;

		return 0;
	}
	case VFIO_DEVICE_SET_IRQS:
	{
		struct vfio_irq_set hdr;
		u8 *data = NULL, *ptr = NULL;
		size_t data_size = 0;

		minsz = offsetofend(struct vfio_irq_set, count);

		if (copy_from_user(&hdr, (void __user *)arg, minsz))
			return -EFAULT;

		ret = vfio_set_irqs_validate_and_prepare(&hdr,
						mdev_state->dev_info.num_irqs,
						VFIO_PCI_NUM_IRQS,
						&data_size);
		if (ret)
			return ret;

		if (data_size) {
			ptr = data = memdup_user((void __user *)(arg + minsz),
						 data_size);
			if (IS_ERR(data))
				return PTR_ERR(data);
		}

		ret = mtty_set_irqs(mdev, hdr.flags, hdr.index, hdr.start,
				    hdr.count, data);

		kfree(ptr);
		return ret;
	}
	case VFIO_DEVICE_RESET:
		return mtty_reset(mdev);
	}
	return -ENOTTY;
}

static int mtty_open(struct mdev_device *mdev)
{
	pr_info("%s\n", __func__);
	return 0;
}

static void mtty_close(struct mdev_device *mdev)
{
	pr_info("%s\n", __func__);
}

static ssize_t
sample_mtty_dev_show(struct device *dev, struct device_attribute *attr,
		     char *buf)
{
	return sprintf(buf, "This is phy device\n");
}

static DEVICE_ATTR_RO(sample_mtty_dev);

static struct attribute *mtty_dev_attrs[] = {
	&dev_attr_sample_mtty_dev.attr,
	NULL,
};

static const struct attribute_group mtty_dev_group = {
	.name  = "mtty_dev",
	.attrs = mtty_dev_attrs,
};

static const struct attribute_group *mtty_dev_groups[] = {
	&mtty_dev_group,
	NULL,
};

static ssize_t
sample_mdev_dev_show(struct device *dev, struct device_attribute *attr,
		     char *buf)
{
	if (mdev_from_dev(dev))
		return sprintf(buf, "This is MDEV %s\n", dev_name(dev));

	return sprintf(buf, "\n");
}

static DEVICE_ATTR_RO(sample_mdev_dev);

static struct attribute *mdev_dev_attrs[] = {
	&dev_attr_sample_mdev_dev.attr,
	NULL,
};

static const struct attribute_group mdev_dev_group = {
	.name  = "vendor",
	.attrs = mdev_dev_attrs,
};

static const struct attribute_group *mdev_dev_groups[] = {
	&mdev_dev_group,
	NULL,
};

static ssize_t
name_show(struct kobject *kobj, struct device *dev, char *buf)
{
	char name[MTTY_STRING_LEN];
	int i;
	const char *name_str[2] = {"Single port serial", "Dual port serial"};

	for (i = 0; i < 2; i++) {
		snprintf(name, MTTY_STRING_LEN, "%s-%d",
			 dev_driver_string(dev), i + 1);
		if (!strcmp(kobj->name, name))
			return sprintf(buf, "%s\n", name_str[i]);
	}

	return -EINVAL;
}

static MDEV_TYPE_ATTR_RO(name);

static ssize_t
available_instances_show(struct kobject *kobj, struct device *dev, char *buf)
{
	char name[MTTY_STRING_LEN];
	int i;
	struct mdev_state *mds;
	int ports = 0, used = 0;

	for (i = 0; i < 2; i++) {
		snprintf(name, MTTY_STRING_LEN, "%s-%d",
			 dev_driver_string(dev), i + 1);
		if (!strcmp(kobj->name, name)) {
			ports = i + 1;
			break;
		}
	}

	if (!ports)
		return -EINVAL;

	list_for_each_entry(mds, &mdev_devices_list, next)
		used += mds->nr_ports;

	return sprintf(buf, "%d\n", (MAX_MTTYS - used)/ports);
}

static MDEV_TYPE_ATTR_RO(available_instances);


static ssize_t device_api_show(struct kobject *kobj, struct device *dev,
			       char *buf)
{
	return sprintf(buf, "%s\n", VFIO_DEVICE_API_PCI_STRING);
}

static MDEV_TYPE_ATTR_RO(device_api);

static struct attribute *mdev_types_attrs[] = {
	&mdev_type_attr_name.attr,
	&mdev_type_attr_device_api.attr,
	&mdev_type_attr_available_instances.attr,
	NULL,
};

static struct attribute_group mdev_type_group1 = {
	.name  = "1",
	.attrs = mdev_types_attrs,
};
/*
static struct attribute_group mdev_type_group2 = {
	.name  = "2",
	.attrs = mdev_types_attrs,
};
*/
static struct attribute_group *mdev_type_groups[] = {
	&mdev_type_group1,
	// &mdev_type_group2,
	NULL,
};

static const struct mdev_parent_ops mdev_fops = {
	.owner                  = THIS_MODULE,
	.dev_attr_groups        = mtty_dev_groups,
	.mdev_attr_groups       = mdev_dev_groups,
	.supported_type_groups  = mdev_type_groups,
	.create                 = mtty_create,
	.remove			        = mtty_remove,
	.open                   = mtty_open,
	.release                = mtty_close,
	.read                   = mtty_read,
	.write                  = mtty_write,
	.ioctl		        = mtty_ioctl,
};

static void mtty_device_release(struct device *dev)
{
	dev_dbg(dev, "mtty: released\n");
}

static int __init mtty_dev_init(void)
{
	int ret = 0;

	pr_info("mtty_dev: %s\n", __func__);

	memset(&mtty_dev, 0, sizeof(mtty_dev));

	idr_init(&mtty_dev.vd_idr);

	ret = alloc_chrdev_region(&mtty_dev.vd_devt, 0, MINORMASK + 1,
				  MTTY_NAME);

	if (ret < 0) {
		pr_err("Error: failed to register mtty_dev, err:%d\n", ret);
		return ret;
	}

	cdev_init(&mtty_dev.vd_cdev, &vd_fops);
	cdev_add(&mtty_dev.vd_cdev, mtty_dev.vd_devt, MINORMASK + 1);

	pr_info("major_number:%d\n", MAJOR(mtty_dev.vd_devt));

	mtty_dev.vd_class = class_create(THIS_MODULE, MTTY_CLASS_NAME);

	if (IS_ERR(mtty_dev.vd_class)) {
		pr_err("Error: failed to register mtty_dev class\n");
		ret = PTR_ERR(mtty_dev.vd_class);
		goto failed1;
	}

	mtty_dev.dev.class = mtty_dev.vd_class;
	mtty_dev.dev.release = mtty_device_release;
	dev_set_name(&mtty_dev.dev, "%s", MTTY_NAME);

	ret = device_register(&mtty_dev.dev);
	if (ret)
		goto failed2;

	ret = mdev_register_device(&mtty_dev.dev, &mdev_fops);
	if (ret)
		goto failed3;

	mutex_init(&mdev_list_lock);
	INIT_LIST_HEAD(&mdev_devices_list);

	goto all_done;

failed3:

	device_unregister(&mtty_dev.dev);
failed2:
	class_destroy(mtty_dev.vd_class);

failed1:
	cdev_del(&mtty_dev.vd_cdev);
	unregister_chrdev_region(mtty_dev.vd_devt, MINORMASK + 1);

all_done:
	return ret;
}

static void __exit mtty_dev_exit(void)
{
	mtty_dev.dev.bus = NULL;
	mdev_unregister_device(&mtty_dev.dev);

	device_unregister(&mtty_dev.dev);
	idr_destroy(&mtty_dev.vd_idr);
	cdev_del(&mtty_dev.vd_cdev);
	unregister_chrdev_region(mtty_dev.vd_devt, MINORMASK + 1);
	class_destroy(mtty_dev.vd_class);
	mtty_dev.vd_class = NULL;
	pr_info("mtty_dev: Unloaded!\n");
}

module_init(mtty_dev_init)
module_exit(mtty_dev_exit)

MODULE_LICENSE("GPL v2");
MODULE_INFO(supported, "Test driver that simulate serial port over PCI");
MODULE_VERSION(VERSION_STRING);
MODULE_AUTHOR(DRIVER_AUTHOR);
