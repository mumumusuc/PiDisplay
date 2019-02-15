//
// Created by mumumusuc on 19-2-9.
//

#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include "ssd1306.h"

// private
static void set_render_mode(struct display *dev, uint8_t mode) {
    dev->interface.write_cmd(dev, CMD_ADDR_MODE);
    dev->interface.write_cmd(dev, mode);
}

static void set_range(struct display *dev, uint8_t start_page, uint8_t end_page, uint8_t start_col, uint8_t end_col) {
    dev->interface.write_cmd(dev, CMD_ADDR_PAGE_RANGE);
    dev->interface.write_cmd(dev, start_page);
    dev->interface.write_cmd(dev, end_page);
    dev->interface.write_cmd(dev, CMD_ADDR_COL_RANGE);
    dev->interface.write_cmd(dev, start_col);
    dev->interface.write_cmd(dev, end_col);
}

/*
static void set_pos(struct display *dev, uint8_t page, uint8_t col) {
    dev->interface.write_cmd(dev, CMD_ADDR_PAGE_START_MASK | page);
    dev->interface.write_cmd(dev, CMD_ADDR_COL_START_LOW_MASK | (col & 0x0F));
    dev->interface.write_cmd(dev, CMD_ADDR_COL_START_HIGH_MASK | ((col & 0xF0) >> 4));
}
*/
static void set_contrast(struct display *dev, uint8_t level) {
    dev->interface.write_cmd(dev, CMD_DISPLAY_CONTRAST);
    dev->interface.write_cmd(dev, level);
}

static void set_voltage(struct display *dev, uint8_t voltage) {
    dev->interface.write_cmd(dev, CMD_POWER_VOLTAGE);
    dev->interface.write_cmd(dev, voltage);

}

static void set_ignore_ram(struct display *dev, uint8_t flag) {
    if (flag) {
        dev->interface.write_cmd(dev, CMD_DISPLAY_ALL);
    } else {
        dev->interface.write_cmd(dev, CMD_DISPLAY_RAM);
    }
}

static void set_reverse(struct display *dev, uint8_t h, uint8_t v) {
    if (v) {
        h = h ? 0 : 1;
        dev->interface.write_cmd(dev, CMD_HARD_SCAN_DIRECT_INVERSE);
    } else {
        dev->interface.write_cmd(dev, CMD_HARD_SCAN_DIRECT_NORMAL);
    }
    if (h) {
        dev->interface.write_cmd(dev, CMD_HARD_MAP_COL_127);
    } else {
        dev->interface.write_cmd(dev, CMD_HARD_MAP_COL_0);
    }
}

static void set_mapping(struct display *dev, uint8_t start_line, uint8_t offset, uint8_t rows) {
    dev->interface.write_cmd(dev, CMD_HARD_START_LINE_MASK | start_line);
    dev->interface.write_cmd(dev, CMD_HARD_VERTICAL_OFFSET);
    dev->interface.write_cmd(dev, offset);
    dev->interface.write_cmd(dev, CMD_HARD_MUX);
    dev->interface.write_cmd(dev, rows - 1);
}

static void set_point_invert(struct display *dev, uint8_t flag) {
    if (flag) {
        dev->interface.write_cmd(dev, CMD_DISPLAY_INVERSE);
    } else {
        dev->interface.write_cmd(dev, CMD_DISPLAY_NORMAL);
    }
}

static void set_frequency(struct display *dev, uint8_t rate, uint8_t div) {
    dev->interface.write_cmd(dev, CMD_TIME_CLOCK);
    dev->interface.write_cmd(dev, ((div - 1) & 0x0F) | ((rate & 0x0F) << 4));
}

static void set_period_pre_charge(struct display *dev, uint8_t phase1, uint8_t phase2) {
    dev->interface.write_cmd(dev, CMD_POWER_PRECHARGE);
    dev->interface.write_cmd(dev, ((phase2 & 0x0F) << 4) | (phase1 & 0x0F));
}

static void set_graphic_zoom(struct display *dev, uint8_t flag) {
    dev->interface.write_cmd(dev, CMD_GRAPHIC_ZOOM);
    flag = flag ? ZOOM_ON : ZOOM_OFF;
    dev->interface.write_cmd(dev, flag);
}

static void set_graphic_fade(struct display *dev, uint8_t mode, uint8_t frame) {
    dev->interface.write_cmd(dev, CMD_GRAPHIC_FADE);
    dev->interface.write_cmd(dev, mode | frame);
}

