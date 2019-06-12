#include<linux/kernel.h>        /* open(), release() */
#include<linux/init.h>
#include<linux/fs.h>            /* ioctl(), read(), write() */
#include<linux/module.h>
#include<linux/slab.h>          /* kmalloc(), kfree() */
#include<linux/uaccess.h>   	/* copy_to_user(), copy_from_user() */
#include<linux/mutex.h>         /* struct mutex */
#include<linux/seq_file.h>      
#include<linux/proc_fs.h>     	/* proc_create(), remove_proc_entry() */


#define  YY_MAJOR       98
#define  VERSION        "yy_drv_lab2"
#define  BUFFER_SIZE    1024

/* module param */
int N = 512;
module_param(N, int, 0);
MODULE_PARM_DESC(N, "the size of the yy_contain buffer.");

/* functions declaration */
void showVersion(void);
ssize_t yy_seq_write(struct file* fp, const char* buf, 
		size_t count, loff_t* f_ops);
ssize_t yy_seq_read(struct file* fp, char* buf, 
        size_t count, loff_t* f_ops);
int yy_seq_open(struct inode* ip, 
        struct file* fp);
int yy_seq_release(struct inode* ip, 
        struct file* fp);


static void add_one(void);
static void* _seq_start(struct seq_file* seq_fp, loff_t* pos);
static void* _seq_next(struct seq_file* seq_fp, void* np, loff_t* pos);
static void _seq_stop(struct seq_file* seq_fp, void* np);
static int _seq_show(struct seq_file* seq_fp, void* np);


static struct mutex lock;
static struct list_head head;
struct yy_contain
{
    struct list_head list;
    int count;
    char buffer[BUFFER_SIZE];
    int size;
};

/* register */
struct file_operations yy_f_ops = {
    open   : yy_seq_open,
    read   : seq_read,
    write  : yy_seq_write,
    release: yy_seq_release,
};

/* seq_file opeations */
static struct seq_operations _seq_fops = {
    .start = _seq_start,
    .next = _seq_next,
    .stop = _seq_stop,
    .show = _seq_show
};


void showVersion(void)
{
    printk("****************************\n");
    printk("\t%s\t\n", VERSION);
    printk("****************************\n");
}

ssize_t yy_seq_read(struct file* fp, 
        char* buf, 
        size_t count, 
        loff_t* f_ops)
{
    printk("yy_seq_read[--kernel--]\n");
    
    return 0;
}

ssize_t yy_seq_write(struct file* fp, 
        const char* buf, 
        size_t count, 
        loff_t* f_ops)
{
    printk("yy_seq_write[--kernel--]\n");

    add_one();


    return 0;
}

int yy_seq_open(struct inode* ip, 
        struct file* fp)
{
    printk("yy_seq_open[--kernel--]\n");
    return seq_open(fp, &_seq_fops);
}

int yy_seq_release(struct inode* ip, 
        struct file* fp)
{
    printk("yy_seq_release[--kernel__]\n");
    
    /*? It seems data should be released in _seq_xxx functions, not here. */
    struct yy_contain* yy_cp;
    while (!list_empty(&head))
    {
        yy_cp = list_entry(head.next, struct yy_contain, list);
        list_del(&yy_cp->list);
        kfree(yy_cp);
    }

    return 0;
}



static void add_one(void)
{
    printk("add_one[--kernel__]\n");

    struct yy_contain* yy_cp = NULL;

    mutex_lock(&lock);
    yy_cp = (struct yy_contain*)kmalloc(sizeof(struct yy_contain), GFP_KERNEL);
    /* init */
    int i = 1;
    for (i = 0; i < BUFFER_SIZE; ++i)
    {
        (*yy_cp).buffer[i] = 0;
    }
    (*yy_cp).size = N;
    (*yy_cp).count = 1;

    if (yy_cp != NULL)
        list_add(&yy_cp->list, &head);
    mutex_unlock(&lock);
}

static void* _seq_start(struct seq_file* seq_fp, loff_t* pos)
{
    printk("_seq_start[--kernel__]\n");
    mutex_lock(&lock);
    return seq_list_start(&head, *pos);
}
static void* _seq_next(struct seq_file* seq_fp, void* np, loff_t* pos)
{
    printk("_seq_next[--kernel__]\n");
    return seq_list_next(np, &head, pos);
}
static void _seq_stop(struct seq_file* seq_fp, void* np)
{
    printk("_seq_stop[--kernel__]\n");
    mutex_unlock(&lock);
}
static int _seq_show(struct seq_file* seq_fp, void* np)
{
    printk("_seq_show[--kernel__]\n");
    struct yy_contain* yy_cp = list_entry(np, struct yy_contain, list);
    seq_printf(seq_fp, "value: %s\n", yy_cp->buffer);
	return 0;
}


/* yy_drv init */
static int yy_init(void)
{
    int ret = -1;
    ret = register_chrdev(YY_MAJOR, VERSION, &yy_f_ops);
    showVersion();
    if (ret < 0) {
        printk("yy_drv register failed with %d[--kernel--]\n", ret);
        return ret;
    }
    else {
        printk("yy_drv register success![--kernel--]\n");
    }
    printk("\n...\nret=%x\n...\n", ret);

    /* init seq_file interface */
    struct proc_dir_entry* entry;

    mutex_init(&lock);

    INIT_LIST_HEAD(&head);
    add_one(); add_one();

    entry = proc_create("yy_data", S_IWUSR | S_IRUGO, NULL, &_seq_fops);
    if (entry == NULL)
    {
		printk("failed to create proc file.\n");
        // free add_one()'s space.
        struct yy_contain* yy_cp;
        while (!list_empty(&head))
        {
            yy_cp = list_entry(head.next, struct yy_contain, list);
            list_del(&yy_cp->list);
            kfree(yy_cp);
        }
        return -ENOMEM;
    }
    return ret;
}

/* yy_drv exit */
static void yy_exit(void)
{
    printk("cleanup yy_drv[--kernel--]\n");	
    
    /* exit seq_file */
    struct yy_contain* yy_cp;
    remove_proc_entry("yy_data", NULL);
    while (!list_empty(&head))
    {
        yy_cp = list_entry(head.next, struct yy_contain, list);
        list_del(&yy_cp->list);
        kfree(yy_cp);
    }

    unregister_chrdev(YY_MAJOR, VERSION);
}

/* module info */
MODULE_DESCRIPTION("Practice module param and proc filesystem interface.");
MODULE_LICENSE("GPL");

/* kernel entry */
module_init(yy_init);
module_exit(yy_exit);
