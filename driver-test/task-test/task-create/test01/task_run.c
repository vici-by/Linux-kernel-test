/*************************************************************************
	> File Name: kfifo_test.c
	> Author: baiy
	> Mail: baiyang0223@163.com
	> Created Time: 2020-12-04-17:19:54
	> Func:   kthread_create and kthread_stop test
 ************************************************************************/
#define pr_fmt(fmt) "[%s:%d:]" fmt, __func__, __LINE__
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/notifier.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 3, 0)
	#include <asm/switch_to.h>
#else
	#include <asm/system.h>
#endif

#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/kfifo.h>



#define TASK_NUM (4)

struct task_run_info {
	struct list_head head;
	struct mutex mutex;
	struct task_struct *task_thread;
	wait_queue_head_t wait;
	atomic_t taskID;
};
struct task_run_info task[TASK_NUM];


static int  task_run_thread(void * data)
{
	unsigned int count  = 10;
	struct task_run_info *cur_task = data;

	while (!kthread_should_stop()) {
		pr_info("This is [%#x] thread, task [%pK]\n", atomic_read(&cur_task->taskID), data);
		msleep(3000);
	}
	pr_info("This is [%#x] thread, task [%pK] Quit\n", atomic_read(&cur_task->taskID), data);

	return 0;
}


static int __init task_run_init(void)
{
	int i;

	pr_info("task_run_init init[E]");
	for (i=0; i<TASK_NUM; ++i) {
		pr_info("Thread[%#x], task [%pK]\n", atomic_read(&task[i].taskID), &task[i]);
		mutex_init(&task[i].mutex);
		INIT_LIST_HEAD(&task[i].head);
		atomic_set(&task[i].taskID, i);

		task[i].task_thread = kthread_run(task_run_thread, &task[i], "task_run");
	}
	pr_info("task_run_init init[X]");

	return 0;
}
module_init(task_run_init);


static void __exit task_run_exit(void)
{
	int i;

	pr_info("task_run_exit exit[E]");
	for (i=0; i<TASK_NUM; ++i) {
		kthread_stop(task[i].task_thread);
	}
	pr_info("task_run_exit exit[X]");
}
module_exit(task_run_exit);


MODULE_LICENSE("GPL v2");
MODULE_INFO(supported, "Test Thread create");

