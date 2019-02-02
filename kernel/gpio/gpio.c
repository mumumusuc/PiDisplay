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

#define RW_SIZE     (2)

static struct cdev *_cdev;
static dev_t dev_num;
static u8 sub_dev_num = 1;
static int reg_major = 232;
static int reg_minor = 0;
static u8 buffer[RW_SIZE] = {0};

static int _open(struct inode *p, struct file *f) {
    printk(KERN_ALERT "[%s] \n", __func__);
    return OK;
}

static int _read(struct file *f, char __user *u, size_t s, loff_t *l) {
    if (s < RW_SIZE)
        return -EFAULT;
    if (copy_from_user(buffer, u, RW_SIZE))
        return -EFAULT;
    buffer[1] = gpio_get_value(buffer[0]);
    printk(KERN_ALERT "[%s] %d , %d , %d\n", __func__, buffer[0], buffer[1], s);
    if (copy_to_user(u, buffer, RW_SIZE))
        return -EFAULT;
    return s;
}

static int _write(struct file *f, const char __user *u, size_t s, loff_t *l) {
    if (s < RW_SIZE)
        return -EFAULT;
    if (copy_from_user(buffer, u, RW_SIZE))
        return -EFAULT;
    printk(KERN_ALERT "[%s] %d , %d , %d\n", __func__, buffer[0], buffer[1], s);
    gpio_set_value(buffer[0], buffer[1]);
    // return wrote size, otherwise echo does not work.
    return s;
}

static long _ioctl(struct file *f, unsigned int cmd, unsigned long arg) {
    int ret = 0;
    gpio_info_t info;
    printk(KERN_ALERT "[%s] cmd = %d \n", __func__, cmd);
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
            ret = copy_from_user(&info, (gpio_info_t __user *) arg, sizeof(gpio_info_t));
            if (ret)
                return -EFAULT;
            info.mode = gpio_get_mode(info.pin);
            ret = copy_to_user((gpio_info_t __user *) arg, &info, sizeof(gpio_info_t));
            if (ret)
                return -EFAULT;
            break;
        case GPIO_IOC_SET_MODE:
            ret = copy_from_user(&info, (gpio_info_t __user *) arg, sizeof(gpio_info_t));
            if (ret)
                return -EFAULT;
            gpio_set_mode(info.pin, info.mode);
            break;
        case GPIO_IOC_GET_VALUE:
            ret = copy_from_user(&info, (gpio_info_t __user *) arg, sizeof(gpio_info_t));
            if (ret)
                return -EFAULT;
            info.value = gpio_get_value(info.pin);
            ret = copy_to_user((gpio_info_t __user *) arg, &info, sizeof(gpio_info_t));
            if (ret)
                return -EFAULT;
            break;
        case GPIO_IOC_SET_VALUE:
            ret = copy_from_user(&info, (gpio_info_t __user *) arg, sizeof(gpio_info_t));
            if (ret)
                return -EFAULT;
            gpio_set_value(info.pin, info.value);
            break;
        default:
            return -ENOTTY;
    }
    return OK;
}

static const struct file_operations _file_ops = {
        .owner = THIS_MODULE,
        .open = _open,
        .read = _read,
        .write =_write,
        .unlocked_ioctl = _ioctl,
};

static int __init _gpio_init(void) {
    printk(KERN_ALERT "%s.\n", __func__);
    dev_num = MKDEV(reg_major, reg_minor);
    if (OK == register_chrdev_region(dev_num, sub_dev_num, node)) {
        printk(KERN_ALERT "register_chrdev_region OK.\n");
    } else {
        printk(KERN_ALERT "register_chrdev_region ERROR.\n");
        return ERR;
    }
    printk(KERN_ALERT "dev_num = %d.\n", dev_num);
    _cdev = kzalloc(sizeof(struct cdev), GFP_KERNEL);
    cdev_init(_cdev, &_file_ops);
    cdev_add(_cdev, dev_num, 1);
    gpio_init();
    return OK;
}

static void __exit _gpio_exit(void) {
    printk(KERN_ALERT "%s.\n", __func__);
    gpio_deinit();
    cdev_del(_cdev);
    unregister_chrdev_region(dev_num, sub_dev_num);
}

module_init(_gpio_init);
module_exit(_gpio_exit);

MODULE_LICENSE("Dual DSB/GPL");
MODULE_AUTHOR("MUMUMUSUC");