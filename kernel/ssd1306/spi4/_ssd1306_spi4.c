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


#define DRIVER_NAME     "ssd1306_spi"
#define DEVICE_NAME     "ssd1306"
#define DEVICE_MAJOR    233
#define DEVICE_SPI_BUS  0
#define DEVICE_SPI_CS   0
#define DEVICE_SPI_SPD  8000000

static unsigned bufsiz = 4096;
#define SPI_MODE_MASK           (SPI_CPHA | SPI_CPOL | SPI_CS_HIGH \
                                | SPI_LSB_FIRST | SPI_3WIRE | SPI_LOOP \
                                | SPI_NO_CS | SPI_READY | SPI_TX_DUAL \
                                | SPI_TX_QUAD | SPI_RX_DUAL | SPI_RX_QUAD)
static LIST_HEAD(device_list);
static DEFINE_MUTEX(device_list_lock);

struct spidev_data {
    dev_t devt;
    spinlock_t spi_lock;
    struct spi_device *spi;
    struct list_head device_entry;
    struct mutex buf_lock;
    unsigned users;
    u8 *tx_buffer;
    u8 *rx_buffer;
    u32 speed_hz;
};

#ifdef CONFIG_OF
static const struct of_device_id spidev_dt_ids[] = {
        { .compatible = "rohm,dh2228fv" },
        { .compatible = "lineartechnology,ltc2488" },
        { .compatible = "ge,achc" },
        { .compatible = "semtech,sx1301" },
        { .compatible = "spidev" },
        {},
};
MODULE_DEVICE_TABLE(of, spidev_dt_ids);
#endif


static int ssd_driver_probe(struct spi_device *spi);

static int ssd_driver_remove(struct spi_device *spi);

#ifdef REGISTER_SPI_DEV
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
                .acpi_match_table = ACPI_PTR(spidev_acpi_ids),
        },
        .probe =        ssd_driver_probe,
        .remove =       ssd_driver_remove,
};
#endif

static ssize_t spidev_sync(struct spidev_data *spidev, struct spi_message *message) {
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


static inline ssize_t spidev_sync_write(struct spidev_data *spidev, size_t len) {
    struct spi_transfer t = {
            .tx_buf         = spidev->tx_buffer,
            .len            = len,
            .speed_hz       = spidev->speed_hz,
    };
    struct spi_message m;

    spi_message_init(&m);
    spi_message_add_tail(&t, &m);
    return spidev_sync(spidev, &m);
}


static inline ssize_t spidev_sync_read(struct spidev_data *spidev, size_t len) {
    struct spi_transfer t = {
            .rx_buf         = spidev->rx_buffer,
            .len            = len,
            .speed_hz       = spidev->speed_hz,
    };
    struct spi_message m;

    spi_message_init(&m);
    spi_message_add_tail(&t, &m);
    return spidev_sync(spidev, &m);
}

static ssize_t spidev_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    struct spidev_data *spidev;
    ssize_t status = 0;
    unsigned long missing;
    printk(KERN_NOTICE "[%s] \n", __func__);
    if (count > bufsiz)
        return -EMSGSIZE;

    spidev = filp->private_data;

    mutex_lock(&spidev->buf_lock);
    missing = copy_from_user(spidev->tx_buffer, buf, count);
    if (missing == 0)
        status = spidev_sync_write(spidev, count);
    else
        status = -EFAULT;
    mutex_unlock(&spidev->buf_lock);

    return status;
}

static ssize_t spidev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    struct spidev_data *spidev;
    ssize_t status = 0;
    printk(KERN_NOTICE "[%s] \n", __func__);
    if (count > bufsiz)
        return -EMSGSIZE;

    spidev = filp->private_data;

    mutex_lock(&spidev->buf_lock);
    status = spidev_sync_read(spidev, count);
    if (status > 0) {
        unsigned long missing;

        missing = copy_to_user(buf, spidev->rx_buffer, status);
        if (missing == status)
            status = -EFAULT;
        else
            status = status - missing;
    }
    mutex_unlock(&spidev->buf_lock);

    return status;
}