/*
static void set_graphic_scroll_H(struct display *dev, uint8_t direct, uint8_t start_page, uint8_t end_page, uint8_t frame) {
    dev->interface.write_cmd(dev, direct);
    dev->interface.write_cmd(dev, 0x00);
    dev->interface.write_cmd(dev, start_page);
    dev->interface.write_cmd(dev, frame);
    dev->interface.write_cmd(dev, end_page);
    dev->interface.write_cmd(dev, 0x00);
    dev->interface.write_cmd(dev, 0xFF);
}

static void
set_graphic_scroll_HV(struct display *dev, uint8_t direct, uint8_t start_page, uint8_t end_page, uint8_t frame,
                      uint8_t offset) {
    dev->interface.write_cmd(dev, direct);
    dev->interface.write_cmd(dev, 0x00);
    dev->interface.write_cmd(dev, start_page);
    dev->interface.write_cmd(dev, frame);
    dev->interface.write_cmd(dev, end_page);
    dev->interface.write_cmd(dev, offset);
}

static void set_graphic_scroll_range_V(struct display *dev, uint8_t fix_rows, uint8_t scroll_rows) {
    dev->interface.write_cmd(dev, CMD_SCROLL_DOWN_AREA);
    dev->interface.write_cmd(dev, fix_rows);
    dev->interface.write_cmd(dev, scroll_rows);
}

static void set_graphic_scroll_enable(struct display *dev) {
    dev->interface.write_cmd(dev, CMD_SCROLL_ENABLE);
}
*/
static void set_graphic_scroll_disable(struct display *dev) {
    dev->interface.write_cmd(dev, CMD_SCROLL_DISABLE);
}

static void set_pin_config(struct display *dev, uint8_t alternative, uint8_t remap) {
    uint8_t cmd = 0x02;
    cmd |= alternative ? 0x10 : 0x00;
    cmd |= remap ? 0x20 : 0x00;
    dev->interface.write_cmd(dev, CMD_HARD_COM_CONFIG);
    dev->interface.write_cmd(dev, cmd);
}

static void
update_screen_range(struct display *dev, const uint8_t *data, uint8_t start_page, uint8_t end_page, uint8_t start_col,
                    uint8_t end_col) {
    uint8_t r, c;
    set_range(dev, start_page, end_page, start_col, end_col);
    for (r = start_page; r <= end_page; r++)
        for (c = start_col; c <= end_col; c++) {
            dev->interface.write_data(dev, *(data + SCREEN_COLUMNS * r + c));
        }
}

#define update(dev, data, size, ps, pe, cs, ce)             \
do{                                                         \
    set_render_mode(dev, MODE_HORIZONTAL);                  \
    if(dev->interface.write_data_buffer) {                  \
        set_range(dev, ps, pe, cs, ce);                     \
        dev->interface.write_data_buffer(dev, data, size);  \
    } else                                                  \
        update_screen_range(dev, data, ps, pe, cs, ce);     \
}while(0)

static void update_screen(struct display *dev, const u8 *data, size_t size) {
    u8 ps = dev->dirty_row_start / 8;
    u8 pe = dev->dirty_row_end / 8;
    u8 cs = dev->dirty_col_start;
    u8 ce = dev->dirty_col_end;
    size_t vsize, slsize = ce - cs + 1;
    u8 lsize = slsize;
    int split_flag = 0;
    if (pe > DISPLAY_HEIGHT - 1)
        pe = DISPLAY_HEIGHT - 1;
    if (ce > DISPLAY_WIDTH - 1) {
        ce = DISPLAY_WIDTH - 1;
        lsize = ce - cs + 1;
        split_flag = 1;
    }
    vsize = lsize * (pe - ps + 1);
    if (!vsize)
        return;
    if (size <= 0)
        size = vsize;
    else
        size = min(size, vsize);
    if (size <= 0)
        return;
    /*
     printk(KERN_DEBUG "[%s] page[%u,%u], col[%u,%u], lsize[%d], size[%d], split[%d]\n", __func__, ps, pe, cs, ce, lsize,
           size, split_flag);
    */
    if (split_flag) {
        u8 i = ps;
        for (; i <= pe; i++)
            update(dev, data + slsize * (i - ps), lsize, i, i, cs, ce);
    } else
        update(dev, data, size, ps, pe, cs, ce);
}

// end private

