
#include <linux/module.h>
#include <linux/moduleparam.h>
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
#include "ssd1306_dev.h"
#include "display.h"

#define REGISTER_SPI_BOARD_INFO
#define DRIVER_NAME     "ssd1306"
#define DEVICE_NAME     "ssd1306"
#define SSD_SPI_SPD     32000000
#define SSD_SPI_MODE    SPI_MODE_0
#define SSD_SPI_BPW     8

#ifdef REGISTER_SPI_BOARD_INFO
struct spi_dev_info {
    u8 gpio_dc;
    u8 gpio_reset;
};
static struct spi_board_info ssd1306_spi_board_info[] = {
        {
                .modalias           = DEVICE_NAME,
                .bus_num            = 0,
                .chip_select        = 0,
                .max_speed_hz       = SSD_SPI_SPD,
                .mode               = SPI_MODE_0,
                .controller_data    = &(struct spi_dev_info) {
                        .gpio_dc = 22,
                        .gpio_reset = 27,
                },
        },
        {
                .modalias           = DEVICE_NAME,
                .bus_num            = 0,
                .chip_select        = 1,
                .max_speed_hz       = SSD_SPI_SPD,
                .mode               = SPI_MODE_0,
                .controller_data    = &(struct spi_dev_info) {
                        .gpio_dc = 23,
                        .gpio_reset = 17,
                },
        },
};

static int register_spi_device(struct spi_board_info *info) {
    char name[16];
    struct device *dev;
    struct spi_master *master;
    struct spi_device *spi;
#ifdef DEBUG
    struct spi_dev_info *data = info->controller_data;
    debug("bus[%d], cs[%d], dc[%d], rst[%d]",
          info->bus_num, info->chip_select, data->gpio_dc, data->gpio_reset);
#endif
    master = spi_busnum_to_master(info->bus_num);
    if (IS_ERR_OR_NULL(master))
        return -ENODEV;
    /* check imcompatible device*/
    snprintf(name, sizeof(name), "%s.%u", dev_name(&master->dev), info->chip_select);
    dev = bus_find_device_by_name(&spi_bus_type, NULL, name);
    if (dev) {
        debug("find imcompatible device[%s],deleting", name);
        //device_del(dev);
        spi_unregister_device(container_of(dev, struct spi_device, dev));
    }
    spi = spi_new_device(master, info);
    if (IS_ERR_OR_NULL(spi))
        return -EBUSY;
    return 0;
}

static void unregister_spi_device(struct spi_board_info *info) {
    char name[16];
    struct device *dev;
    struct spi_master *master;
    debug();
    master = spi_busnum_to_master(info->bus_num);
    snprintf(name, sizeof(name), "%s.%u", dev_name(&master->dev), info->chip_select);
    dev = bus_find_device_by_name(&spi_bus_type, NULL, name);
    if (dev) {
        debug("deleting device[%s]", name);
        //device_del(dev);
        spi_unregister_device(container_of(dev, struct spi_device, dev));
    }
}

#endif

/* from device tree */
static const struct of_device_id spi_dt_ids[] = {
        {.compatible = DEVICE_NAME},
        {},
};
MODULE_DEVICE_TABLE(of, spi_dt_ids);

/* from spi_board_info */
static const struct spi_device_id spi_board_ids[] = {
        {DEVICE_NAME, 0},
        {},
};
MODULE_DEVICE_TABLE(spi, spi_board_ids);