static struct spi_ioc_transfer *
spidev_get_ioc_message(unsigned int cmd, struct spi_ioc_transfer __user *u_ioc, unsigned *n_ioc) {
    u32 tmp;

    /* Check type, command number and direction */
    if (_IOC_TYPE(cmd) != SPI_IOC_MAGIC
        || _IOC_NR(cmd) != _IOC_NR(SPI_IOC_MESSAGE(0))
        || _IOC_DIR(cmd) != _IOC_WRITE)
        return ERR_PTR(-ENOTTY);

    tmp = _IOC_SIZE(cmd);
    if ((tmp % sizeof(struct spi_ioc_transfer)) != 0)
        return ERR_PTR(-EINVAL);
    *n_ioc = tmp / sizeof(struct spi_ioc_transfer);
    if (*n_ioc == 0)
        return NULL;

    /* copy into scratch area */
    return memdup_user(u_ioc, tmp);
}

static int spidev_message(struct spidev_data *spidev,
                          struct spi_ioc_transfer *u_xfers, unsigned n_xfers) {
    struct spi_message msg;
    struct spi_transfer *k_xfers;
    struct spi_transfer *k_tmp;
    struct spi_ioc_transfer *u_tmp;
    unsigned n, total, tx_total, rx_total;
    u8 *tx_buf, *rx_buf;
    int status = -EFAULT;

    spi_message_init(&msg);
    k_xfers = kcalloc(n_xfers, sizeof(*k_tmp), GFP_KERNEL);
    if (k_xfers == NULL)
        return -ENOMEM;

    /* Construct spi_message, copying any tx data to bounce buffer.
     * We walk the array of user-provided transfers, using each one
     * to initialize a kernel version of the same transfer.
     */
    tx_buf = spidev->tx_buffer;
    rx_buf = spidev->rx_buffer;
    total = 0;
    tx_total = 0;
    rx_total = 0;
    for (n = n_xfers, k_tmp = k_xfers, u_tmp = u_xfers;
         n;
         n--, k_tmp++, u_tmp++) {
        k_tmp->len = u_tmp->len;

        total += k_tmp->len;
        /* Since the function returns the total length of transfers
         * on success, restrict the total to positive int values to
         * avoid the return value looking like an error.  Also check
         * each transfer length to avoid arithmetic overflow.
         */
        if (total > INT_MAX || k_tmp->len > INT_MAX) {
            status = -EMSGSIZE;
            goto done;
        }

        if (u_tmp->rx_buf) {
            /* this transfer needs space in RX bounce buffer */
            rx_total += k_tmp->len;
            if (rx_total > bufsiz) {
                status = -EMSGSIZE;
                goto done;
            }
            k_tmp->rx_buf = rx_buf;
            rx_buf += k_tmp->len;
        }
        if (u_tmp->tx_buf) {
            /* this transfer needs space in TX bounce buffer */
            tx_total += k_tmp->len;
            if (tx_total > bufsiz) {
                status = -EMSGSIZE;
                goto done;
            }
            k_tmp->tx_buf = tx_buf;
            if (copy_from_user(tx_buf, (const u8 __user *)
                                       (uintptr_t) u_tmp->tx_buf,
                               u_tmp->len))
                goto done;
            tx_buf += k_tmp->len;
        }

        k_tmp->cs_change = !!u_tmp->cs_change;
        k_tmp->tx_nbits = u_tmp->tx_nbits;
        k_tmp->rx_nbits = u_tmp->rx_nbits;
        k_tmp->bits_per_word = u_tmp->bits_per_word;
        k_tmp->delay_usecs = u_tmp->delay_usecs;
        k_tmp->speed_hz = u_tmp->speed_hz;
        if (!k_tmp->speed_hz)
            k_tmp->speed_hz = spidev->speed_hz;
#ifdef VERBOSE
        dev_dbg(&spidev->spi->dev,
                    "  xfer len %u %s%s%s%dbits %u usec %uHz\n",
                    u_tmp->len,
                    u_tmp->rx_buf ? "rx " : "",
                    u_tmp->tx_buf ? "tx " : "",
                    u_tmp->cs_change ? "cs " : "",
                    u_tmp->bits_per_word ? : spidev->spi->bits_per_word,
                    u_tmp->delay_usecs,
                    u_tmp->speed_hz ? : spidev->spi->max_speed_hz);
#endif
        spi_message_add_tail(k_tmp, &msg);
    }

    status = spidev_sync(spidev, &msg);
    if (status < 0)
        goto done;

    /* copy any rx data out of bounce buffer */
    rx_buf = spidev->rx_buffer;
    for (n = n_xfers, u_tmp = u_xfers; n; n--, u_tmp++) {
        if (u_tmp->rx_buf) {
            if (copy_to_user((u8 __user *)
                                     (uintptr_t) u_tmp->rx_buf, rx_buf,
                             u_tmp->len)) {
                status = -EFAULT;
                goto done;
            }
            rx_buf += u_tmp->len;
        }
    }
    status = total;

    done:
    kfree(k_xfers);
    return status;
}


