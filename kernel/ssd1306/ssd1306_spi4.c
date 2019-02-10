//
// Created by mumumusuc on 19-2-6.
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
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/acpi.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include "ssd1306_dev.h"


#define DRIVER_NAME     "ssd1306_spi4"
#define CLASS_NAME      "ssd1306_spi4"
#define DEVICE_NAME     "ssd1306_spi4"
#define DEVICE_MAJOR    233
#define DEVICE_MINOR    0
#define DEVICE_MODE     S_IWUGO|S_IRUGO

#define SSD_SPI_SPD     8000000
#define SSD_SPI_MODE    SPI_MODE_0
#define SSD_SPI_BPW     8
#define SSD_SPI_DC      22
#define SSD_SPI_RST     27

static unsigned bufsiz = SCREEN_COLUMNS * SCREEN_ROWS / 8;

module_param(bufsiz, int, S_IRUGO);
static unsigned minor = 0;

static display_info_t ssd1306_info = {
        .width  =   SCREEN_COLUMNS,
        .height =   SCREEN_ROWS,
        .format =   1,
        .size   =   SCREEN_COLUMNS * SCREEN_ROWS / 8,
        .vendor =   "SSD1306_128_64",
};

typedef struct {
    dev_t devt;
    spinlock_t spi_lock;
    struct spi_device *spi;
    struct mutex buf_lock;
    unsigned users;
    u32 speed_hz;
    u8 gpio_dc;
    display_t display;
} ssd_spi_data_t;

static const struct of_device_id spidev_dt_ids[] = {
        {.compatible = "ssd1306"},
        {.compatible = "spidev"},
        {},
};
MODULE_DEVICE_TABLE(of, spidev_dt_ids);

static struct class *ssd_spi_class;

static ssd_spi_data_t *ssd_spi;

static int ssd_driver_probe(struct spi_device *spi);

static int ssd_driver_remove(struct spi_device *spi);

#ifdef REGISTER_SSD_SPI_DEV
static struct spi_device *ssd_spi;
static const struct spi_device_id ssd_spi_id[] = {
        {DEVICE_NAME, 0},
        {},
};
static struct spi_driver ssd_spi_driver = {
        .id_table = ssd_spi_id,
        .probe = ssd_driver_prob,
        .remove = ssd_driver_remove,
        .driver={
                .name = DEVICE_NAME,
                .owner = THIS_MODULE,
        }
};
static struct spi_board_info ssd_spi_info = {
        .mode = 0x00,
        .modalias = DEVICE_NAME,
        .bus_num = DEVICE_SPI_BUS,
        .chip_select = DEVICE_SPI_CS,
        .max_speed_hz = DEVICE_SPI_SPD,
};
#else
static struct spi_driver ssd_spi_driver = {
        .driver = {
                .name =  DRIVER_NAME,
                .owner = THIS_MODULE,
                .of_match_table = of_match_ptr(spidev_dt_ids),
        },
        .probe =        ssd_driver_probe,
        .remove =       ssd_driver_remove,
};
#endif


static ssize_t spidev_sync(ssd_spi_data_t *spidev, struct spi_message *message) {
    int status;
    struct spi_device *spi;
    spin_lock_irq(&spidev->spi_lock);
    spi = spidev->spi;
    spin_unlock_irq(&spidev->spi_lock);
    if (spi == NULL)
        status = -ESHUTDOWN;
    else
        status = spi_sync(spi, message);
    if (status == 0)
        status = message->actual_length;

    return status;
}

static inline ssize_t spidev_sync_write(ssd_spi_data_t *spidev, const u8 *tx_buffer, size_t len) {
    struct spi_transfer t = {
            .tx_buf         = tx_buffer,
            .len            = len,
            .speed_hz       = spidev->speed_hz,
    };
    struct spi_message m;
    spi_message_init(&m);
    spi_message_add_tail(&t, &m);
    return spidev_sync(spidev, &m);
}

static ssize_t ssd_spi_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    ssd_spi_data_t *spidev;
    ssize_t status = 0;
    if (count > bufsiz)
        return -EMSGSIZE;
    spidev = filp->private_data;
    mutex_lock(&spidev->buf_lock);
    status = copy_from_user(spidev->display.screen, (u8 __user *) buf, count);
    mutex_unlock(&spidev->buf_lock);
    if (status == 0) {
        display_update(&spidev->display, count);
        status = count;
    }
    return status;
}

static ssize_t ssd_spi_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    ssize_t status = 0;
    printk(KERN_INFO "[%s] \n", __func__);
    return status;
}

