
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include "dht11.h"

//#define DEV_DEBUG

#ifdef DEV_DEBUG
#define debug(fmt, ...)  printk(KERN_DEBUG"[%s] "fmt"\n",__func__,##__VA_ARGS__)
#else
#define debug(fmt, ...)  {}
#endif

#define DEV_NAME        "dht11"
#define DEV_GPIO_LABLE  "dht11_data"
#define DEV_TIME_OUT    1000
#define DATA_BITS       40
#define DATA_BUF_SIZE   5       /* 40bit / sizeof(u8) = 5byte */
#define DATA_BUF_READ   4

#define _wait(flag, timeout)      ({dev_timer = (timeout); while ((flag) && (dev_timer--)) ndelay(500);dev_timer==0;})
#define wait(flag)                _wait((flag),DEV_TIME_OUT)


typedef struct {
    u8 data[DATA_BUF_SIZE];
    struct mutex data_locker;
} dht11_data_t;

static unsigned dev_gpio_data = 20;
static unsigned dev_timer = 0;
DEFINE_MUTEX(dev_locker);

module_param(dev_gpio_data, uint, S_IRUGO);


static int dht11_trigger(dht11_data_t *data) {
    int ret = 0;
    int timeout = false;
    u8 bit = 0;
    u8 value;
    u8 *buffer = data->data;
    mutex_lock(&dev_locker);
    debug();
    gpio_direction_output(dev_gpio_data, 1);
    msleep(500);
    gpio_set_value(dev_gpio_data, 0);
    usleep_range(18 * 1000, 20 * 1000);
    //gpio_set_value(dev.gpio_data, 1);
    gpio_direction_input(dev_gpio_data);
    udelay(20);
    timeout = wait(gpio_get_value(dev_gpio_data));
    if (timeout) {
        ret = -ENODEV;
        goto done;
    }
    /* prepare receive data */
    //mod_timer(&dev.timer, jiffies + msecs_to_jiffies(15));
    udelay(100);
    timeout = wait(gpio_get_value(dev_gpio_data));
    while (bit < DATA_BITS) {
        timeout = wait(!gpio_get_value(dev_gpio_data));
        udelay(40);
        value = gpio_get_value(dev_gpio_data);
        buffer[bit / 8] |= value << (7 - bit % 8);
        timeout = wait (gpio_get_value(dev_gpio_data));
        bit++;
    }
    if (timeout) {
        ret = -EBUSY;
        goto done;
    }
    //debug("[ %2X , %2X , %2X , %2X , %2X ]", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
    if ((0xFF & (buffer[0] + buffer[1] + buffer[2] + buffer[3])) != buffer[4])
        ret = -ENODATA;

    done:
    mutex_unlock(&dev_locker);
    return ret;
}

static int dht11_open(struct inode *inode, struct file *filp) {
    dht11_data_t *data = kzalloc(sizeof(dht11_data_t), GFP_KERNEL);
    debug();
    if (!data)
        return -ENOMEM;
    mutex_init(&data->data_locker);
    filp->private_data = data;
    return 0;
}

static int dht11_release(struct inode *inode, struct file *filp) {
    dht11_data_t *data = filp->private_data;
    debug();
    mutex_destroy(&data->data_locker);
    kfree(data);
    filp->private_data = NULL;
    return 0;
}

static ssize_t dht11_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    int ret = 0;
    dht11_data_t *data = filp->private_data;
    u8 *buffer = data->data;
    size_t size = DATA_BUF_READ;
    debug();
    if (*f_pos >= size) {
        *f_pos = 0;
        return 0;
    }
    if (count > size - *f_pos)
        count = size - *f_pos;

    mutex_lock(&data->data_locker);
    memset(buffer, 0, DATA_BUF_SIZE);
    ret = dht11_trigger(data);
    if (ret != 0)
        goto done;
    debug("TEM[ %u.%u C], HUM[ %u.%u %%]", buffer[2], buffer[3], buffer[0], buffer[1]);
    ret = copy_to_user(buf, buffer, count);
    if (ret == 0) {
        *f_pos += count;
        ret = count;
    }

    done:
    mutex_unlock(&data->data_locker);
    debug("ret = %d", ret);
    return ret;
}

static struct file_operations dev_fops = {
        .owner          = THIS_MODULE,
        .open           = dht11_open,
        .release        = dht11_release,
        .read           = dht11_read,
};

static struct miscdevice dht11_dev = {
        .minor  = MISC_DYNAMIC_MINOR,
        .name   = DEV_NAME,
        .mode   = S_IRUGO | S_IWUGO, /* 0666 */
        .fops   = &dev_fops,
};

static int __init dht11_init(void) {
    int ret = 0;
    debug();
    ret = misc_register(&dht11_dev);
    if (ret != 0) {
        pr_err("create misc device failed \n");
        return ret;
    }
    dev_gpio_data = dev_gpio_data;
    ret = gpio_request(dev_gpio_data, DEV_GPIO_LABLE);
    if (ret != 0)
        pr_err("request gpio(%u) failed \n", dev_gpio_data);
    return ret;
}

static void __exit dht11_exit(void) {
    debug();
    misc_deregister(&dht11_dev);
    gpio_free(dev_gpio_data);
    mutex_destroy(&dev_locker);
}

module_init(dht11_init);
module_exit(dht11_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("mumumusuc@gmail.com");
