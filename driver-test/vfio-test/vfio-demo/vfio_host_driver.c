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
#include <linux/bitmap.h>
#include <linux/kthread.h>
#include <linux/workqueue.h>
// #include <asm-generic/atomic64.h>

#include "vfio_msg.h"

#define AXI_DMA_CHANNEL_MAX (16)

typedef struct VF_DESC{
    struct mutex msg_lock;
    struct list_head msg_head;
    void (*msg_func)(void *);
}VF_DESC;


typedef struct VF_DESC_INFO{
    unsigned long statbit[BITS_TO_LONGS(AXI_DMA_CHANNEL_MAX)];  // VF 状态的bitmap
    unsigned long msgbit[BITS_TO_LONGS(AXI_DMA_CHANNEL_MAX)];   // VF 消息的bitmap
    VF_DESC vf_desc_arr[AXI_DMA_CHANNEL_MAX];                   // VF 的描述信息

    // 消息处理相关
    struct task_struct *msg_thread;                             // 消息处理线程
    atomic64_t   thread_stat;
    wait_queue_head_t   msg_queue;                              // 等待队列
    atomic64_t          msg_cnt;
    struct mutex msgbitlock;
}VF_DESC_INFO;
struct VF_DESC_INFO * pvf_desc_info;

struct work_struct work;

void vfio_add_msg(unsigned int channel, VFIO_MSG_LIST * msgl)
{
    struct VFIO_MSG      * msg;

#if 0
    unsigned long        * statbit;
    unsigned long        * msgbit;

    statbit = &(pvf_desc_info->statbit[0]);  // TBD: 这里兼容性不好，需要计算所在的unsigned long
    msgbit  = &(pvf_desc_info->msgbit[0]);  // TBD: 这里兼容性不好，需要计算所在的unsigned long

    mutex_lock(&pvf_desc_info->msgbitlock);
    if((0 == (*statbit & (1<<(channel % BITS_PER_LONG)))) || (msgl == NULL)) {
        mutex_unlock(&pvf_desc_info->msgbitlock);
        pr_info("INVAILD CHANNEL : %d,msgl is %#lx\n",channel,(unsigned long)msgl);
        if(msgl)
            kfree(msgl);
        return;
    }
    pr_info("msgl is %#lx,channel is %d\n",(unsigned long)msgl,channel);
    list_add_tail(&(msgl->list), &(pvf_desc_info->vf_desc_arr[channel].msg_head));
    pr_info("start bitmap is %#lx\n",*msgbit);
    __set_bit(channel, msgbit);
    pr_info("end bitmap is %#lx\n",*msgbit);
    atomic64_inc(&(pvf_desc_info->msg_cnt));
    mutex_unlock(&pvf_desc_info->msgbitlock);
    msg = &(msgl->msg);
    pr_info("PUSH msg magic is %#x, cmd is %#x\n",msg->magic,msg->cmd);
    wake_up_interruptible(&(pvf_desc_info->msg_queue));
#else
    msg = &(msgl->msg);
    pr_info("PUSH msg magic is %#x, cmd is %#x\n",msg->magic,msg->cmd);
    switch(msg->cmd){
        case CMD_CHANNEL_START:
            //....
            pr_info("CMD CHANNEL START");
            break;
        case CMD_CHANNEL_STOP:
            //....
            pr_info("CMD CHANNEL STOP");
            break;
        default:
            pr_err("msg magic is %#x, cmd is %#x\n",msg->magic,msg->cmd);
            pr_err("Invaild CMD \n");
            break;
    }
#endif
}
EXPORT_SYMBOL(vfio_add_msg);

static void vfio_msg_fn(void * data)
{
    struct VFIO_MSG_LIST * msgl, *tmpmsgl;
    struct VFIO_MSG      * msg;

    struct VF_DESC *desc  = (struct VF_DESC *)data;
    mutex_lock(&desc->msg_lock);
    list_for_each_entry_safe(msgl,tmpmsgl, &desc->msg_head, list){
        msg = &(msgl->msg);
        pr_info("POP msg  magic is %#x, cmd is %#x\n",msg->magic,msg->cmd);
        //。。。。。。。 TBD
        atomic64_dec(&(pvf_desc_info->msg_cnt));
        list_del(&(msgl->list));
        kfree(msgl);
    }
    mutex_unlock(&desc->msg_lock);
}

