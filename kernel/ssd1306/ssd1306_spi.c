//
// Created by mumumusuc on 19-2-12.
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
#include <asm-generic/bug.h>
#include "ssd1306.h"

#define DRIVER_NAME     "ssd1306_spi"
#define SSD_SPI_SPD     16000000
#define SSD_SPI_MODE    SPI_MODE_0
#define SSD_SPI_BPW     8
#define SSD_SPI_DC0     22
#define SSD_SPI_DC1     23
#define SSD_SPI_RST0    27
#define SSD_SPI_RST1    17

#ifdef USE_BOARD_INFO
static struct spi_board_info display_spi_board_info[] = {
        {
                .modalias       = "ssd1306",
                .bus_num        = 0,
                .chip_select    = 3,
                .max_speed_hz   = SSD_SPI_SPD,
                .mode           = SPI_MODE_0,
        },
};
static struct spi_dev_info{

} spi_dev_info;
#endif

static const u8 spi_gpio_dc[] = {0, 0, SSD_SPI_DC0, SSD_SPI_DC1};
static const u8 spi_gpio_rst[] = {0, 0, SSD_SPI_RST0, SSD_SPI_RST1};
static const struct of_device_id spi_dt_ids[] = {
        {.compatible = "ssd1306"},
        //{.compatible = "spidev"},
        {},
};
MODULE_DEVICE_TABLE(of, spi_dt_ids);

struct spi_dev {
    struct display display;
    struct spi_device *spi;
    spinlock_t spi_lock;
    u8 gpio_dc;
    u32 speed_hz;
    unsigned users;
};

// display interface
ssize_t spi_write_cmd(struct display *, u8);

ssize_t spi_write_data(struct display *, u8);

ssize_t spi_write_data_buffer(struct display *, const u8 *, size_t);

static struct interface spi_interface = {
        .write_cmd          = spi_write_cmd,
        .write_data         = spi_write_data,
        .write_data_buffer  = spi_write_data_buffer,
};

// spi driver
static int spi_driver_probe(struct spi_device *);

static int spi_driver_remove(struct spi_device *);

static struct spi_driver display_spi_driver = {
        .driver = {
                .name =  DRIVER_NAME,
                .owner = THIS_MODULE,
                .of_match_table = of_match_ptr(spi_dt_ids),
        },
        .probe  =  spi_driver_probe,
        .remove =  spi_driver_remove,
};

