
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

#define DEV_DEBUG

#ifdef DEV_DEBUG
#define debug(fmt, ...)  printk(KERN_DEBUG"[%s] "fmt"\n",__func__,##__VA_ARGS__)
#else
#define debug(fmt,...)  {}
#endif

#define DEV_NAME        "dht11"
#define DEV_GPIO_LABLE  "dht11_data"
#define DATA_BITS       40
#define DATA_BUF_SIZE   5       /* 40bit / sizeof(u8) = 5byte */
#define RETRY           2


typedef struct {
    u8 data[DATA_BUF_SIZE];
    struct mutex locker;
} dht11_t;

static unsigned dev_gpio_data = 20;

module_param(dev_gpio_data, uint, S_IRUGO);

static struct {
    u8 gpio_data;
    int interrupted;
    struct timer_list timer;
    struct mutex dev_locker;
} dev;

static void watch_dog(unsigned long data) {
    int *interrupted = (int *) data;
    debug();
    *interrupted = true;
}

static int dht11_trigger(dht11_t *data) {
    int ret = 0;
    u8 bit = 0;
    u8 value;
    u8 *buffer = data->data;
    debug();
    mutex_lock(&dev.dev_locker);
    dev.interrupted = false;
    dev.timer.data = (unsigned long) &dev.interrupted;
    dev.timer.expires = jiffies + msecs_to_jiffies(1500);
    add_timer(&dev.timer);
    gpio_direction_output(dev.gpio_data, 1);
    msleep(1000);
    gpio_set_value(dev.gpio_data, 0);
    mdelay(18);
    //gpio_set_value(dev.gpio_data, 1);
    gpio_direction_input(dev.gpio_data);
    udelay(20);
    while (gpio_get_value(dev.gpio_data) && !dev.interrupted);
    if (dev.interrupted) {
        ret = -ENODEV;
        goto done;
    }
    /* prepare receive data */
    mod_timer(&dev.timer, jiffies + msecs_to_jiffies(10));
    udelay(100);
    while (gpio_get_value(dev.gpio_data) && !dev.interrupted);
    while (bit < DATA_BITS && !dev.interrupted) {
        while (!gpio_get_value(dev.gpio_data) && !dev.interrupted);
        udelay(40);
        value = gpio_get_value(dev.gpio_data);
        buffer[bit / 8] |= value << (7 - bit % 8);
        while (gpio_get_value(dev.gpio_data) && !dev.interrupted);
        bit++;
    }
    if (dev.interrupted) {
        ret = -EBUSY;
        goto done;
    }
    //debug("[ %2X , %2X , %2X , %2X , %2X ]", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
    if ((0xFF & (buffer[0] + buffer[1] + buffer[2] + buffer[3])) != buffer[4])
        ret = -ENODATA;

    done:
    if (timer_pending(&dev.timer))
        del_timer(&dev.timer);
    mutex_unlock(&dev.dev_locker);
    return ret;
}

static int dht11_open(struct inode *inode, struct file *filp) {
    dht11_t *data = kzalloc(sizeof(dht11_t), GFP_KERNEL);
    debug();
    if (!data)
        return -ENOMEM;
    mutex_init(&data->locker);
    filp->private_data = data;
    return 0;
}

static int dht11_release(struct inode *inode, struct file *filp) {
    dht11_t *data = filp->private_data;
    debug();
    mutex_destroy(&data->locker);
    kfree(data);
    filp->private_data = NULL;
    return 0;
}

static long dht11_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    int ret = 0;
    dht11_t *data = filp->private_data;
    debug();
    // Check type and command number
    if (_IOC_NR(cmd) > IOC_MAXNR) {
        debug("cmd numer [%d] exceeded", _IOC_NR(cmd));
        return -ENOTTY;
    }
    if (_IOC_TYPE(cmd) != IOC_MAGIC) {
        debug("cmd type [%c] error", _IOC_TYPE(cmd));
        return -ENOTTY;
    }
    switch (cmd) {
        case DHT_IOC_TRIG:
            dht11_trigger(data);
            break;
        case DHT_IOC_READ:
            mutex_lock(&data->locker);
            ret = copy_to_user(data->data, (u8 __user *) arg, DATA_BUF_SIZE);
            mutex_unlock(&data->locker);
            break;
        default:
            break;
    }
    return ret;
}

static ssize_t dht11_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    int ret = 0;
    dht11_t *data = filp->private_data;
    u8 buffer[] = {
            '0' + data->data[0],
            '.',
            '0' + data->data[1],
            ';',
            '0' + data->data[2],
            '.',
            '0' + data->data[3],
            '\n',
            '\0',
    };
    size_t size = sizeof(buffer);
    debug();
    if (*f_pos >= size)
        return 0;
    if (count > size - *f_pos)
        count = size - *f_pos;
    mutex_lock(&data->locker);
    ret = copy_to_user(buf, buffer, count);
    if (ret == 0) {
        *f_pos += count;
        ret = count;
    }
    mutex_unlock(&data->locker);
    return ret;
}

static ssize_t dht11_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    int ret = 0;
    dht11_t *data = filp->private_data;
    u8 *buffer = data->data;
    mutex_lock(&data->locker);
    ret = copy_from_user(buffer, buf, DATA_BUF_SIZE);
    mutex_unlock(&data->locker);
    if (ret != 0)
        return ret;
    debug("write[%d] , %d", buffer[0] - '0', count);
    memset(buffer, 0, DATA_BUF_SIZE);
    ret = dht11_trigger(data);
    if (ret == 0) {
        debug("read TEM[ %u.%u C], HUM[ %u.%u %%]", buffer[2], buffer[3], buffer[0], buffer[1]);
        ret = count;
    }
    return ret;
}

static struct file_operations dev_fops = {
        .owner          = THIS_MODULE,
        .open           = dht11_open,
        .release        = dht11_release,
        .read           = dht11_read,
        .write          = dht11_write,
        .unlocked_ioctl = dht11_ioctl,
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
    dev.gpio_data = dev_gpio_data;
    ret = gpio_request(dev.gpio_data, DEV_GPIO_LABLE);
    if (ret != 0)
        pr_err("request gpio(%u) failed \n", dev.gpio_data);
    init_timer(&dev.timer);
    dev.timer.function = watch_dog;
    mutex_init(&dev.dev_locker);
    return ret;
}

static void __exit dht11_exit(void) {
    debug();
    if (timer_pending(&dev.timer))
        del_timer(&dev.timer);
    misc_deregister(&dht11_dev);
    gpio_free(dev.gpio_data);
    mutex_destroy(&dev.dev_locker);
}

module_init(dht11_init);
module_exit(dht11_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("mumumusuc@gmail.com");