static long ssd_spi_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    int retval = 0;
    ssd_spi_data_t *spidev;
    struct spi_device *spi;
    const size_t buf_size = ssd1306_info.size;
    if (_IOC_TYPE(cmd) != SSD_IOC_MAGIC)
        return -ENOTTY;
    if (_IOC_NR(cmd) >= SSD_IOC_MAXNR)
        return -ENOTTY;
    spidev = filp->private_data;
    spin_lock_irq(&spidev->spi_lock);
    spi = spi_dev_get(spidev->spi);
    spin_unlock_irq(&spidev->spi_lock);

    if (spi == NULL)
        return -ESHUTDOWN;

    switch (cmd) {
        case SSD_IOC_RSET:
            display_reset(&spidev->display);
            break;
        case SSD_IOC_INFO:
            mutex_lock(&spidev->buf_lock);
            retval = copy_to_user((display_info_t __user *) arg, &ssd1306_info, sizeof(display_info_t));
            mutex_unlock(&spidev->buf_lock);
            break;
        case SSD_IOC_CLEAR:
            display_clear(&spidev->display);
            break;
        case SSD_IOC_ON:
            display_turn_on(&spidev->display);
            break;
        case SSD_IOC_OFF:
            display_turn_off(&spidev->display);
            break;
        case SSD_IOC_DISPLAY:
            mutex_lock(&spidev->buf_lock);
            retval = copy_from_user(spidev->display.screen, (u8 __user *) arg, buf_size);
            mutex_unlock(&spidev->buf_lock);
            if (retval == 0)
                display_update(&spidev->display, buf_size);
            break;
        default:
            break;
    }
    spi_dev_put(spi);
    return retval;
}

static int ssd_spi_open(struct inode *inode, struct file *filp) {
    ssd_spi_data_t *spidev = ssd_spi;
    int status = -ENXIO;
    printk(KERN_INFO
           "[%s] \n", __func__);
    if (spidev->devt != inode->i_rdev) {
        printk(KERN_ALERT "[%s] nothing for minor %d\n", __func__, iminor(inode));
        goto err_find_dev;
    }
    mutex_lock(&spidev->buf_lock);
    display_init(&spidev->display);
    mutex_unlock(&spidev->buf_lock);
    spidev->users++;
    printk(KERN_INFO "[%s] current users = %u\n", __func__, spidev->users);
    filp->private_data = spidev;
    nonseekable_open(inode, filp);
    return 0;

    err_find_dev:
    return status;
}

static int ssd_spi_release(struct inode *inode, struct file *filp) {
    ssd_spi_data_t *spidev;
    spidev = filp->private_data;
    filp->private_data = NULL;
    printk(KERN_INFO "[%s] current users = %u\n", __func__, spidev->users);
    spidev->users--;
    if (!spidev->users) {
        int dofree;
        mutex_lock(&spidev->buf_lock);
        display_deinit(&spidev->display);
        mutex_unlock(&spidev->buf_lock);
        spin_lock_irq(&spidev->spi_lock);
        if (spidev->spi)
            spidev->speed_hz = spidev->spi->max_speed_hz;
        dofree = (spidev->spi == NULL);
        spin_unlock_irq(&spidev->spi_lock);
        if (dofree)
            kfree(spidev);
    }
    return 0;
}

// define com methods
ssize_t write_cmd(struct display *dev, u8 cmd) {
    ssize_t status = 0;
    struct spi_device *spi;
    ssd_spi_data_t *spidev = container_of(dev, ssd_spi_data_t, display);
    printk(KERN_INFO "[%s] \n", __func__);
    spi = spi_dev_get(spidev->spi);
    if (IS_ERR_OR_NULL(spi)) {
        printk(KERN_ALERT "[%s] get spi failed", __func__);
        return -ENODEV;
    }
    mutex_lock(&spidev->buf_lock);
    gpio_set_value(spidev->gpio_dc, GPIO_LO);
    status = spidev_sync_write(spidev, &cmd, 1);
    mutex_unlock(&spidev->buf_lock);
    spi_dev_put(spi);
    return status;
}

ssize_t write_data(struct display *dev, u8 data) {
    ssize_t status = 0;
    ssd_spi_data_t *spidev = container_of(dev, ssd_spi_data_t, display);
    struct spi_device *spi = spi_dev_get(spidev->spi);
    printk(KERN_INFO "[%s] \n", __func__);
    if (IS_ERR_OR_NULL(spi)) {
        printk(KERN_ALERT "[%s] get spi failed", __func__);
        return -ENODEV;
    }
    mutex_lock(&spidev->buf_lock);
    gpio_set_value(spidev->gpio_dc, GPIO_HI);
    status = spidev_sync_write(spidev, &data, 1);
    mutex_unlock(&spidev->buf_lock);
    spi_dev_put(spi);
    return status;
}

ssize_t write_data_buffer(struct display *dev, const u8 *data, size_t size) {
    ssize_t status = 0;
    ssd_spi_data_t *spidev = container_of(dev, ssd_spi_data_t, display);
    struct spi_device *spi = spi_dev_get(spidev->spi);
    printk(KERN_INFO "[%s] \n", __func__);
    if (IS_ERR_OR_NULL(spi)) {
        printk(KERN_ALERT "[%s] get spi failed", __func__);
        return -ENODEV;
    }
    mutex_lock(&spidev->buf_lock);
    gpio_set_value(spidev->gpio_dc, GPIO_HI);
    status = spidev_sync_write(spidev, data, size);
    mutex_unlock(&spidev->buf_lock);
    spi_dev_put(spi);
    return status;
}
// end define com methods

