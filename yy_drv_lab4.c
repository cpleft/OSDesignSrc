#include<linux/kernel.h>        /* open(), release() */
#include<linux/init.h>
#include<linux/fs.h>            /* ioctl(), read(), write() */
#include<linux/module.h>
#include<linux/slab.h>          /* kmalloc(), kfree() */
#include<linux/uaccess.h>	/* copy_to_user(), copy_from_user() */
#include<linux/mm.h>

#define  YY_MAJOR       98
#define  VERSION        "yy_drv_lab4"
#define  BUFFER_SIZE    1024

static unsigned long ADDRESS;

static void yy_vma_open(struct vm_area_struct* vma)
{
    printk("Simple VMA open, virt %lx, phys %lx\n", 
            vma->vm_start, vma->vm_pgoff << PAGE_SHIFT)
}

static void yy_vma_close(struct vm_area_struct* vma)
{
    printk("yy_vma_close\n");
}

static int yy_vma_fault(struct vm_area_struct* vma, 
        struct vm_fault* vmf)
{
    printk("yy_vma_fault\n");
    struct page* pp;
    unsigned long offset = ADDRESS;
    unsigned long virtaddr = vmf->virtual_address - vma->vm_start + offset;

    pp = virt_to_page(virtaddr);
    if (!pp)
    {
        return VM_FAULT_SIGBUS;
    }
    get_page(pp);
    vmf->page = pp;
    return 0;
}

static struct vm_operations_struct yy_vmops = {
    .open = yy_vma_open,
    .close = yy_vma_close,
    .fault = yy_vma_fault
};

static int yy_fault_mmap(struct file* fp, 
        struct vm_area_struct* vma)
{
    printk("yy_fault_mmap\n");
    unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
    if (offset >= __pa(high_memory) || (fp->f_flags & O_SYNC))  /*? */
    {
        vma->vm_flags |= VM_IO;
    }
    vma->vm_flags |= VM_RESERVED;
    vma->vm_ops = &yy_vmops;
    yy_vma_open(vma);
    return 0;
}

/* file ops */
static int yy_open(struct inode* ip, 
        struct file* fp)
{
    printk("yy_open\n");
}

static int yy_release(struct inode* ip, 
        struct file* fp)
{
    printk("yy_release\n");
}

static struct file_operations yy_fops = {
    .open = yy_open,
    .release = yy_release,
    .mmap = yy_fault_mmap
};

static int yy_init()
{
    int ret = -1;
    ret = register_chrdev(YY_MAJOR, VERSION, &yy_fops);
    showVersion();
    if (ret < 0) {
        printk("yy_drv register failed with %d[--kernel--]\n", ret);
        return ret;
    }
    ADDRESS = __get_free_pages(GFP_KERNEL, get_order(65535));
    if (ADDRESS == 0)
    {
        unregister_chrdev(YY_MAJOR, VERSION);
        printk("yy_drv register failed with ADDRESS = 0![--kernel--]\n");
        return -ENOMEM;
    }

    printk("yy_drv register success![--kernel--]\n");
    printk("\n...\nret=%x\nADDRESS=%lu\n...\n", ret, ADDRESS);
    return ret;
}

/* yy_drv exit */
static void yy_exit()
{
    free_pages(ADDRESS, get_order(65535));
    unregister_chrdev(YY_MAJOR, VERSION);
    printk("cleanup yy_drv[--kernel--]\n");	
}

/* module info */
MODULE_DESCRIPTION("tasklet and work queue.");
MODULE_LICENSE("GPL");
/* kernel entry */
module_init(yy_init);
module_exit(yy_exit);
