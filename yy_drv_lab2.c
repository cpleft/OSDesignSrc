#include<linux/kernel.h>        /* open(), release() */
#include<linux/init.h>
#include<linux/fs.h>            /* ioctl(), read(), write() */
#include<linux/module.h>
#include<linux/slab.h>          /* kmalloc(), kfree() */
#include<linux/uaccess.h>   	/* copy_to_user(), copy_from_user() */
#include<linux/mutex.h>         /* struct mutex */
#include<linux/seq_file.h>      
#include<linux/proc_fs.h>     	/* proc_create(), remove_proc_entry() */

#define VERSION     "yy_drv_lab2"
#define PROC_NAME   "yy_proc"
#define MAX_SIZE    1024

static int n = MAX_SIZE;
module_param(n, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(n, "size of kernel data buffer.");

static struct yy_proc_data {
    char buf[MAX_SIZE];
    struct yy_proc_data* next;
};
static loff_t yy_proc_data_size = 0;/* 记录数据链表中的结构体一共有几个 */
static struct yy_proc_data* head = NULL;
static struct yy_proc_data* tail = NULL;

static void* yy_seq_start(struct seq_file* sfp, loff_t* pos)
{
    printk("yy_seq_start[--kernel--]\n");

    struct yy_proc_data* tmp = head;
    int index = *pos;

    if (!*pos)
        return head;
    while (index--)
    {
        if (!tmp)
            return NULL;
        tmp = tmp->next;
    }
    return tmp;
}

static void* yy_seq_next(struct seq_file* sfp, void* v, loff_t* pos)
{
    printk("yy_seq_next[--kernel--]\n");
    struct yy_proc_data* tmp = (struct yy_proc_data*)v;

    *pos = *pos + 1;
    tmp = tmp->next;
    
    if (!tmp)
        return NULL;

    return tmp;
}

static void yy_seq_show(struct seq_file* sfp, void* v)
{
    printk("yy_seq_show[--kernel--]\n");
    
    if (v == NULL)
        return;
    struct yy_proc_data* tmp = (struct yy_proc_data*)v;

    seq_printf(sfp, "%s(yy_seq_show)\n", tmp->buf);
}

static void yy_seq_stop(struct seq_file* sfp, void* v)
{
    printk("ct_seq_stop!\n");
    return;
}

static struct seq_operations yy_seq_ops = {
    .start = yy_seq_start,
    .next = yy_seq_next,
    .show = yy_seq_show,
    .stop = yy_seq_stop
};


static int yy_open(struct inode* ip, struct file* fp)
{
    return seq_open(fp, &yy_seq_ops);
}

static ssize_t yy_write(
        struct file* fp, 
        const char* buf, 
        size_t count, 
        loff_t* pos)
{
    struct yy_proc_data* yy_proc_data_new = (struct yy_proc_data*)
        kmalloc(sizeof(struct yy_proc_data), GFP_KERNEL);
    if (yy_proc_data_new == NULL)
    {
        printk("kmalloc for yy_proc_data_new failed.\n");
        return -1;
    }

    int write_size = count;
    if (count > n)
        write_size = n;

    if(copy_from_user(yy_proc_data_new->buf, buf, write_size) == 0)
    {
        printk("copy_from_user success.\n");
    }
    else
    {
        printk("copy_from_user failed.\n");
        /* kfree() */
        kfree(yy_proc_data_new);
        return -1;
    }

    yy_proc_data_new->next = NULL;
    tail->next = yy_proc_data_new;
    tail = yy_proc_data_new;
    return write_size;
}

static struct file_operations yy_file_ops = {
    .open = yy_open,
    .read = seq_read,
    .write = yy_write,
    .release = seq_release
};


/* yy_proc init */
static int yy_init(void)
{
    printk("init yy_proc[--kernel--]\n");	

    /* proc */
    struct proc_dir_entry* entry;
    entry = proc_create(PROC_NAME, S_IWUSR | S_IRUGO, NULL, &yy_file_ops);
    if (entry == NULL)
    {
        printk("proc_create failed.\n");
        return -ENOMEM;
    }
    printk("proc_create success.\n");
    
    /* init data */
    /* init a yy_proc_data */
    struct yy_proc_data* yy_proc_data_new = (struct yy_proc_data*)
        kmalloc(sizeof(struct yy_proc_data), GFP_KERNEL);
    if (yy_proc_data_new == NULL)
    {
        printk("kmalloc for yy_proc_data_new failed.\n");
        return -1;
    }

    head = yy_proc_data_new;
    tail = yy_proc_data_new;

	strcpy(yy_proc_data_new->buf, "first data initialed in init().\n");
    yy_proc_data_new->next = NULL;

    
    return 0;
}

/* yy_proc exit */
static void yy_exit(void)
{
    printk("cleanup yy_proc[--kernel--]\n");	

    /* remove proc */
    remove_proc_entry(PROC_NAME, NULL);

    /* kfree data */
    struct yy_proc_data* it = head;
    while (it != NULL)
    {
        struct yy_proc_data* it_tmp = it;
        it = it->next;
        kfree(it_tmp);
    }
    return;
}

/* module info */
MODULE_DESCRIPTION("Practice module param and proc filesystem interface.");
MODULE_LICENSE("GPL");

/* kernel entry */
module_init(yy_init);
module_exit(yy_exit);