static int vfio_msg_thread(void *data)
{
    struct VF_DESC_INFO * pdesc_info;
    int i;
    int err;
    unsigned long idx;

    pdesc_info = (struct VF_DESC_INFO * )data;
    while(1){
#if 0
        err = wait_event_interruptible_timeout(pdesc_info->msg_queue,
            atomic64_read(&pdesc_info->msg_cnt),  msecs_to_jiffies(500));

        pr_info("wait return %d\n",err);
        if(err < 0){
            pr_info("thread receive signal, quit\n");
            return -1;
        }
#else
        msleep(1000);
#endif
        if(atomic64_read(&pdesc_info->thread_stat)) {
            pr_info("Thread end");
            return 0;
        }
        while(atomic64_read(&pdesc_info->msg_cnt)){
            mutex_lock(&pdesc_info->msgbitlock);
            idx = find_first_bit(pdesc_info->msgbit, AXI_DMA_CHANNEL_MAX);
            __clear_bit(idx, pdesc_info->msgbit);

            mutex_unlock(&pdesc_info->msgbitlock);
            if(idx < AXI_DMA_CHANNEL_MAX){
                pdesc_info->vf_desc_arr[idx].msg_func(&pdesc_info->vf_desc_arr[idx]);
            } else {
                pr_err("err:msg cnt is %lld, but bitmap is 0\n",atomic64_read(&pdesc_info->msg_cnt));
                atomic64_set(&pdesc_info->msg_cnt, 0);
                break;
            }
        }
    }
/*
    while(!thread_could_stop()){
           wait();
    }
*/
    return 0;
}

static int __init vfio_host_driver_init(void)
{
    int i;
    int err;

    // 1. 初始化相关结构体
    pvf_desc_info = kzalloc(sizeof(VF_DESC_INFO), GFP_KERNEL);
    if(!pvf_desc_info){
        pr_err("Short of memory");
        return -ENOMEM;
    }
    bitmap_zero(pvf_desc_info->statbit, BITS_TO_LONGS(AXI_DMA_CHANNEL_MAX) * BITS_PER_LONG);
    bitmap_zero(pvf_desc_info->msgbit,  BITS_TO_LONGS(AXI_DMA_CHANNEL_MAX) * BITS_PER_LONG);
    for(i=0; i<AXI_DMA_CHANNEL_MAX; ++i){
        mutex_init(&(pvf_desc_info->vf_desc_arr[i].msg_lock));
        INIT_LIST_HEAD(&(pvf_desc_info->vf_desc_arr[i].msg_head));
        pvf_desc_info->vf_desc_arr[i].msg_func = vfio_msg_fn;
    }
    init_waitqueue_head(&pvf_desc_info->msg_queue);
    atomic64_set(&pvf_desc_info->msg_cnt, 0);
    atomic64_set(&pvf_desc_info->thread_stat, 0);
    mutex_init(&(pvf_desc_info->msgbitlock));

    // 2.初始化自己的相关channel
    // ...这里假设16个通道都是OK的，所以都置1了
    __bitmap_set(pvf_desc_info->statbit, 0, AXI_DMA_CHANNEL_MAX);


    // 3.初始化mtty接口


    // 4.开启消息处理线程
    // pvf_desc_info->msg_thread = kthread_run(vfio_msg_thread,pvf_desc_info,"mtty msg");

    return 0;



err1:
    for(i=0; i<AXI_DMA_CHANNEL_MAX; ++i){
        mutex_destroy(&(pvf_desc_info->vf_desc_arr[i].msg_lock));
    }
err:
    if(pvf_desc_info){
        kfree(pvf_desc_info);
        pvf_desc_info = NULL;
    }

    return err;
}
static void __exit vfio_host_driver_exit(void)
{
    int i;
    int err;
    // kthread_stop(pvf_desc_info->msg_thread);

    // TBD: 这种方法比较low，需要改善
    // atomic64_set(&pvf_desc_info->thread_stat, 1);
    // wake_up_interruptible(&(pvf_desc_info->msg_queue));

    msleep(1000);
    for(i=0; i<AXI_DMA_CHANNEL_MAX; ++i){
        mutex_destroy(&(pvf_desc_info->vf_desc_arr[i].msg_lock));
    }

    if(pvf_desc_info){
        kfree(pvf_desc_info);
        pvf_desc_info = NULL;
    }
}

module_init(vfio_host_driver_init)
module_exit(vfio_host_driver_exit)

MODULE_LICENSE("GPL v2");
