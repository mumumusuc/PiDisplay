//
// Created by mumumusuc on 19-1-31.
//

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

#define OK          (0)
#define ERR         (-1)
#define BUFFER_MAX  (10)

struct cdev *hello_cdev;
struct file_operations *hello_file_ops;
dev_t dev_num;
uint8_t sub_dev_num = 1;
uint8_t buffer[BUFFER_MAX] = "hello.";
int reg_major = 232;
int reg_minor = 0;
int flag = 0;

int hello_open(struct inode *p, struct file *f) {
    printk(KERN_ALERT "%s.\n", __func__);
    return OK;
}

int hello_read(struct file *f, char __user *u, size_t s, loff_t *l) {
    printk(KERN_ALERT "%s.\n", __func__);
    strcpy(u, buffer);
    return OK;
}

int hello_write(struct file *f, const char __user *u, size_t s, loff_t *l) {
    printk(KERN_ALERT "%s.\n", __func__);
    strcpy(buffer, u);
    return OK;
}

int __init hello_init(void) {
    printk(KERN_ALERT "%s.\n", __func__);
    dev_num = MKDEV(reg_major, reg_minor);
    if (OK == register_chrdev_region(dev_num, sub_dev_num, "hello_char")) {
        printk(KERN_ALERT "register_chrdev_region OK.\n");
    } else {
        printk(KERN_ALERT "register_chrdev_region OK.\n");
        return ERR;
    }
    printk(KERN_ALERT "dev_num = %d.\n", dev_num);
    hello_cdev = kzalloc(sizeof(struct cdev), GFP_KERNEL);
    hello_file_ops = kzalloc(sizeof(struct file_operations), GFP_KERNEL);
    hello_file_ops->open = hello_open;
    hello_file_ops->read = hello_read;
    hello_file_ops->write = hello_write;
    hello_file_ops->owner = THIS_MODULE;
    cdev_init(hello_cdev, hello_file_ops);
    cdev_add(hello_cdev, dev_num, 3);

    return OK;
}

void __exit hello_exit(void) {
    printk(KERN_ALERT "%s.\n", __func__);
    cdev_del(hello_cdev);
    unregister_chrdev_region(dev_num, sub_dev_num);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("Dual DSB/GPL");
MODULE_AUTHOR("MUMUMUSUC");