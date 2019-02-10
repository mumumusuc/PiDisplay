/*
 * HC-SR04
 * DC       -   5V
 * I        -   15mA
 * FRQ      -   40Hz
 * MAX_DIST -   4m  (4/340 = 11.76ms = 11765us)
 * MIN_DIST -   2cm (0.02/340 = 0.059ms = 59us)
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <asm/delay.h>
#include <linux/uaccess.h>
#include <linux/completion.h>
#include "hc_sr04.h"

#define DEV_NAME    "hc-sr04"
#define RW_SIZE     sizeof(u32)

#define SOUND_SPD               340
#define ECHO_TIME_OUT           12 //ms
#define GPIO_LO                 0
#define GPIO_HI                 1
#define PIN_TRIG                21
#define PIN_ECHO                20
#define ECHO_IRQ_FLAG              (IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_ONESHOT)
#define CALC_TIME_US(begin, end)    ({1000000 * ((end).tv_sec - (begin).tv_sec) + ((end).tv_usec - (begin).tv_usec);})

typedef struct {
    u8 echo;
    u8 trig;
    u32 hold;
    u32 interval;
    struct completion done;
    struct mutex mutex;
    struct timeval time;
} hc_sr_t;

static hc_sr_t *hc_sr;

static irqreturn_t irq_handler_echo(int irq, void *dev) {
    u8 lev = gpio_get_value(hc_sr->echo);
    printk(KERN_INFO "[%s] echo = %d \n", __func__, lev);
    if (lev == 0) {
        // stop timer and calc distance
        struct timeval end;
        do_gettimeofday(&end);
        hc_sr->hold = CALC_TIME_US(hc_sr->time, end) / 2;
        hc_sr->interval = hc_sr->hold * SOUND_SPD / 1000;
        complete(&hc_sr->done);
        printk(KERN_INFO "[%s] time = %d us, interval = %d mm\n", __func__, hc_sr->hold, hc_sr->interval);
    } else {
        // start timer
        do_gettimeofday(&hc_sr->time);
    }
    return IRQ_HANDLED;
}

static void hc_sr_trig(hc_sr_t *dev) {
    while (gpio_get_value(dev->echo));
    printk(KERN_INFO "[%s] \n", __func__);
    init_completion(&dev->done);
    gpio_set_value(dev->trig, GPIO_HI);
    udelay(20);
    gpio_set_value(dev->trig, GPIO_LO);
    if (0 == wait_for_completion_timeout(&dev->done, ECHO_TIME_OUT))
        printk(KERN_NOTICE "[%s] echo time out\n", __func__);
}

static int init_gpio(hc_sr_t *dev) {
    int ret = 0;
    const u8 trig = dev->trig;
    const u8 echo = dev->echo;
    ret = gpio_request_one(trig, GPIOF_OUT_INIT_LOW, "HC_SR_TRIG");
    if (ret != 0)
        return ret;
    ret = gpio_request_one(echo, GPIOF_IN, "HC_SR_ECHO");
    if (ret != 0)
        return ret;
    enable_irq(gpio_to_irq(echo));
    ret = request_irq(gpio_to_irq(echo), irq_handler_echo, ECHO_IRQ_FLAG, "hc_sr echo", NULL);
    if (ret < 0) {
        printk(KERN_ALERT"[%s] irq_request rising failed \n", __func__);
        goto clear_gpio;
    }
    return 0;
    clear_gpio:
    gpio_free(trig);
    gpio_free(echo);
    return ret;
};

static int dev_open(struct inode *inode, struct file *filp) {
    printk(KERN_INFO "[%s] \n", __func__);
    filp->private_data = hc_sr;
    return 0;
}

static int dev_release(struct inode *inode, struct file *filp) {
    printk(KERN_INFO "[%s] \n", __func__);
    filp->private_data = NULL;
    return 0;
}

static long dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    int ret = 0;
    hc_sr_t *dev = (hc_sr_t *) filp->private_data;
    printk(KERN_INFO "[%s] \n", __func__);
    // Check type and command number
    if (_IOC_NR(cmd) > IOC_MAXNR) {
        printk(KERN_ALERT "[%s] cmd numer [%d] exceeded \n", __func__, _IOC_NR(cmd));
        return -ENOTTY;
    }
    if (_IOC_TYPE(cmd) != IOC_MAGIC) {
        printk(KERN_ALERT "[%s] cmd type [%c] error \n", __func__, _IOC_TYPE(cmd));
        return -ENOTTY;
    }
    mutex_lock(&dev->mutex);

    switch (cmd) {
        case HC_SR_IOC_TRIG:
            hc_sr_trig(dev);
            break;
        case HC_SR_IOC_READ:
            ret = put_user(dev->hold, (u32 __user *) arg);
            break;
        case HC_SR_IOC_TRIG_READ:
            hc_sr_trig(dev);
            ret = put_user(dev->hold, (u32 __user *) arg);
            break;
        default:
            break;
    }
    mutex_unlock(&dev->mutex);
    return ret;
}

static ssize_t dev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    int ret;
    hc_sr_t *dev = (hc_sr_t *) filp->private_data;
    printk(KERN_INFO "[%s] \n", __func__);
    if (count < 1)
        return 0;
    if (*f_pos >= RW_SIZE)
        return 0;
    if (count < RW_SIZE)
        return 0;
    if (count > RW_SIZE - *f_pos)
        count = RW_SIZE - *f_pos;
    mutex_lock(&dev->mutex);
    hc_sr_trig(dev);
    ret = put_user(dev->interval, (u32 __user *) buf);
    if (ret == 0) {
        *f_pos += count;
        ret = count;
        printk(KERN_INFO "[%s] interval = %d mm\n", __func__, dev->interval);
    }
    mutex_unlock(&dev->mutex);
    return ret;
}

static ssize_t dev_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    int ret = 0;
    u8 tirg = -1;
    hc_sr_t *dev = (hc_sr_t *) filp->private_data;
    printk(KERN_INFO "[%s] \n", __func__);
    if (count < 1)
        return 0;
    mutex_lock(&dev->mutex);
    ret = get_user(tirg, (u32 __user *) buf);
    if (ret == 0)
        ret = count;
    if (tirg == 0) {
        hc_sr_trig(dev);
    }
    mutex_unlock(&dev->mutex);
    return ret;
}

static struct file_operations dev_fops = {
        .owner = THIS_MODULE,
        .open = dev_open,
        .release = dev_release,
        .unlocked_ioctl = dev_ioctl,
        .read = dev_read,
        .write = dev_write,
};

static struct miscdevice hc_sr_dev = {
        .minor = MISC_DYNAMIC_MINOR,
        .name=DEV_NAME,
        .fops = &dev_fops,
        .mode = 0666,
};

static int __init hc_sr_init(void) {
    int ret = 0;
    printk(KERN_INFO "[%s] \n", __func__);

    ret = misc_register(&hc_sr_dev);
    if (ret != 0) {
        printk(KERN_ALERT"[%s] create misc device failed \n", __func__);
        return ret;
    }

    hc_sr = (hc_sr_t *) kzalloc(sizeof(hc_sr_t), GFP_KERNEL);
    if (IS_ERR(hc_sr)) {
        printk(KERN_ALERT"[%s] alloc mem failed \n", __func__);
        return -ENOMEM;
    }
    hc_sr->trig = PIN_TRIG;
    hc_sr->echo = PIN_ECHO;
    mutex_init(&hc_sr->mutex);
    ret = init_gpio(hc_sr);
    if (ret != 0)
        return ret;
    return 0;
}

static void __exit hc_sr_exit(void) {
    printk(KERN_INFO "[%s] \n", __func__);
    misc_deregister(&hc_sr_dev);
    free_irq(gpio_to_irq(PIN_ECHO), NULL);
    mutex_destroy(&hc_sr->mutex);
    gpio_free(hc_sr->echo);
    gpio_free(hc_sr->trig);
    kfree(hc_sr);
}

module_init(hc_sr_init);
module_exit(hc_sr_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("mumumusuc");