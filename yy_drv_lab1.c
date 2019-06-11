#include<linux/kernel.h>                    /* open(), release() */
#include<linux/init.h>
#include<linux/fs.h>                        /* ioctl(), read(), write() */
#include<linux/module.h>
#include<linux/slab.h>                      /* kmalloc(), kfree() */
#include<linux/uaccess.h>					/* copy_to_user(), copy_from_user() */

#define  YY_MAJOR       98
#define  VERSION        "yy_drv_lab1"
#define  BUFFER_SIZE    1024

struct yy_contain
{
    int count;
    char buffer[BUFFER_SIZE];
    int size;
};
struct yy_contain* yy_cp = NULL;

void showVersion(void)
{
    printk("****************************\n");
    printk("\t%s\t\n", VERSION);
    printk("****************************\n");
}

ssize_t yy_read(struct file* fp, 
        char* buf, 
        size_t count, 
        loff_t* f_ops)
{
    printk("yy_read[--kernel--]\n");

    int read_size = count;
    if ((*yy_cp).size <  count)
    {
        read_size = (*yy_cp).size;
    }

    if(!copy_to_user(buf, (*yy_cp).buffer, read_size))
    {
        printk("copy_to_user success.\n");
    }
    else
    {
        printk("copy_to_user failed.\n");
        return -1;
    }
    return read_size;
}

ssize_t yy_write(struct file* fp, 
        const char* buf, 
        size_t count, 
        loff_t* f_ops)
{
    printk("yy_write[--kernel--]\n");

    int write_size = count;
    if (write_size > BUFFER_SIZE)
    {
        write_size = BUFFER_SIZE;
    }

    if(!copy_from_user((*yy_cp).buffer, buf, write_size))
    {
        printk("copy_from_user success.\n");
    }
    else
    {
        printk("copy_from_user failed.\n");
        return -1;
    }
    return write_size;
}

int yy_ioctl(struct inode* ip, 
        struct file* fp, 
        unsigned int cmd, 
        unsigned long data)
{
    printk("yy_ioctl[--kernel--]\n");       
    return 0;
}

int yy_open(struct inode* ip, 
        struct file* fp)
{
    printk("yy_open[--kernel--]\n");

    if (yy_cp == NULL) 
    {
        yy_cp = (struct yy_contain*)kmalloc(sizeof(struct yy_contain), GFP_KERNEL);
        
        /* init */
        int i = 1;
        for (i = 0; i < BUFFER_SIZE; ++i)
        {
            (*yy_cp).buffer[i] = 0;
        }

        (*yy_cp).size = 0;
        (*yy_cp).count = 1;
    }
    else
    {
        (*yy_cp).count++;
    }

    if (yy_cp == NULL)
    {
        printk("kmalloc for buffer failed\n");
    }

    printk("buffer has initialed, count is %d\n", (*yy_cp).count);

    return 0;
}

int yy_release(struct inode* ip, 
        struct file* fp)
{
    printk("yy_release[--kernel__]\n");

    if ((*yy_cp).count > 1)
    {
        (*yy_cp).count--;
        printk("release a count, now count is %d\n", (*yy_cp).count);
    }
    else if ((*yy_cp).count == 1)
    {
        /* release memory */
        kfree(yy_cp);
        printk("count was 0; released the memory.\n");
    }
    else 
        return -1;

    return 0;
}


/* register */
struct file_operations yy_f_ops = {
    open   : yy_open,
    read   : yy_read,
    write  : yy_write,
    /* ioctl  : yy_ioctl, */
    release: yy_release,
};

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
    return ret;
}

/* yy_drv exit */
static void yy_exit(void)
{
    printk("cleanup yy_drv[--kernel--]\n");	
    unregister_chrdev(YY_MAJOR, VERSION);
}

/* module info */
MODULE_DESCRIPTION("Simple int driver module");
MODULE_LICENSE("GPL");

/* kernel entry */
module_init(yy_init);
module_exit(yy_exit);