static long spidev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    int retval = 0;
    struct spidev_data *spidev;
    struct spi_device *spi;
    u32 tmp;
    unsigned n_ioc;
    struct spi_ioc_transfer *ioc;
    printk(KERN_NOTICE "[%s] \n", __func__);
    if (_IOC_TYPE(cmd) != SPI_IOC_MAGIC)
        return -ENOTTY;

    spidev = filp->private_data;
    spin_lock_irq(&spidev->spi_lock);
    spi = spi_dev_get(spidev->spi);
    spin_unlock_irq(&spidev->spi_lock);

    if (spi == NULL)
        return -ESHUTDOWN;

    mutex_lock(&spidev->buf_lock);

    switch (cmd) {
        /* read requests */
        case SPI_IOC_RD_MODE:
            retval = put_user(spi->mode & SPI_MODE_MASK, (__u8
                    __user *) arg);
            break;
        case SPI_IOC_RD_MODE32:
            retval = put_user(spi->mode & SPI_MODE_MASK, (__u32
                    __user *) arg);
            break;
        case SPI_IOC_RD_LSB_FIRST:
            retval = put_user((spi->mode & SPI_LSB_FIRST) ? 1 : 0, (__u8
                    __user *) arg);
            break;
        case SPI_IOC_RD_BITS_PER_WORD:
            retval = put_user(spi->bits_per_word, (__u8
                    __user *) arg);
            break;
        case SPI_IOC_RD_MAX_SPEED_HZ:
            retval = put_user(spidev->speed_hz, (__u32
                    __user *) arg);
            break;
            /* write requests */
        case SPI_IOC_WR_MODE:
        case SPI_IOC_WR_MODE32:
            if (cmd == SPI_IOC_WR_MODE)
                retval = get_user(tmp, (u8
                        __user *) arg);
            else
                retval = get_user(tmp, (u32
                        __user *) arg);
            if (retval == 0) {
                u32 save = spi->mode;

                if (tmp & ~SPI_MODE_MASK) {
                    retval = -EINVAL;
                    break;
                }

                tmp |= spi->mode & ~SPI_MODE_MASK;
                spi->mode = (u16) tmp;
                retval = spi_setup(spi);
                if (retval < 0)
                    spi->mode = save;
                else
                    dev_dbg(&spi->dev, "spi mode %x\n", tmp);
            }
            break;
        case SPI_IOC_WR_LSB_FIRST:
            retval = get_user(tmp, (__u8
                    __user *) arg);
            if (retval == 0) {
                u32 save = spi->mode;

                if (tmp)
                    spi->mode |= SPI_LSB_FIRST;
                else
                    spi->mode &= ~SPI_LSB_FIRST;
                retval = spi_setup(spi);
                if (retval < 0)
                    spi->mode = save;
                else
                    dev_dbg(&spi->dev, "%csb first\n", tmp ? 'l' : 'm');
            }
            break;
        case SPI_IOC_WR_BITS_PER_WORD:
            retval = get_user(tmp, (__u8
                    __user *) arg);
            if (retval == 0) {
                u8 save = spi->bits_per_word;

                spi->bits_per_word = tmp;
                retval = spi_setup(spi);
                if (retval < 0)
                    spi->bits_per_word = save;
                else
                    dev_dbg(&spi->dev, "%d bits per word\n", tmp);
            }
            break;
        case SPI_IOC_WR_MAX_SPEED_HZ:
            retval = get_user(tmp, (__u32
                    __user *) arg);
            if (retval == 0) {
                u32 save = spi->max_speed_hz;

                spi->max_speed_hz = tmp;
                retval = spi_setup(spi);
                if (retval >= 0)
                    spidev->speed_hz = tmp;
                else
                    dev_dbg(&spi->dev, "%d Hz (max)\n", tmp);
                spi->max_speed_hz = save;
            }
            break;

        default:
            ioc = spidev_get_ioc_message(cmd, (struct spi_ioc_transfer __user *) arg, &n_ioc);
            if (IS_ERR(ioc)) {
                retval = PTR_ERR(ioc);
                break;
            }
            if (!ioc)
                break;  /* n_ioc is also 0 */

            /* translate to spi_message, execute */
            retval = spidev_message(spidev, ioc, n_ioc);
            kfree(ioc);
            break;
    }

    mutex_unlock(&spidev->buf_lock);
    spi_dev_put(spi);
    return retval;
}

