
#define pr_fmt(fmt) "PROC_DEBUG: " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>

#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/version.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>



#define PROC_DEBUG_DIR   		"test_proc"
#define PROC_DEBUG_FILE   		"test_file"
static int g_test = 0;
struct proc_dir_entry *proc_dir, *proc_file;



static int debug_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "0x%x\n", g_test);
    return 0;
}

static int debug_proc_open(struct inode *inode, struct  file *file)
{
    return single_open(file, debug_proc_show, NULL);
}

ssize_t debug_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
    unsigned char temp_buf[128];
    unsigned int len = 0;
    if (count >= 128) {
        len = 127;
    } else {
        len = count;
    }
    if(copy_from_user(temp_buf, buffer, len))
        return -EFAULT;
    temp_buf[len] = '\0';
    g_test = simple_strtoul(temp_buf, NULL, 10);
    return count;
}

static struct file_operations debug_proc_ops = {
    .owner   = THIS_MODULE,
    .open    = debug_proc_open,
    .read    = seq_read,
    .write   = debug_proc_write,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int init_debug_port(void)
{
	// 参数为NULL， 默认在/proc下创建目录

	
	proc_dir = proc_mkdir(PROC_DEBUG_DIR, NULL);
    if (NULL == proc_dir) {
        printk(KERN_ALERT"Create dir /proc/%s error!\n", PROC_DEBUG_DIR);
        return -1;
    }
	    // Create a test entryunder USER_ROOT_DIR
    proc_file = proc_create(PROC_DEBUG_FILE, S_IRWXUGO, proc_dir, &debug_proc_ops);
    if (NULL == proc_file) {
        printk(KERN_ALERT"Create entry %s under /proc/%s error!\n",
               PROC_DEBUG_FILE, PROC_DEBUG_DIR);
        goto err_out;
    }
	return 0;
err_out1:
    remove_proc_entry(PROC_DEBUG_FILE, proc_dir);
err_out:
    remove_proc_entry(PROC_DEBUG_DIR, NULL);

	return -1;
}


static void exit_debug_port(void)
{
    remove_proc_entry(PROC_DEBUG_FILE, proc_dir);
    remove_proc_entry(PROC_DEBUG_DIR, NULL);	
}


static int __init hello_init(void)
{
    printk(KERN_INFO"%s,%s,line=%d\n",__FILE__,__func__,__LINE__);
    pr_info("hello init\n");
	init_debug_port();
    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_INFO"%s,%s,line=%d\n",__FILE__,__func__,__LINE__);
    pr_info("hello exit\n");
	exit_debug_port();

    return;
}

module_init(hello_init);
module_exit(hello_exit); 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("baiy <baiyang0223@163.com>");
MODULE_DESCRIPTION("This is a test driver");
MODULE_VERSION("1.0.0.0");
