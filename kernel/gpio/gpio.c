//
// Created by mumumusuc on 19-1-31.
//

#include "gpio.h"
#include "bcm2835_gpio.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#define GPIO_DRV_NAME       "gpiomem"
#define GPIO_CLS_NAME       "gpiomem"
#define GPIO_DEV_NAME       "gpio"
#define GPIO_MAJOR      232
#define RW_SIZE         3

typedef struct {
    struct cdev cdev;
    struct mutex mutex;
    u8 buffer[RW_SIZE];
} gpio_dev_t;
static gpio_dev_t *gpio_dev;
static dev_t gpio_major = GPIO_MAJOR;
static struct class *gpio_class;

module_param(gpio_major, int, S_IRUGO);
static const u8 gpio_minor = 0;

static int _gpio_open(struct inode *p, struct file *f) {
    printk(KERN_NOTICE "[%s] \n", __func__);
    f->private_data = gpio_dev;
    return OK;
}

static int _gpio_read(struct file *f, char __user *u, size_t s, loff_t *l) {
    u8 mode;
    int ret = 0;
    size_t count = s;
    const loff_t pos = *l;
    gpio_dev_t *dev = (gpio_dev_t *) f->private_data;
    if (pos >= RW_SIZE)
        return 0;
    if (count < RW_SIZE)
        return 0;
    if (count > RW_SIZE - pos)
        count = RW_SIZE - pos;

    mutex_lock(&dev->mutex);
    if (copy_from_user(dev->buffer + pos, u, count))
        ret = -EFAULT;
    else {
        *l += count;
        ret = count;
        mode = 0x01 & dev->buffer[0];
        if (mode == 0)
            dev->buffer[2] = gpio_get_value(dev->buffer[1]);
        else
            dev->buffer[2] = gpio_get_mode(dev->buffer[1]);
        printk(KERN_NOTICE "[%s] %d , %d , %d (%d)\n", __func__, dev->buffer[0], dev->buffer[1], dev->buffer[2], count);
        if (copy_to_user(u, dev->buffer + pos, count))
            ret = -EFAULT;
    }
    mutex_unlock(&dev->mutex);
    return ret;
}

static int _gpio_write(struct file *f, const char __user *u, size_t s, loff_t *l) {
    u8 mode;
    int ret = 0;
    size_t count = s;
    const loff_t pos = *l;
    gpio_dev_t *dev = (gpio_dev_t *) f->private_data;
    if (pos >= RW_SIZE)
        return 0;
    if (count < RW_SIZE)
        return 0;
    if (count > RW_SIZE)
        count = RW_SIZE - pos;

    mutex_lock(&dev->mutex);
    if (copy_from_user(dev->buffer + pos, u, count))
        ret = -EFAULT;
    else {
        *l += count;
        ret = s;
        mode = 0x01 & dev->buffer[0];
        if (mode == 0)
            gpio_set_value(dev->buffer[1], dev->buffer[2] & 0x01);
        else
            gpio_set_mode(dev->buffer[1], dev->buffer[2] & 0x07);
        printk(KERN_NOTICE "[%s] %d , %d , %d (%d)\n", __func__, dev->buffer[0], dev->buffer[1], dev->buffer[2], ret);
    }
    mutex_unlock(&dev->mutex);
    // return wrote size, otherwise echo does not work.
    return ret;
}

