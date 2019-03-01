#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include "display.h"
#include "ili9341.h"

#define GPIO_RST                            "ili9341_reset"
#define GPIO_LED                            "ili9341_led"
#define gpio(dev, name) ({                              \
    struct disp_gpio *gpio;                             \
    list_for_each_entry(gpio, &(dev)->gpios, list) {    \
    if(0== strcmp((name),gpio->label))                  \
        break;                                          \
    }                                                   \
    gpio;                                               \
})
#define roi(dev)                            (dev)->roi
#define write_cmd(dev, cmd)                 (dev)->interface->write_cmd((dev), (cmd));
#define write_data(dev, data)               (dev)->interface->write_data((dev), (data));
#define write_data_buffer(dev, buf, size)   (dev)->interface->write_data_buffer((dev),(buf),(size));

static struct surface blank = {
        .padding     = 0,
        .data        = NULL,
        .bpp         = DISPLAY_BBP,
        .width       = DISPLAY_WIDTH,
        .height      = DISPLAY_HEIGHT,
        .line_length = DISPLAY_LINE,
};

static struct roi entire = {
        .dirty      = true,
        .col_start  = 0,
        .row_start  = 0,
        .row_offset = 0,
        .col_end    = DISPLAY_WIDTH - 1,
        .row_end    = DISPLAY_HEIGHT - 1,
};

// private

static int display_init_gpios(struct display *dev) {
    int ret;
    struct list_head *gpio_list = &dev->gpios;
    struct disp_gpio *gpio;
    debug();
    list_for_each_entry(gpio, gpio_list, list) {
        ret = gpio->flag;
        if (!gpio->flag) {
            ret = gpio_request_one(gpio->pin, gpio->mode, gpio->label);
            gpio->flag = ret < 0 ? ret : true;
            if (ret < 0 && ret != EPROBE_DEFER) {
                pr_err("init gpio_%u[%s] failed\n", gpio->pin, gpio->label);
                goto deinit;
            }
        }
    }
    return ret;

    deinit:
    list_for_each_entry(gpio, gpio_list, list) {
        if (gpio->flag)
            gpio_free(gpio->pin);
    }
    return ret;
}

static void display_free_gpios(struct display *dev) {
    struct list_head *gpio_list = &dev->gpios;
    struct disp_gpio *gpio, *next;
    debug();
    list_for_each_entry_safe(gpio, next, gpio_list, list) {
        if (gpio->flag)
            gpio_free(gpio->pin);
        list_del(&gpio->list);
    }
}

// end private

int display_init(struct display *dev, unsigned id) {
    int ret = 0;
    debug();
    dev->id = id;
    INIT_LIST_HEAD(&dev->gpios);
    dev->roi = NULL;
    dev->surface = NULL;
    mutex_init(&dev->dev_locker);
    return ret;
}

void display_deinit(struct display *dev) {
    debug();
    display_free_gpios(dev);
    dev->roi = NULL;
    dev->surface = NULL;
    if (blank.data) {
        vfree(blank.data);
        blank.data = NULL;
    }
    mutex_destroy(&dev->dev_locker);
}

void display_reset(struct display *dev) {
    debug();
}

void display_turn_on(struct display *dev) {
    struct disp_gpio *led = gpio(dev, GPIO_LED);
    debug();
    gpio_set_value(led->pin, 1);
}

void display_clear(struct display *dev) {
    debug();
    if (blank.data)
        memset(blank.data, 0, DISPLAY_SIZE);
    else {
        blank.data = vzalloc(DISPLAY_SIZE);
        if (!blank.data) {
            pr_err("[%s] alloc blank.data error(%d)\n", __func__, -ENOMEM);
            return;
        }
    }
    // TODO:
}

void display_update(struct display *dev, struct surface *surface) {
    //mutex_lock(&dev->dev_locker);
    //update_screen_dirty(dev, surface, dev->roi ? dev->roi : &entire);
    //mutex_unlock(&dev->dev_locker);
}

void display_turn_off(struct display *dev) {
    struct disp_gpio *led = gpio(dev, GPIO_LED);
    debug();
    gpio_set_value(led->pin, 0);
}

int display_set_roi(struct display *dev, int xoffset, int yoffset, size_t width, size_t height) {
    size_t col_end, row_end;
    if (!dev->roi)
        return -EINVAL;
    dev->roi->dirty = false;
    if (xoffset > DISPLAY_WIDTH - 1 || yoffset > DISPLAY_HEIGHT - 1)
        return -EFAULT;
    col_end = xoffset + width - 1;
    row_end = yoffset + height - 1;
    if (col_end < 0 || row_end < 0)
        return -EFAULT;
    spin_lock(&roi(dev)->locker);
    roi(dev)->col_start = xoffset < 0 ? 0 : xoffset;
    roi(dev)->row_start = yoffset < 0 ? 0 : yoffset;
    roi(dev)->col_end = col_end > (DISPLAY_WIDTH - 1) ? (DISPLAY_WIDTH - 1) : col_end;
    roi(dev)->row_end = row_end > (DISPLAY_HEIGHT - 1) ? (DISPLAY_HEIGHT - 1) : row_end;
    roi(dev)->row_offset = yoffset % 8;
    spin_unlock(&roi(dev)->locker);
    /*
    debug("col[%u,%u] row[%u,%u] offset[%d]",
          roi(dev)->col_start, roi(dev)->col_end,
          roi(dev)->row_start, roi(dev)->row_end,
          roi(dev)->row_offset);
    */
    dev->roi->dirty = true;
    return 0;
}