int display_init(struct display *dev, struct interface *interface) {
    int ret = 0;
    char gpio_tmp[16];
    printk(KERN_DEBUG"[%s]\n", __func__);
    if (interface->write_cmd)
        dev->interface.write_cmd = interface->write_cmd;
    if (interface->write_data)
        dev->interface.write_data = interface->write_data;
    if (interface->write_data_buffer)
        dev->interface.write_data_buffer = interface->write_data_buffer;
    dev->gpio_reset = interface->gpio_reset;
    if (dev->gpio_reset <= 0)
        return -EFAULT;
    sprintf(gpio_tmp, "ssd1306_rst_%u", dev->gpio_reset);
    //printk(KERN_DEBUG"[%s] init %s", __func__, gpio_tmp);
    ret = gpio_request_one(dev->gpio_reset, GPIOF_OUT_INIT_HIGH, gpio_tmp);
    if (ret < 0) {
        pr_err("gpio_request(%s)failed with %d\n", gpio_tmp, ret);
        return ret;
    }
    dev->vmem_size = DISPLAY_SIZE;
    if (!dev->vmem) {
        dev->vmem = vzalloc(dev->vmem_size);
        if (!dev->vmem) {
            ret = -ENOMEM;
            goto alloc_failed;
        }
    }
    dev->dirty_col_start = dev->dirty_row_start = 0;
    dev->dirty_col_end = DISPLAY_WIDTH - 1;
    dev->dirty_row_end = DISPLAY_HEIGHT - 1;
    spin_lock_init(&dev->dirty_locker);
    mutex_init(&dev->mem_mutex);
    printk(KERN_DEBUG"[%s] end\n", __func__);
    return ret;

    alloc_failed:
    gpio_free(dev->gpio_reset);
    return ret;
}

void display_deinit(struct display *dev) {
    printk(KERN_DEBUG"[%s]\n", __func__);
    gpio_free(dev->gpio_reset);
    if (dev->vmem) {
        mutex_lock(&dev->mem_mutex);
        vfree(dev->vmem);
        mutex_unlock(&dev->mem_mutex);
    }
    mutex_destroy(&dev->mem_mutex);
}

void display_reset(struct display *dev) {
    printk(KERN_DEBUG"[%s]\n", __func__);
    //gpio_set_value(dev->gpio_reset, 0);
    //udelay(10);
    gpio_set_value(dev->gpio_reset, 1);
    set_reverse(dev, SSD1306_FALSE, SSD1306_TRUE);
    set_mapping(dev, 0, 0, 64);
    set_contrast(dev, 0xFF);
    set_point_invert(dev, SSD1306_FALSE);
    set_ignore_ram(dev, SSD1306_FALSE);
    set_frequency(dev, 8, 1);
    set_period_pre_charge(dev, 2, 2);
    set_pin_config(dev, SSD1306_TRUE, SSD1306_FALSE);
    set_voltage(dev, VOLTAGE_0_DOT_77X);
    set_graphic_scroll_disable(dev);
    set_graphic_fade(dev, FADE_OFF, FADE_FRAME_8);
    set_graphic_zoom(dev, SSD1306_FALSE);
}

void display_turn_on(struct display *dev) {
    printk(KERN_DEBUG"[%s]\n", __func__);
    dev->interface.write_cmd(dev, CMD_POWER_CHARGE_PUMP);
    dev->interface.write_cmd(dev, CHARGE_PUMP_ON);
    dev->interface.write_cmd(dev, CMD_DISPLAY_ON);
}

void display_clear(struct display *dev) {
    if(dev->vmem && dev->vmem_size){
        mutex_lock(&dev->mem_mutex);
        memset(dev->vmem, 0, dev->vmem_size);
        mutex_unlock(&dev->mem_mutex);
        display_update(dev, NULL, 0);
    }
}

void display_update(struct display *dev, const u8 *buffer, size_t size) {
    mutex_lock(&dev->mem_mutex);
    if (buffer)
        update_screen(dev, buffer, size);
    else if (dev->vmem)
        update_screen(dev, dev->vmem, size);
    mutex_unlock(&dev->mem_mutex);
}

void display_turn_off(struct display *dev) {
    dev->interface.write_cmd(dev, CMD_POWER_CHARGE_PUMP);
    dev->interface.write_cmd(dev, CHARGE_PUMP_OFF);
    dev->interface.write_cmd(dev, CMD_DISPLAY_OFF);
}