/*************************************************************************
    > File Name: hlist_test.c
    > Author: baiy
    > Mail: baiyang0223@163.com
    > Created Time: 2021-04-06-14:34:52
    > Func:  测试Linux 内核hlist链表, 参考文档：
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


struct test_hlist{
	unsigned int nr;
	struct	hlist_node node;
};

#define NR_TEST_HASH	(2)
static  struct hlist_head test_lists[NR_TEST_HASH];  // hash表数组


static void hlist_test(void)
{
	int i = 0;

	pr_info("Test HList [E]\n");

	// 初始化Hlist链表头
	for (i=0; i<NR_TEST_HASH; ++i) {
		INIT_HLIST_HEAD(&test_lists[i]);
	}

	// 申请并添加哈希表
	pr_info("create hlist\n");
	for (i=0; i<30; ++i){
		// 生成NR_TEST_HASH以内得随机数，然后添加到自己得链表中
		struct  test_hlist *l = NULL;
		unsigned int index = get_random_int() & (NR_TEST_HASH - 1);
		l = kzalloc(sizeof(struct test_hlist), GFP_KERNEL);
		INIT_HLIST_NODE(&l->node);
		l->nr = i;
		hlist_add_head(&l->node, &test_lists[index]);
		pr_info("l->nr is %d insert hlist %d", i, index);
	}


	// 哈希表的遍历
	pr_info("traversal hlist\n");
	{
		for (i=0; i<NR_TEST_HASH; ++i){
			struct  test_hlist *l = NULL;
			pr_info("traversal hlist %d", i);
			hlist_for_each_entry(l, &test_lists[i], node ) {
				pr_info("l->nr is %d\n", l->nr);
			}
		}
	}


	// hlist的添加
	// hlist_add_head 	添加到链表头部或尾部

	// hlist的删除
	// hlist_del / hlist_del_init  删除并对其初始化

	// hlist的判空
	// hlist_empty 直接判断 READ_ONCE(head->first) == NULL; 即可


	// 释放链表
	pr_info("delete hlist\n");
	{
		for (i=0; i<NR_TEST_HASH; ++i){
			struct  test_hlist *l = NULL;
			struct hlist_node  *n = NULL;
			pr_info("delete hlist %d", i);
			hlist_for_each_entry_safe(l, n, &test_lists[i], node ) {
				hlist_del(&l->node);
				kfree(l);
			}
		}
	}

	pr_info("Test HList [X]\n");
}

static int __init hlist_dev_init(void)
{
	pr_info("hlist init[E]");
	hlist_test();
	pr_info("hlist init[X]");
    return 0;
}
module_init(hlist_dev_init);

static void __exit hlist_dev_exit(void)
{
	pr_info("hlist exit");
}
module_exit(hlist_dev_exit);

MODULE_LICENSE("GPL v2");
MODULE_INFO(supported, "Test driver that simulate serial port over PCI");