// private methods
static ssize_t spidev_sync(struct spi_dev *spidev, struct spi_message *message) {
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

static inline ssize_t spi_sync_write(struct spi_dev *spidev, const u8 *tx_buffer, size_t len) {
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

static int spi_driver_probe(struct spi_device *spi) {
    int ret = 0;
    struct spi_dev *spidev;
    char gpio_tmp[16];
    u32 prop_tmp;
    const char *alias = spi->modalias;
    const u8 bus = spi->master->bus_num;
    const u8 cs = spi->chip_select;
    if (spi->dev.of_node && !of_match_device(spi_dt_ids, &spi->dev)) {
        dev_err(&spi->dev, "buggy DT: spidev listed directly in DT\n");
        WARN_ON(spi->dev.of_node && !of_match_device(spi_dt_ids, &spi->dev));
    }
    printk(KERN_DEBUG "[%s] probe %s:%d.%d\n", __func__, alias, bus, cs);

    spidev = kzalloc(sizeof(struct spi_dev), GFP_KERNEL);
    if (IS_ERR_OR_NULL(spidev)) {
        printk(KERN_ALERT "[%s] alloc spidev failed\n", __func__);
        return -ENOMEM;
    }
    // init spi
    spi->max_speed_hz = SSD_SPI_SPD;
    spi->bits_per_word = SSD_SPI_BPW;
    spi->mode |= SSD_SPI_MODE;
    ret = spi_setup(spi);
    if (ret < 0) {
        printk(KERN_ALERT "[%s] setup spi failed\n", __func__);
        goto free_dev;
    }
    // init spi_dev
    spidev->spi = spi;
    spidev->speed_hz = spi->max_speed_hz;
    ret = of_property_read_u32(spi->dev.of_node, "display-dc", &prop_tmp);
    if (ret == 0)
        spidev->gpio_dc = prop_tmp;
    else {
        spidev->gpio_dc = spi_gpio_dc[cs];
        printk(KERN_DEBUG "[%s] display-dc not found , use default value %u\n", __func__, spidev->gpio_dc);
    }
    ret = of_property_read_u32(spi->dev.of_node, "display-reset", &prop_tmp);
    if (ret == 0)
        spi_interface.gpio_reset = prop_tmp;
    else {
        spi_interface.gpio_reset = spi_gpio_rst[cs];
        printk(KERN_DEBUG "[%s] display-reset not found , use default value %u\n", __func__, spi_interface.gpio_reset);
    }
    spin_lock_init(&spidev->spi_lock);
    sprintf(gpio_tmp, "ssd1306_dc_%u", spidev->gpio_dc);
    gpio_request_one(spidev->gpio_dc, GPIOF_OUT_INIT_LOW, gpio_tmp);
    // init display
    ret = display_driver_probe(&spi->dev, &spidev->display, &spi_interface);
    if (ret < 0) {
        printk(KERN_ALERT "[%s] display driver init failed\n", __func__);
        goto free_all;
    }
    // save
    spi_set_drvdata(spi, spidev);
    return ret;

    // failure
    free_all:
    gpio_free(spidev->gpio_dc);
    free_dev:
    kfree(spidev);
    return ret;
}

static int spi_driver_remove(struct spi_device *spi) {
    struct spi_dev *spidev;
    const char *alias = spi->modalias;
    const u8 bus = spi->master->bus_num;
    const u8 cs = spi->chip_select;
    printk(KERN_DEBUG "[%s] remove %s%d.%d\n", __func__, alias, bus, cs);
    spidev = spi_get_drvdata(spi);
    spin_lock_irq(&spidev->spi_lock);
    spidev->spi = NULL;
    spin_unlock_irq(&spidev->spi_lock);
    dislay_driver_remove(&spidev->display);
    if (spidev->users == 0)
        kfree(spidev);
    return 0;
}

ssize_t spi_write_cmd(struct display *display, u8 cmd) {
    ssize_t status = 0;
    struct spi_dev *spidev = container_of(display, struct spi_dev, display);
    struct spi_device *spi = spi_dev_get(spidev->spi);
    if (IS_ERR_OR_NULL(spi)) {
        printk(KERN_ALERT "[%s] get spi failed\n", __func__);
        return -ENODEV;
    }
    gpio_set_value(spidev->gpio_dc, 0);
    status = spi_sync_write(spidev, &cmd, 1);
    spi_dev_put(spi);
    return status;
}

ssize_t spi_write_data(struct display *display, u8 data) {
    ssize_t status = 0;
    struct spi_dev *spidev = container_of(display, struct spi_dev, display);
    struct spi_device *spi = spi_dev_get(spidev->spi);
    if (IS_ERR_OR_NULL(spi)) {
        printk(KERN_ALERT "[%s] get spi failed\n", __func__);
        return -ENODEV;
    }
    gpio_set_value(spidev->gpio_dc, 1);
    status = spi_sync_write(spidev, &data, 1);
    spi_dev_put(spi);
    return status;
}

ssize_t spi_write_data_buffer(struct display *display, const u8 *data, size_t size) {
    ssize_t status = 0;
    struct spi_dev *spidev = container_of(display, struct spi_dev, display);
    struct spi_device *spi = spi_dev_get(spidev->spi);
    if (IS_ERR_OR_NULL(spi)) {
        printk(KERN_ALERT "[%s] get spi failed\n", __func__);
        return -ENODEV;
    }
    gpio_set_value(spidev->gpio_dc, 1);
    status = spi_sync_write(spidev, data, size);
    spi_dev_put(spi);
    return status;
}

int display_driver_spi_init(void) {
    printk(KERN_INFO "[%s] \n", __func__);
    return spi_register_driver(&display_spi_driver);
}

void display_driver_spi_exit(void) {
    printk(KERN_INFO "[%s] \n", __func__);
    spi_unregister_driver(&display_spi_driver);
}