struct spi_dev {
    struct display dev;
    struct spi_device *spi;
    spinlock_t spi_lock;
    u32 speed_hz;
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

static struct spi_driver ssd1306_spi_driver = {
        .driver     = {
                .name =  DRIVER_NAME,
                .owner = THIS_MODULE,
                .of_match_table = of_match_ptr(spi_dt_ids),
        },
        .probe      =  spi_driver_probe,
        .remove     =  spi_driver_remove,
        .id_table   = spi_board_ids,
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
    struct spi_dev_info *dev_info = NULL;
    u32 prop_tmp;
    if (spi->dev.of_node && !of_match_device(spi_dt_ids, &spi->dev)) {
        dev_err(&spi->dev, "buggy DT: spidev listed directly in DT\n");
        WARN_ON(spi->dev.of_node && !of_match_device(spi_dt_ids, &spi->dev));
    }
    debug("probe %s[%d.%d]", spi->modalias, spi->master->bus_num, spi->chip_select);
    spidev = kzalloc(sizeof(struct spi_dev), GFP_KERNEL);
    if (IS_ERR_OR_NULL(spidev))
        return -ENOMEM;
    // init spi
    spi->max_speed_hz = SSD_SPI_SPD;
    spi->bits_per_word = SSD_SPI_BPW;
    spi->mode |= SSD_SPI_MODE;
    ret = spi_setup(spi);
    if (ret < 0)
        goto free_dev;
    debug("spi setup");
    // init spi_dev
    spidev->spi = spi;
    spidev->speed_hz = spi->max_speed_hz;
#ifdef REGISTER_SPI_BOARD_INFO
    dev_info = spi->controller_data;
    spidev->dev.gpio.spi_dc = dev_info->gpio_dc;
    spidev->dev.gpio.reset = dev_info->gpio_reset;
#endif
    if (!dev_info) {
        ret = of_property_read_u32(spi->dev.of_node, "display-dc", &prop_tmp);
        if (ret != 0)
            goto free_dev;
        spidev->dev.gpio.spi_dc = prop_tmp;

        ret = of_property_read_u32(spi->dev.of_node, "display-reset", &prop_tmp);
        if (ret != 0)
            goto free_dev;
        spidev->dev.gpio.reset = prop_tmp;
    }
    debug("dc[%d],reset[%d]", spidev->dev.gpio.spi_dc, spidev->dev.gpio.reset);
    // init display
    ret = display_init(&spidev->dev);
    if (ret < 0)
        goto free_dev;
    spin_lock_init(&spidev->spi_lock);
    spidev->dev.interface = &spi_interface;
    ret = display_driver_probe(&spi->dev, &spidev->dev);
    if (ret < 0) {
        debug("display driver init failed");
        goto free_all;
    }
    // save
    spi_set_drvdata(spi, spidev);
    return ret;

    // failure
    free_all:
    display_deinit(&spidev->dev);
    free_dev:
    kfree(spidev);
    return ret;
}

static int spi_driver_remove(struct spi_device *spi) {
    struct spi_dev *spidev;
    debug("remove %s[%d.%d]", spi->modalias, spi->master->bus_num, spi->chip_select);
    spidev = spi_get_drvdata(spi);
    spin_lock_irq(&spidev->spi_lock);
    spidev->spi = NULL;
    spin_unlock_irq(&spidev->spi_lock);
    display_driver_remove(&spidev->dev);
    display_deinit(&spidev->dev);
    kfree(spidev);
    return 0;
}

ssize_t spi_write_cmd(struct display *display, u8 cmd) {
    ssize_t status = 0;
    struct spi_dev *spidev = container_of(display, struct spi_dev, dev);
    struct spi_device *spi = spi_dev_get(spidev->spi);
    if (IS_ERR_OR_NULL(spi))
        return -ENODEV;
    gpio_set_value(display->gpio.spi_dc, 0);
    status = spi_sync_write(spidev, &cmd, 1);
    spi_dev_put(spi);
    return status;
}

ssize_t spi_write_data(struct display *display, u8 data) {
    ssize_t status = 0;
    struct spi_dev *spidev = container_of(display, struct spi_dev, dev);
    struct spi_device *spi = spi_dev_get(spidev->spi);
    if (IS_ERR_OR_NULL(spi))
        return -ENODEV;
    gpio_set_value(display->gpio.spi_dc, 1);
    status = spi_sync_write(spidev, &data, 1);
    spi_dev_put(spi);
    return status;
}

ssize_t spi_write_data_buffer(struct display *display, const u8 *data, size_t size) {
    ssize_t status = 0;
    struct spi_dev *spidev = container_of(display, struct spi_dev, dev);
    struct spi_device *spi = spi_dev_get(spidev->spi);
    if (IS_ERR_OR_NULL(spi))
        return -ENODEV;
    gpio_set_value(display->gpio.spi_dc, 1);
    status = spi_sync_write(spidev, data, size);
    spi_dev_put(spi);
    return status;
}

int display_driver_spi_init(void) {
    int ret = 0;
    int num;
    debug();
    ret = spi_register_driver(&ssd1306_spi_driver);
    if (ret < 0)
        return ret;
#ifdef REGISTER_SPI_BOARD_INFO
    for (num = 0; num < ARRAY_SIZE(ssd1306_spi_board_info); num++) {
        ret = register_spi_device(&ssd1306_spi_board_info[num]);
        if (ret < 0) {
            spi_unregister_driver(&ssd1306_spi_driver);
            break;
        }
    }
#endif
    return ret;
}

void display_driver_spi_exit(void) {
    int num;
    debug();
#ifdef REGISTER_SPI_BOARD_INFO
    for (num = 0; num < ARRAY_SIZE(ssd1306_spi_board_info); num++)
        unregister_spi_device(&ssd1306_spi_board_info[num]);
#endif
    spi_unregister_driver(&ssd1306_spi_driver);
}
