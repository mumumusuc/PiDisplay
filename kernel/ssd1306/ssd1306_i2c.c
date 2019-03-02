
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/acpi.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include "ssd1306_dev.h"
#include "display.h"

#define REGISTER_I2C_BOARD_INFO
#define I2C_BUS_NUM     1
#define DRIVER_NAME     "ssd1306"
#define DEVICE_NAME     "ssd1306"
#define DEVICE_ADDR1    SSD1306_I2C_ADDR
#define DEVICE_ADDR2    (SSD1306_I2C_ADDR+1)
#define I2C_DATA_REG    0x40
#define I2C_CMD_REG     0x00

struct i2c_dev {
    struct display display;
    struct i2c_client *client;
    u8 buf[2];
    struct spinlock buf_lock;
};

struct i2c_data {
    u8 gpio_reset;
};

// display interface
ssize_t i2c_write_cmd(struct display *, u8);

ssize_t i2c_write_data(struct display *, u8);

//ssize_t i2c_write_data_buffer(struct display *, const u8 *, size_t);

static struct interface i2c_interface = {
        .write_cmd          = i2c_write_cmd,
        .write_data         = i2c_write_data,
        .write_data_buffer  = NULL,
};

#ifdef REGISTER_I2C_BOARD_INFO

static struct i2c_board_info ssd1306_i2c_board_info[] = {
        {
                I2C_BOARD_INFO(DEVICE_NAME, DEVICE_ADDR1),
                .platform_data =&(struct i2c_data) {
                        .gpio_reset = 4,
                }
        },
        /*
        {
                I2C_BOARD_INFO(DEVICE_NAME, DEVICE_ADDR2),
        },*/
};

static int register_i2c_device(u8 id, struct i2c_board_info *info) {
    int ret = 0;
    char name[8];
    struct device *dev;
    struct i2c_adapter *adapter;
    struct i2c_client *client;
    debug();
    adapter = i2c_get_adapter(I2C_BUS_NUM);
    if (!adapter) {
        pr_err("get adapter i2c@%d failed\n", I2C_BUS_NUM);
        return -ENODEV;
    }
    snprintf(name, sizeof(name), "%d-%04x", I2C_BUS_NUM, info->addr);
    dev = bus_find_device_by_name(&i2c_bus_type, NULL, name);
    if (dev) {
        debug("%s found,deleting...", name);
        //device_del(dev);
        i2c_unregister_device(container_of(dev, struct i2c_client, dev));
    }
    client = i2c_new_device(adapter, info);
    if (!client) {
        pr_err("register %s failed\n", name);
        ret = -EBUSY;
        goto done;
    }
    return ret;

    done:
    i2c_put_adapter(adapter);
    return ret;
}

static void unregister_i2c_device(u8 id, struct i2c_board_info *info) {
    //struct i2c_adapter *adapter;
    //struct i2c_client *client;
    struct device *dev;
    char name[8];
    //adapter = i2c_get_adapter(I2C_BUS_NUM);
    snprintf(name, sizeof(name), "%d-%04x", I2C_BUS_NUM, info->addr);
    debug("device %s", name);
    dev = bus_find_device_by_name(&i2c_bus_type, NULL, name);
    i2c_unregister_device(container_of(dev, struct i2c_client, dev));
}

#endif

/* from device tree */
static const struct of_device_id i2c_dt_ids[] = {
        {.compatible = DEVICE_NAME},
        {},
};
MODULE_DEVICE_TABLE(of, i2c_dt_ids);

/* from i2c board_info */
static const struct i2c_device_id i2c_board_ids[] = {
        {DEVICE_NAME, 0},
        {},
};
MODULE_DEVICE_TABLE(i2c, i2c_board_ids);

static int i2c_driver_probe(struct i2c_client *, const struct i2c_device_id *);

static int i2c_driver_remove(struct i2c_client *);

