//
// Created by mumumusuc on 19-2-9.
//

#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include "ssd1306_dev.h"

// private
static void set_render_mode(display_t *dev, uint8_t mode) {
    dev->write_cmd(dev, CMD_ADDR_MODE);
    dev->write_cmd(dev, mode);
}

static void set_range(display_t *dev, uint8_t start_page, uint8_t end_page, uint8_t start_col, uint8_t end_col) {
    dev->write_cmd(dev, CMD_ADDR_PAGE_RANGE);
    dev->write_cmd(dev, start_page);
    dev->write_cmd(dev, end_page);
    dev->write_cmd(dev, CMD_ADDR_COL_RANGE);
    dev->write_cmd(dev, start_col);
    dev->write_cmd(dev, end_col);
}

/*
static void set_pos(display_t *dev, uint8_t page, uint8_t col) {
    dev->write_cmd(dev, CMD_ADDR_PAGE_START_MASK | page);
    dev->write_cmd(dev, CMD_ADDR_COL_START_LOW_MASK | (col & 0x0F));
    dev->write_cmd(dev, CMD_ADDR_COL_START_HIGH_MASK | ((col & 0xF0) >> 4));
}
*/
static void set_contrast(display_t *dev, uint8_t level) {
    dev->write_cmd(dev, CMD_DISPLAY_CONTRAST);
    dev->write_cmd(dev, level);
}

static void set_voltage(display_t *dev, uint8_t voltage) {
    dev->write_cmd(dev, CMD_POWER_VOLTAGE);
    dev->write_cmd(dev, voltage);

}

static void set_ignore_ram(display_t *dev, uint8_t flag) {
    if (flag) {
        dev->write_cmd(dev, CMD_DISPLAY_ALL);
    } else {
        dev->write_cmd(dev, CMD_DISPLAY_RAM);
    }
}

static void set_reverse(display_t *dev, uint8_t h, uint8_t v) {
    if (v) {
        h = h ? 0 : 1;
        dev->write_cmd(dev, CMD_HARD_SCAN_DIRECT_INVERSE);
    } else {
        dev->write_cmd(dev, CMD_HARD_SCAN_DIRECT_NORMAL);
    }
    if (h) {
        dev->write_cmd(dev, CMD_HARD_MAP_COL_127);
    } else {
        dev->write_cmd(dev, CMD_HARD_MAP_COL_0);
    }
}

static void set_mapping(display_t *dev, uint8_t start_line, uint8_t offset, uint8_t rows) {
    dev->write_cmd(dev, CMD_HARD_START_LINE_MASK | start_line);
    dev->write_cmd(dev, CMD_HARD_VERTICAL_OFFSET);
    dev->write_cmd(dev, offset);
    dev->write_cmd(dev, CMD_HARD_MUX);
    dev->write_cmd(dev, rows - 1);
}

static void set_point_invert(display_t *dev, uint8_t flag) {
    if (flag) {
        dev->write_cmd(dev, CMD_DISPLAY_INVERSE);
    } else {
        dev->write_cmd(dev, CMD_DISPLAY_NORMAL);
    }
}

static void set_frequency(display_t *dev, uint8_t rate, uint8_t div) {
    dev->write_cmd(dev, CMD_TIME_CLOCK);
    dev->write_cmd(dev, ((div - 1) & 0x0F) | ((rate & 0x0F) << 4));
}

static void set_period_pre_charge(display_t *dev, uint8_t phase1, uint8_t phase2) {
    dev->write_cmd(dev, CMD_POWER_PRECHARGE);
    dev->write_cmd(dev, ((phase2 & 0x0F) << 4) | (phase1 & 0x0F));
}

static void set_graphic_zoom(display_t *dev, uint8_t flag) {
    dev->write_cmd(dev, CMD_GRAPHIC_ZOOM);
    flag = flag ? ZOOM_ON : ZOOM_OFF;
    dev->write_cmd(dev, flag);
}

static void set_graphic_fade(display_t *dev, uint8_t mode, uint8_t frame) {
    dev->write_cmd(dev, CMD_GRAPHIC_FADE);
    dev->write_cmd(dev, mode | frame);
}

