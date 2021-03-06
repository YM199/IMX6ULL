#ifndef __LED_H__
#define __LED_H__

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define DEBUG

#ifdef DEBUG 
    #define debug(...) printk(__VA_ARGS__)
#else 
    #define debug(...)
#endif

#define LED_CNT   1
#define LED_NAME  "led" /*设备名*/



#define LED_ON  1  /*开灯*/
#define LED_OFF 0  /*关灯*/


struct chrdev
{
    dev_t devid;            /*设备号*/
    struct cdev cdev;   
    struct class *class;
    struct device *device;
    int major;              /*主设备号*/
    int minor;              /*次设备号*/
    struct device_node *nd; /*设备节点*/
    int led_gpio;           /*led使用的GPIO编号*/
    atomic_t lock;          /*原子变量*/
};

#endif