static int spidev_open(struct inode *inode, struct file *filp) {
    struct spidev_data *spidev;
    int status = -ENXIO;
    printk(KERN_NOTICE "[%s] \n", __func__);
    mutex_lock(&device_list_lock);

    list_for_each_entry(spidev, &device_list, device_entry) {
        if (spidev->devt == inode->i_rdev) {
            status = 0;
            break;
        }
    }

    if (status) {
        pr_debug("spidev: nothing for minor %d\n", iminor(inode));
        goto err_find_dev;
    }

    if (!spidev->tx_buffer) {
        spidev->tx_buffer = kmalloc(bufsiz, GFP_KERNEL);
        if (!spidev->tx_buffer) {
            dev_dbg(&spidev->spi->dev, "open/ENOMEM\n");
            status = -ENOMEM;
            goto err_find_dev;
        }
    }

    if (!spidev->rx_buffer) {
        spidev->rx_buffer = kmalloc(bufsiz, GFP_KERNEL);
        if (!spidev->rx_buffer) {
            dev_dbg(&spidev->spi->dev, "open/ENOMEM\n");
            status = -ENOMEM;
            goto err_alloc_rx_buf;
        }
    }

    spidev->users++;
    filp->private_data = spidev;
    nonseekable_open(inode, filp);

    mutex_unlock(&device_list_lock);
    return 0;

    err_alloc_rx_buf:
    kfree(spidev->tx_buffer);
    spidev->tx_buffer = NULL;
    err_find_dev:
    mutex_unlock(&device_list_lock);
    return status;
}

static int spidev_release(struct inode *inode, struct file *filp) {
    struct spidev_data *spidev;
    printk(KERN_NOTICE "[%s] \n", __func__);
    mutex_lock(&device_list_lock);
    spidev = filp->private_data;
    filp->private_data = NULL;

    /* last close? */
    spidev->users--;
    if (!spidev->users) {
        int dofree;

        kfree(spidev->tx_buffer);
        spidev->tx_buffer = NULL;

        kfree(spidev->rx_buffer);
        spidev->rx_buffer = NULL;

        spin_lock_irq(&spidev->spi_lock);
        if (spidev->spi)
            spidev->speed_hz = spidev->spi->max_speed_hz;

        /* ... after we unbound from the underlying device? */
        dofree = (spidev->spi == NULL);
        spin_unlock_irq(&spidev->spi_lock);

        if (dofree)
            kfree(spidev);
    }
    mutex_unlock(&device_list_lock);

    return 0;
}