/*
static void set_graphic_scroll_H(display_t *dev, uint8_t direct, uint8_t start_page, uint8_t end_page, uint8_t frame) {
    dev->write_cmd(dev, direct);
    dev->write_cmd(dev, 0x00);
    dev->write_cmd(dev, start_page);
    dev->write_cmd(dev, frame);
    dev->write_cmd(dev, end_page);
    dev->write_cmd(dev, 0x00);
    dev->write_cmd(dev, 0xFF);
}

static void
set_graphic_scroll_HV(display_t *dev, uint8_t direct, uint8_t start_page, uint8_t end_page, uint8_t frame,
                      uint8_t offset) {
    dev->write_cmd(dev, direct);
    dev->write_cmd(dev, 0x00);
    dev->write_cmd(dev, start_page);
    dev->write_cmd(dev, frame);
    dev->write_cmd(dev, end_page);
    dev->write_cmd(dev, offset);
}

static void set_graphic_scroll_range_V(display_t *dev, uint8_t fix_rows, uint8_t scroll_rows) {
    dev->write_cmd(dev, CMD_SCROLL_DOWN_AREA);
    dev->write_cmd(dev, fix_rows);
    dev->write_cmd(dev, scroll_rows);
}

static void set_graphic_scroll_enable(display_t *dev) {
    dev->write_cmd(dev, CMD_SCROLL_ENABLE);
}
*/
static void set_graphic_scroll_disable(display_t *dev) {
    dev->write_cmd(dev, CMD_SCROLL_DISABLE);
}

static void set_pin_config(display_t *dev, uint8_t alternative, uint8_t remap) {
    uint8_t cmd = 0x02;
    cmd |= alternative ? 0x10 : 0x00;
    cmd |= remap ? 0x20 : 0x00;
    dev->write_cmd(dev, CMD_HARD_COM_CONFIG);
    dev->write_cmd(dev, cmd);
}

static void
update_screen_range(display_t *dev, const uint8_t *data, uint8_t start_page, uint8_t end_page, uint8_t start_col,
                    uint8_t end_col) {
    uint8_t r, c;
    set_range(dev, start_page, end_page, start_col, end_col);
    for (r = start_page; r <= end_page; r++)
        for (c = start_col; c <= end_col; c++) {
            dev->write_data(dev, *(data + SCREEN_COLUMNS * r + c));
        }
}

static void update_screen(display_t *dev, const uint8_t *data, size_t size) {
    set_render_mode(dev, MODE_HORIZONTAL);
    if (dev->write_data_buffer) {
        set_range(dev, 0, 7, 0, 127);
        dev->write_data_buffer(dev, data, size);
        return;
    } else {
        update_screen_range(dev, data, 0, 7, 0, 127);
    }
}

// end private
int display_init(display_t *dev) {
    gpio_request_one(dev->gpio_reset, GPIOF_OUT_INIT_LOW, "ssd1306_reset");
    if (!dev->screen) {
        dev->screen = kmalloc(1024, GFP_KERNEL);
        if (!dev->screen) {
            return -ENOMEM;
        }
    }
    return 0;
}

void display_deinit(display_t *dev) {
    gpio_free(dev->gpio_reset);
    kfree(dev->screen);
}

void display_reset(display_t *dev) {
    gpio_set_value(dev->gpio_reset, GPIO_LO);
    udelay(10);
    gpio_set_value(dev->gpio_reset, GPIO_HI);
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

void display_turn_on(display_t *dev) {
    dev->write_cmd(dev, CMD_POWER_CHARGE_PUMP);
    dev->write_cmd(dev, CHARGE_PUMP_ON);
    dev->write_cmd(dev, CMD_DISPLAY_ON);
}

void display_clear(display_t *dev) {
    display_reset(dev);
    display_turn_on(dev);
}

void display_update(display_t *dev, size_t size) {
    update_screen(dev, dev->screen, size);
}

void display_turn_off(display_t *dev) {
    dev->write_cmd(dev, CMD_POWER_CHARGE_PUMP);
    dev->write_cmd(dev, CHARGE_PUMP_OFF);
    dev->write_cmd(dev, CMD_DISPLAY_OFF);
}