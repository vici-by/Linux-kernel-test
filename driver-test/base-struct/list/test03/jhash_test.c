/*************************************************************************
    > File Name: jhash_test.c
    > Author: baiy
    > Mail: baiyang0223@163.com
    > Created Time: 2021-04-06-14:34:52
    > Func:  测试Linux 内核jhash, 参考文档：
    > https://www.yuque.com/docs/share/779a01b2-8660-40ed-8289-3feaf1b60f53?#
 ************************************************************************/
#define pr_fmt(fmt) "[%s:%d]: " fmt, __func__, __LINE__
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/notifier.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/random.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 3, 0)
    #include <asm/switch_to.h>
#else
    #include <asm/system.h>
#endif

#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/jhash.h>


//常规算法的黄金分隔
#define VOICE_HASH_GOLDEN_INTERER   (0x9e370001)

//hash桶的大小为2的11次方，即2047+1
#define HASH_SIZE                   2048


//常用的黄金分隔的hash算法
#define VOICE_HASH_INTERGER(uiKey,uiHash) \
{\
    uiHash = uiKey * (VOICE_HASH_GOLDEN_INTERER); \
    uiHash = uiHash >> 21;\
}



//统计指定节点数量的hash桶冲突情况
void show_hash_node(int num,int *TestArray,int IsJhash)
{
    int conflic_num = 0;
    char *pcHashName = "common";
    int i = 0;
    for(i=0;i<HASH_SIZE;i++)
    {
        if(num == TestArray[i])
        {
            conflic_num++;
        }
    }
    if (1 == IsJhash)
    {
        pcHashName = "Jhash";
    }

    pr_info("算法%s:节点数为%d的hash桶的个数为:%d\n",pcHashName,num,conflic_num);
    return;
}

//显示hash冲突的情况
void show_hash_conflict(int *TestArray,int IsJhash)
{
    //统计节点数为0到9的hash桶冲突个数
    int i = 0;
    for(i=0;i<10;i++)
    {
        show_hash_node(i,TestArray,IsJhash);
    }
    return;
}



static void jhash_test(void)
{
	int index	   =0;
	long HashIndex =0;
	long JHashIndex=0;

	//测试key样本为0到1024
	int MaxIndex   =1024;

	//每一个HashIndex对应数组里的一个节点
	int * TestArray1 = kzalloc(HASH_SIZE, GFP_KERNEL);
	int * TestArray2 = kzalloc(HASH_SIZE, GFP_KERNEL);
	memset(TestArray1,0,sizeof(HASH_SIZE));
	memset(TestArray2,0,sizeof(HASH_SIZE));

	for (index=0; index<MaxIndex; index++)
	{
		//使用传统Hash算法计算HashIndex
		VOICE_HASH_INTERGER(index,HashIndex);
		//在数组对应的节点+1,模拟实际项目中下挂的数据结构
		TestArray1[HashIndex] +=1;

		//使用Jhash算法计算JHashIndex
		JHashIndex = jhash_1word(index, VOICE_HASH_GOLDEN_INTERER);
		//只取指定位数的
		JHashIndex = JHashIndex & (HASH_SIZE-1);
		//在数组对应的节点+1,模拟实际项目中下挂的数据结构
		TestArray2[JHashIndex] +=1;
	}

	//统计传统的Hash冲突的情况
	show_hash_conflict(TestArray1,0);
	//统计JHash冲突的情况
	show_hash_conflict(TestArray2,1);

	kfree(TestArray1);
	kfree(TestArray2);
}


static int __init jhash_dev_init(void)
{
	pr_info("jhash init[E]");
	jhash_test();
	pr_info("jhash init[X]");
    return 0;
}
module_init(jhash_dev_init);

static void __exit jhash_dev_exit(void)
{
	pr_info("jhash exit");
}
module_exit(jhash_dev_exit);

MODULE_LICENSE("GPL v2");
MODULE_INFO(supported, "Test driver that simulate serial port over PCI");