static const struct file_operations spidev_fops = {
        .owner =        THIS_MODULE,
        .write =        spidev_write,
        .read =         spidev_read,
        .unlocked_ioctl = spidev_ioctl,
        .open =         spidev_open,
        .release =      spidev_release,
        .llseek =       no_llseek,
};


static __init int ssd_spi_init(void) {
    int status;
    printk(KERN_NOTICE "[%s] \n", __func__);
    status = register_chrdev(DEVICE_MAJOR, DRIVER_NAME, &spidev_fops);
    if (status < 0)
        return status;

    status = spi_register_driver(&ssd_spi_driver);
    if (status < 0) {
        unregister_chrdev(DEVICE_MAJOR, DRIVER_NAME);
    }
    return status;
}

static __exit void ssd_spi_exit(void) {
    printk(KERN_NOTICE "[%s] \n", __func__);
    spi_unregister_driver(&ssd_spi_driver);
    unregister_chrdev(DEVICE_MAJOR, DRIVER_NAME);
}

#ifdef CONFIG_ACPI

/* Dummy SPI devices not to be used in production systems */
#define SPIDEV_ACPI_DUMMY       1

static const struct acpi_device_id spidev_acpi_ids[] = {
        /*
         * The ACPI SPT000* devices are only meant for development and
         * testing. Systems used in production should have a proper ACPI
         * description of the connected peripheral and they should also use
         * a proper driver instead of poking directly to the SPI bus.
         */
        { "SPT0001", SPIDEV_ACPI_DUMMY },
        { "SPT0002", SPIDEV_ACPI_DUMMY },
        { "SPT0003", SPIDEV_ACPI_DUMMY },
        {},
};
MODULE_DEVICE_TABLE(acpi, spidev_acpi_ids);

static void spidev_probe_acpi(struct spi_device *spi)
{
        const struct acpi_device_id *id;

        if (!has_acpi_companion(&spi->dev))
                return;

        id = acpi_match_device(spidev_acpi_ids, &spi->dev);
        if (WARN_ON(!id))
                return;

        if (id->driver_data == SPIDEV_ACPI_DUMMY)
                dev_warn(&spi->dev, "do not use this driver in production systems!\n");
}
#else

static inline void spidev_probe_acpi(struct spi_device *spi) {}

#endif


static int ssd_driver_probe(struct spi_device *spi) {
    struct spidev_data *spidev;
    int status = 0;
    printk(KERN_NOTICE "[%s] \n", __func__);
    if (spi->dev.of_node && !of_match_device(spidev_dt_ids, &spi->dev)) {
        dev_err(&spi->dev, "buggy DT: spidev listed directly in DT\n");
        WARN_ON(spi->dev.of_node && !of_match_device(spidev_dt_ids, &spi->dev));
    }

    spidev_probe_acpi(spi);

    /* Allocate driver data */
    spidev = kzalloc(sizeof(*spidev), GFP_KERNEL);
    if (!spidev)
        return -ENOMEM;

    /* Initialize the driver data */
    spidev->spi = spi;
    spin_lock_init(&spidev->spi_lock);
    mutex_init(&spidev->buf_lock);

    INIT_LIST_HEAD(&spidev->device_entry);

    spidev->speed_hz = spi->max_speed_hz;

    if (status == 0)
        spi_set_drvdata(spi, spidev);
    else
        kfree(spidev);

    return status;
}

static int ssd_driver_remove(struct spi_device *spi) {
    struct spidev_data *spidev = spi_get_drvdata(spi);
    printk(KERN_NOTICE "[%s] \n", __func__);

    /* make sure ops on existing fds can abort cleanly */
    spin_lock_irq(&spidev->spi_lock);
    spidev->spi = NULL;
    spin_unlock_irq(&spidev->spi_lock);

    /* prevent new opens */
    mutex_lock(&device_list_lock);
    list_del(&spidev->device_entry);
    if (spidev->users == 0)
        kfree(spidev);
    mutex_unlock(&device_list_lock);

    return 0;

}

module_init(ssd_spi_init);
module_exit(ssd_spi_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("mumumusuc");