#include<linux/kernel.h>        /* open(), release() */
#include<linux/init.h>
#include<linux/fs.h>            /* ioctl(), read(), write() */
#include<linux/module.h>
#include<linux/slab.h>          /* kmalloc(), kfree() */
#include<linux/uaccess.h>	/* copy_to_user(), copy_from_user() */
#include<linux/interrupt.h>     /* tasklet_struct */
#include<linux/timer.h>
#include<linux/time.h>
#include<linux/workqueue.h>
#include<asm/atomic.h>

#define  YY_MAJOR       98
#define  VERSION        "yy_drv_lab3"
#define  BUFFER_SIZE    1024

static struct tasklet_struct yy_tasklet;

static void tasklet_handler(unsigned long data)
{
    printk(KERN_ALERT "tasklet_handler is running.\n");
}

static struct work_struct yy_work;
static void work_handler(struct work_struct* data)
{
    printk(KERN_ALERT "work_handler is running.\n");
}

static struct workqueue_struct* yy_workqueue;


struct timer_data {
    struct time_list timer;
    unsigned long prev_jiffies;
    unsigned int loops;
};
struct timer_data yy_data1;
struct timer_data yy_data2;

atomic_t wq_run_times;
unsigned int failed_cnt = 0;
static int work_delay1 = 2 * HZ;
static int work_delay2 = 3 * HZ;

void test_timer_fn1(unsigned long arg)
{
    struct timer_data* data = (struct timer_data*) arg;
    unsigned long j = jiffies;

    data->timer.expires += work_delay1;
    data->prev_jiffies = j;
    add_timer(&data->timer);

    if (queue_work(yy_workqueue, &yy_work) == 0)
    {
        printk("Timer (0) add work queue failed\n");
        failed_cnt++;
    }
    data->loops++;
    printk("timer-0 loops: %u\n", data->loops);
}

void test_timer_fn2(unsigned long arg)
{
    struct timer_data* data = (struct timer_data*) arg;
    unsigned long j = jiffies;

    data->timer.expires += work_delay2;
    data->prev_jiffies = j;
    add_timer(&data->timer);

    tasklet_schedule(&yy_tasklet);

    data->loops++;
    printk("timer-1 loops: %u\n", data->loops);
}


static int yy_init(void)
{
    /* timer */
    unsigned long j = jiffies;
    printk("jiffies: %lu\n", jiffies);

    atomic_set(&wq_run_times, 0);

    /* yy_data1: workqueue*/
    init_timer(&yy_data1.timer);
    yy_data1.loops = 0;
    yy_data1.prev_jiffies = j;
    yy_data1.timer.function = test_timer_fn1;
    yy_data1.timer.data = (unsigned long)(&yy_data1);
    yy_data1.timer.expires = j + work_delay1;
    add_timer(&yy_data1.timer);

    /* yy_data2: tasklet*/
    init_timer(&yy_data2.timer);
    yy_data2.loops = 0;
    yy_data2.prev_jiffies = j;
    yy_data2.timer.function = test_timer_fn2;
    yy_data2.timer.data = (unsigned long)(&yy_data2);
    yy_data2.timer.expires = j + work_delay2;
    add_timer(&yy_data2.timer);

    /* work */
    INIT_WORK(&yy_work, work_handler);
    printk(KERN_ALERT "yy_work init successfully.\n");

    /* workqueue */
    yy_workqueue = create_singlethread_workqueue("yy-workqueue");
    printk(KERN_ALERT "yy_workqueue init successfully.\n");

    /* tasklet */
    tasklet_init(&yy_tasklet, tasklet_handler, 0);
    printk(KERN_ALERT "yy_tasklet init successfully.\n");

    printk(KERN_ALERT "yy_init sucessfully.\n");
    return 0;
}

static void yy_exit()
{
    tasklet_kill(&yy_tasklet);
    printk(KERN_ALERT "yy_exit.\n");	
}
/* module info */
MODULE_DESCRIPTION("tasklet and work queue.");
MODULE_LICENSE("GPL");
/* kernel entry */
module_init(yy_init);
module_exit(yy_exit);
