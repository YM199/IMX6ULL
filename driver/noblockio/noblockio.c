#include "include/noblockio.h"

struct irq_dev irqdev;

static int irq_open(struct inode *inode, struct file *filp);
static ssize_t irq_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt);
unsigned int irq_poll( struct file *filp, struct poll_table_struct *wait );

static struct file_operations irq_fops = {
    .owner = THIS_MODULE,
    .open = irq_open,
    .read = irq_read,
    .poll = irq_poll,
};

static int irq_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &irqdev;	/* 设置私有数据 */
	return 0;
}

static ssize_t irq_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
    int ret = 0;
    int data = 123;
    struct irq_dev *dev = ( struct irq_dev * )filp->private_data;

    unsigned char releasekey = atomic_read(&dev->releasekey);


    if( releasekey )
    {
        ret = copy_to_user( buf, &data, sizeof( data ));
        atomic_set( &dev->releasekey, 0 );
    }
    else
    {
        return -1;/*没有被按下直接返回错误码*/
        //return -EINVAL;
    }

    return 0;
}

unsigned int irq_poll( struct file *filp, struct poll_table_struct *wait )
{
    unsigned int mask = 0;
    struct irq_dev *dev = ( struct irq_dev * )filp->private_data;

    if( atomic_read( &dev->releasekey ) ) /*按键按下并且松开*/
    {
        mask =  POLLIN | POLLRDNORM; /*有数据可以读取*/
    }

    return mask;
}

/*================================================================
 * 函数名：key0_handler
 * 功能描述：按键中断处理函数
 * 参数：
 *      irqreturn_t
 * 返回值：
 *      成功: 0
 *      失败: -1
 * 作者：Yang Mou 2022/1/19
================================================================*/
static irqreturn_t key0_handler(int irq, void *dev_id)
{
    struct irq_dev *dev = ( struct irq_dev * )dev_id;

    dev->timer.data = (volatile long)dev_id; /*将结构体传递给timer定时器*/
    mod_timer( &dev->timer, jiffies + msecs_to_jiffies( 10 ) ); /*定时10ms，定时器开始运行*/
    return IRQ_RETVAL(IRQ_HANDLED);
}


void timer_function( unsigned long arg )
{
    /*按键按下10ms后进入，相当于是个消抖*/
    struct irq_dev *dev = ( struct irq_dev * )arg;
    struct irq_keydesc *keydesc;
    unsigned char value;
    keydesc = &dev->irqkeydesc; /*获取irq_keydesc结构体*/

    value = gpio_get_value( keydesc->gpio ); /*读取按键*/
    if( value == 0 ) /*按键还没松开*/
    {
    }
    else /*按键已经松开*/
    {
        atomic_set( &dev->releasekey, 1 ); /*表示按键按下并且已经松开*/
    }



    return;
}

/*================================================================ 
 * 函数名：keyio_init
 * 功能描述：初始化GPIO
 * 参数：
 *      void
 * 返回值：
 *      成功: 0
 *      失败: -1
 * 作者：Yang Mou 2022/1/19
================================================================*/
static int keyio_init(void)
{
    int ret = 0;

    irqdev.nd = of_find_node_by_path( "/key" ); /* 获取/dev/key */
    debug( NULL == irqdev.nd );

    irqdev.irqkeydesc.gpio = of_get_named_gpio( irqdev.nd, "key-gpio", 0 );
    debug( irqdev.irqkeydesc.gpio < 0 );
    memset( irqdev.irqkeydesc.name, 0, sizeof( irqdev.irqkeydesc.name ) );/*将数组清0*/
    sprintf( irqdev.irqkeydesc.name, "KEY%d", 0 );/*中断名字*/
    gpio_request( irqdev.irqkeydesc.gpio, irqdev.irqkeydesc.name ); /*申请GPIO*/
    gpio_direction_input( irqdev.irqkeydesc.gpio ); /*设置GPIO为输入*/
    irqdev.irqkeydesc.irqnum = irq_of_parse_and_map( irqdev.nd, 0); /*获取中断号*/

    /*中断*/
    irqdev.irqkeydesc.handler = key0_handler; /*设置中断回调函数*/

    /*申请中断*/
    ret = request_irq( irqdev.irqkeydesc.irqnum,
                       irqdev.irqkeydesc.handler,
                       IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, /*上升沿和下降沿*/
                       irqdev.irqkeydesc.name, &irqdev );/*irqdev 会传递给中断处理函数 irq_handler_t 的第二个参数*/

    debug( ret < 0 );

    init_timer( &irqdev.timer ); /*初始化定时器*/
    irqdev.timer.function = timer_function; /*设置定时器的回调函数，当前定时器并未开始运行*/



    return 0;
}

/*================================================================
 * 函数名：Irq_init
 * 功能描述：加载驱动模块时会调用该函数
 * 参数：
 *      void
 * 返回值：
 *      成功: 0
 *      失败:
 * 作者：Yang Mou 2022/1/21
================================================================*/
static int __init Irq_init( void )
{
    /*申请设备号*/
    if( irqdev.major )
    {
        /*指定主设备号*/
        irqdev.devid = MKDEV( irqdev.major, 0 );
        register_chrdev_region( irqdev.devid, IRQ_CNT, IRQ_NAME );
    }
    else
    {
        /*动态分配主设备号*/
        alloc_chrdev_region( &irqdev.devid, 0, IRQ_CNT, IRQ_NAME );
        irqdev.major = MAJOR( irqdev.devid );
        irqdev.minor = MINOR( irqdev.devid );
    }

    /*创建字符设备*/
    irqdev.cdev.owner = THIS_MODULE;
    cdev_init( &irqdev.cdev, &irq_fops );
    cdev_add( &irqdev.cdev, irqdev.devid, IRQ_CNT );

    /*自动创建设备节点*/
    irqdev.class =  class_create( THIS_MODULE, IRQ_NAME );
    debug( IS_ERR( irqdev.class ) );
    irqdev.device = device_create( irqdev.class, NULL, irqdev.devid, NULL, IRQ_NAME );
    debug( IS_ERR( irqdev.device ) );

    atomic_set( &irqdev.releasekey, 0 );
    keyio_init();

    return 0;
}

/*================================================================
 * 函数名：Irq_exit
 * 功能描述：卸载驱动模块时会调用该函数
 * 参数：
 *      void
 * 返回值：
 *      成功: 0
 *      失败:
 * 作者：Yang Mou 2022/1/21
================================================================*/
static void __exit Irq_exit( void )
{
    free_irq( irqdev.irqkeydesc.irqnum, &irqdev ); /*释放中断*/
    gpio_free( irqdev.irqkeydesc.gpio ); /*释放GPIO*/

    cdev_del( &irqdev.cdev ); /*删除字符设备*/
    unregister_chrdev_region( irqdev.devid, IRQ_CNT ); /*注销设备号*/

    /*删除自动创建的设备节点*/
    device_destroy( irqdev.class, irqdev.devid );
    class_destroy( irqdev.class );

    return;
}

module_init(Irq_init);
module_exit(Irq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YangMou");