static struct i2c_driver ssd1306_i2c_driver = {
        .driver     = {
                .owner          = THIS_MODULE,
                .name           = DEVICE_NAME,
                .of_match_table = i2c_dt_ids,
        },
        .probe      = i2c_driver_probe,
        .remove     = i2c_driver_remove,
        .id_table   = i2c_board_ids,
};

ssize_t i2c_write_cmd(struct display *display, u8 cmd) {
    int ret;
    struct i2c_dev *i2cdev = container_of(display, struct i2c_dev, display);
    struct i2c_client *client = i2cdev->client;
    spin_lock(&i2cdev->buf_lock);
    i2cdev->buf[0] = I2C_CMD_REG;
    i2cdev->buf[1] = cmd;
    spin_unlock(&i2cdev->buf_lock);
    ret = i2c_master_send(client, i2cdev->buf, 2);
    //ret = i2c_transfer(client->adapter, , 1);
    return ret;
}

ssize_t i2c_write_data(struct display *display, u8 data) {
    int ret;
    struct i2c_dev *i2cdev = container_of(display, struct i2c_dev, display);
    struct i2c_client *client = i2cdev->client;
    spin_lock(&i2cdev->buf_lock);
    i2cdev->buf[0] = I2C_DATA_REG;
    i2cdev->buf[1] = data;
    spin_unlock(&i2cdev->buf_lock);
    ret = i2c_master_send(client, i2cdev->buf, 2);
    //ret = i2c_transfer(client->adapter, , 1);
    return ret;
}

static int i2c_driver_probe(struct i2c_client *client, const struct i2c_device_id *device_id) {
    int ret;
    u32 prop_tmp;
    struct i2c_dev *i2cdev;
    struct i2c_data *i2cdata;
    debug("address[%02X], i2c_device_id[%s]", client->addr, device_id->name);

    i2cdev = kzalloc(sizeof(struct i2c_dev), GFP_KERNEL);
    if (!i2cdev)
        return -ENOMEM;
#ifdef REGISTER_I2C_BOARD_INFO
    i2cdata = client->dev.platform_data;
#endif
    if (i2cdata)
        i2cdev->display.gpio.reset = i2cdata->gpio_reset;
    else
        of_property_read_u32(client->dev.of_node, "display-reset", &prop_tmp);
    debug("reset[%d]", i2cdev->display.gpio.reset);
    i2cdev->client = client;
    ret = display_init(&i2cdev->display);
    if (ret < 0)
        goto free_dev;
    i2cdev->display.interface = &i2c_interface;
    ret = display_driver_probe(&client->dev, &i2cdev->display);
    if (ret < 0)
        goto deinit;
    spin_lock_init(&i2cdev->buf_lock);
    i2c_set_clientdata(client, i2cdev);
    return ret;

    deinit:
    display_deinit(&i2cdev->display);
    free_dev:
    kfree(i2cdev);
    return ret;
}

static int i2c_driver_remove(struct i2c_client *client) {
    struct i2c_dev *i2cdev;
    debug("address[%02X]", client->addr);

    i2cdev = i2c_get_clientdata(client);
    display_driver_remove(&i2cdev->display);
    display_deinit(&i2cdev->display);
    kfree(i2cdev);
    return 0;
}

int display_driver_i2c_init(void) {
    int ret, num;
    debug();
    ret = i2c_add_driver(&ssd1306_i2c_driver);
    if (ret < 0)
        return ret;
#ifdef REGISTER_I2C_BOARD_INFO
    for (num = 0; num < ARRAY_SIZE(ssd1306_i2c_board_info); num++) {
        ret = register_i2c_device(num, &ssd1306_i2c_board_info[num]);
        if (ret < 0) {
            i2c_del_driver(&ssd1306_i2c_driver);
            break;
        }
    }
#endif
    return ret;
}

void display_driver_i2c_exit(void) {
    int num;
    debug();
#ifdef REGISTER_I2C_BOARD_INFO
    for (num = 0; num < ARRAY_SIZE(ssd1306_i2c_board_info); num++)
        unregister_i2c_device(num, &ssd1306_i2c_board_info[num]);
#endif
    i2c_del_driver(&ssd1306_i2c_driver);
}