// define driver methods
static int ssd_driver_probe(struct spi_device *spi) {
    ssd_spi_data_t *spidev;
    struct device *dev;
    int status = 0;
    if (spi->dev.of_node && !of_match_device(spidev_dt_ids, &spi->dev)) {
        dev_err(&spi->dev, "buggy DT: spidev listed directly in DT\n");
        WARN_ON(spi->dev.of_node && !of_match_device(spidev_dt_ids, &spi->dev));
    }
    if (minor > 0)
        return 0;
    printk(KERN_INFO "[%s] \n", __func__);
    spidev = kzalloc(sizeof(*spidev), GFP_KERNEL);
    if (!spidev)
        return -ENOMEM;
    // setup spi params
    spi->mode |= SSD_SPI_MODE;
    spi->bits_per_word = SSD_SPI_BPW;
    spi->max_speed_hz = SSD_SPI_SPD;
    if (spi_setup(spi) < 0)
        printk(KERN_ALERT "[%s] setup spi failed", __func__);
    spidev->spi = spi;
    spidev->gpio_dc = SSD_SPI_DC;
    spidev->speed_hz = spi->max_speed_hz;
    spidev->display.write_cmd = write_cmd;
    spidev->display.write_data = write_data;
    spidev->display.write_data_buffer = write_data_buffer;
    spidev->display.gpio_reset = SSD_SPI_RST;
    gpio_request_one(spidev->gpio_dc, GPIOF_OUT_INIT_LOW, "ssd_spi_dc");
    spin_lock_init(&spidev->spi_lock);
    mutex_init(&spidev->buf_lock);
    printk(KERN_INFO "[%s] set spi speed = %d", __func__, spidev->speed_hz);
    spi_dev_put(spi);
    // create device
    printk(KERN_INFO "[%s] create %s%d.%d\n", __func__, DEVICE_NAME, spi->master->bus_num, spi->chip_select);
    spidev->devt = MKDEV(DEVICE_MAJOR, DEVICE_MINOR);
    dev = device_create(ssd_spi_class,
                        &spi->dev,
                        spidev->devt,
                        spidev,
                        DEVICE_NAME);
    status = PTR_ERR_OR_ZERO(dev);
    if (status == 0) {
        spi_set_drvdata(spi, spidev);
        ssd_spi = spidev;
        minor += 1;
    } else
        kfree(spidev);

    return status;
}

static int ssd_driver_remove(struct spi_device *spi) {
    ssd_spi_data_t *spidev;
    spidev = spi_get_drvdata(spi);
    if (IS_ERR_OR_NULL(spidev))
        return 0;
    printk(KERN_INFO "[%s]\n", __func__);
    spin_lock_irq(&spidev->spi_lock);
    spidev->spi = NULL;
    spin_unlock_irq(&spidev->spi_lock);
    gpio_free(spidev->gpio_dc);
    // destroy device
    device_destroy(ssd_spi_class, spidev->devt);
    if (spidev->users == 0) {
        kfree(spidev);
    }
    return 0;
}
// end define driver methods

// define init & exit
static const struct file_operations ssd_spi_fops = {
        .owner          =   THIS_MODULE,
        .write          =   ssd_spi_write,
        .read           =   ssd_spi_read,
        .unlocked_ioctl =   ssd_spi_ioctl,
        .open           =   ssd_spi_open,
        .release        =   ssd_spi_release,
        .llseek         =   no_llseek,
};

static inline char *set_dev_mode(struct device *dev, umode_t *mode) {
    *mode = DEVICE_MODE;
    return NULL;
}

static __init int ssd_spi_init(void) {
    int status;
    printk(KERN_INFO "[%s] \n", __func__);
    // register chrdev
    status = register_chrdev(DEVICE_MAJOR, DRIVER_NAME, &ssd_spi_fops);
    if (status < 0) {
        printk(KERN_ALERT "[%s] register chrdev failed ", __func__);
        return status;
    }
    // register class
    ssd_spi_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(ssd_spi_class)) {
        printk(KERN_ALERT "[%s] create class failed \n", __func__);
        status = PTR_ERR(ssd_spi_class);
        goto clear_device;
    }
    ssd_spi_class->devnode = set_dev_mode;
    // register driver
    status = spi_register_driver(&ssd_spi_driver);
    if (status < 0) {
        printk(KERN_ALERT "[%s] register driver failed ", __func__);
        goto clear_class;
    }
    return status;
    // clear
    clear_class:
    class_destroy(ssd_spi_class);
    clear_device:
    unregister_chrdev(DEVICE_MAJOR, DRIVER_NAME);
    return status;
}

static __exit void ssd_spi_exit(void) {
    printk(KERN_INFO "[%s] \n", __func__);
    ssd_spi_class->devnode = NULL;
    spi_unregister_driver(&ssd_spi_driver);
    class_destroy(ssd_spi_class);
    unregister_chrdev(DEVICE_MAJOR, DRIVER_NAME);
}
// end define init & exit

module_init(ssd_spi_init);
module_exit(ssd_spi_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("mumumusuc");