static long _gpio_ioctl(struct file *f, unsigned int cmd, unsigned long arg) {
    int ret = OK;
    gpio_info_t info;
    gpio_dev_t *dev = (gpio_dev_t *) f->private_data;
    printk(KERN_NOTICE "[%s] cmd = %d \n", __func__, cmd);
    if (_IOC_TYPE(cmd) != IOC_MAGIC) {
        pr_err("[%s] command type [%c] error \n", __func__, _IOC_TYPE(cmd));
        return -ENOTTY;
    }
    if (_IOC_NR(cmd) > IOC_MAXNR) {
        pr_err("[%s] command numer [%d] exceeded \n", __func__, _IOC_NR(cmd));
        return -ENOTTY;
    }
    if (_IOC_DIR(cmd) & _IOC_READ)
        ret = !access_ok(VERIFY_WRITE, (void __user *) arg, _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
        ret = !access_ok(VERIFY_READ, (void __user *) arg, _IOC_SIZE(cmd));
    if (ret)
        return -EFAULT;
    switch (cmd) {
        case GPIO_IOC_GET_MODE:
            mutex_lock(&dev->mutex);
            if (copy_from_user(&info, (gpio_info_t __user *) arg, sizeof(gpio_info_t)))
                ret = -EFAULT;
            else {
                info.mode = gpio_get_mode(info.pin);
                if (copy_to_user((gpio_info_t __user *) arg, &info, sizeof(gpio_info_t)))
                    ret = -EFAULT;
            }
            mutex_unlock(&dev->mutex);
            break;
        case GPIO_IOC_SET_MODE:
            mutex_lock(&dev->mutex);
            if (copy_from_user(&info, (gpio_info_t __user *) arg, sizeof(gpio_info_t)))
                ret = -EFAULT;
            else {
                gpio_set_mode(info.pin, info.mode);
            }
            mutex_unlock(&dev->mutex);
            break;
        case GPIO_IOC_GET_VALUE:
            mutex_lock(&dev->mutex);
            if (copy_from_user(&info, (gpio_info_t __user *) arg, sizeof(gpio_info_t)))
                ret = -EFAULT;
            else {
                info.value = gpio_get_value(info.pin);
                if (copy_to_user((gpio_info_t __user *) arg, &info, sizeof(gpio_info_t)))
                    ret = -EFAULT;
            }
            mutex_unlock(&dev->mutex);
            break;
        case GPIO_IOC_SET_VALUE:
            mutex_lock(&dev->mutex);
            if (copy_from_user(&info, (gpio_info_t __user *) arg, sizeof(gpio_info_t)))
                ret = -EFAULT;
            else {
                gpio_set_value(info.pin, info.value);
            }
            mutex_unlock(&dev->mutex);
            break;
        default:
            return -ENOTTY;
    }
    return ret;
}

static const struct file_operations gpio_file_ops = {
        .owner = THIS_MODULE,
        .open = _gpio_open,
        .read = _gpio_read,
        .write = _gpio_write,
        .unlocked_ioctl = _gpio_ioctl,
};

static char *set_mode(struct device *d, umode_t *mode) {
    *mode = 0666;
    return NULL;
}

static int __init _gpio_init(void) {
    int ret, err;
    dev_t devno;
    printk(KERN_NOTICE "[%s]\n", __func__);
    devno = MKDEV(gpio_major, gpio_minor);
    if (gpio_major)
        ret = register_chrdev_region(devno, 1, GPIO_DRV_NAME);
    else {
        ret = alloc_chrdev_region(&devno, 0, 1, GPIO_DRV_NAME);
        gpio_major = MAJOR(devno);
    }
    if (ret < 0) {
        printk(KERN_ALERT "[%s] create region failed \n", __func__);
        return ret;
    }

    gpio_class = class_create(THIS_MODULE, GPIO_CLS_NAME);
    if (IS_ERR(gpio_class)) {
        printk(KERN_ALERT "[%s] create class failed \n", __func__);
        return -ENOMEM;
    }
    gpio_class->devnode = set_mode;
    printk(KERN_NOTICE "[%s] %p \n", __func__, gpio_class->devnode);

    if (IS_ERR(device_create(gpio_class, NULL, devno, NULL, GPIO_DEV_NAME))) {
        printk(KERN_ALERT "[%s] create device failed \n", __func__);
        ret = -ENOMEM;
        goto fail_create_device;
    }

    gpio_dev = (gpio_dev_t *) kzalloc(sizeof(gpio_dev_t), GFP_KERNEL);
    if (IS_ERR(gpio_dev)) {
        ret = -ENOMEM;
        goto fail_malloc;
    }

    cdev_init(&gpio_dev->cdev, &gpio_file_ops);
    err = cdev_add(&gpio_dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ALERT "Error %d adding gpio%d", err, gpio_minor);
        goto fail_malloc;
    }
    printk(KERN_NOTICE "devno = %d , major = %d , minor = %d \n", devno, gpio_major, gpio_minor);

    mutex_init(&gpio_dev->mutex);
    gpio_init();
    return OK;

    fail_create_device:
    printk(KERN_ALERT "[%s] fail_create_device \n", __func__);
    class_destroy(gpio_class);
    unregister_chrdev_region(devno, 1);
    return ret;

    fail_malloc:
    printk(KERN_ALERT "[%s] fail_malloc \n", __func__);
    device_destroy(gpio_class, devno);
    class_destroy(gpio_class);
    unregister_chrdev_region(devno, 1);
    return ret;
}

static void __exit _gpio_exit(void) {
    dev_t devno = MKDEV(gpio_major, gpio_minor);
    printk(KERN_NOTICE "[%s] rmmod %d, %d, %p\n", __func__, gpio_major, devno, gpio_class->devnode);
    if (IS_ERR(gpio_dev)) {
        printk(KERN_ALERT "[%s] dev is invalid\n", __func__);
        return;
    }
    gpio_deinit();
    // set devnode to NULL, or may cause kernel NULL error.
    gpio_class->devnode = NULL;
    device_destroy(gpio_class, devno);
    class_unregister(gpio_class);
    class_destroy(gpio_class);
    unregister_chrdev_region(devno, 1);
    cdev_del(&gpio_dev->cdev);
    mutex_destroy(&gpio_dev->mutex);
    kfree(gpio_dev);
}

module_init(_gpio_init);
module_exit(_gpio_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("mumumusuc");