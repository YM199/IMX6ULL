#include "include/led_driver.h"

struct xxx_dev xxxdev; /*定义设备结构体变量*/

/*
* 函数名称: open
* 函数功能:
* 函数备注:应用层调用open时自动执行该函数
*/
static int open( struct inode *inode, struct file *filp );
{
    return 0;
}

/*
* 函数名称: write
* 函数功能:
* 函数备注:应用层调用write时自动执行该函数
*/
static ssize_t write( struct file *filp, const char __user *buf, size_t cnt, loff_t *offt )
{
    return 0;
}

/*
* 结构名称：fops
* 结构类型：file_operations
* 结构备注：每一个设备号对应一个file_operations类型的结构，存放该驱动程序的各种操作函数
*/
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = open,
    .write = write,
};

/*
* 函数名称: probe
* 函数功能:
* 函数备注: 驱动和设备匹配成功以后此函数就会执行
*/
static int probe( struct platform_device *dev )
{
    cdev_init( &xxxdev.cdev, &fops ); /*注册字符设备驱动*/

    return 0;
}

static int remove( struct platform_device *dev )
{
    cdev_del( &xxxdev.cdev ); /*删除 dev*/

    return 0;
}

/*
* 结构名称: of_match
* 结构功能: 驱动和设备匹配列表
*/
static const struct of_device_id of_match[] = {
    { .compatible = "xxx-gpio" },
    {},
};

/*
* 结构名称: driver
* 结构功能: platform 平台驱动结构体
*/
static struct platform_driver driver = {
    .driver  = {
        .name = "xxx",
        .of_match_table = of_match,
    },

    .probe = probe,
    .remove = remove,
};

/*
 * 函数名：driver_init
 * 功能描述：完成驱动的注册和调用其他注册函数
 * 备忘：加载驱动时自动调用该函数
*/
static int __init driver_init( void ) 
{
    return platform_driver_register( &driver ); /*数向 Linux 内核注册一个 platform 驱动*/
}

/*================================================================
 * 函数名：driver_exit
 * 功能描述：释放驱动相关资源
 * 备忘：驱动卸载时自动执行该函数
================================================================*/
static void __exit driver_exit( void )
{
    platform_driver_unregister( &driver ); /*卸载 platform 驱动*/
}

module_init( driver_init );
module_exit( driver_exit );

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "YangMou" );