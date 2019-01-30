//
// Created by mumumusuc on 19-1-25.
//

#include <assert.h>
#include <string.h>
#include <display_protected.h>
#include <gpio.h>
#include "common.h"
#include "display.h"
#include "ssd1306_protected.h"
#include "ssd1306_def.h"

#undef LOG_TAG
#define LOG_TAG "SSD1306"

// default methods
static void _begin_com(SSD1306 *self) {
    DEFAULT_METHOD();
}

static void _end_com(SSD1306 *self) {
    DEFAULT_METHOD();
}

static void _write_data(SSD1306 *self, uint8_t data) {
    DEFAULT_METHOD();
}

static void _write_cmd(SSD1306 *self, uint8_t cmd) {
    DEFAULT_METHOD();
}
// end default methods

// private
static void set_render_mode(SSD1306 *self, uint8_t mode) {
    eval_vtbl(self, write_cmd, CMD_ADDR_MODE);
    eval_vtbl(self, write_cmd, mode);
}

static void set_range(SSD1306 *self, uint8_t start_page, uint8_t end_page, uint8_t start_col, uint8_t end_col) {
    eval_vtbl(self, write_cmd, CMD_ADDR_PAGE_RANGE);
    eval_vtbl(self, write_cmd, start_page);
    eval_vtbl(self, write_cmd, end_page);
    eval_vtbl(self, write_cmd, CMD_ADDR_COL_RANGE);
    eval_vtbl(self, write_cmd, start_col);
    eval_vtbl(self, write_cmd, end_col);

}

static void set_pos(SSD1306 *self, uint8_t page, uint8_t col) {
    eval_vtbl(self, write_cmd, CMD_ADDR_PAGE_START_MASK | page);
    eval_vtbl(self, write_cmd, CMD_ADDR_COL_START_LOW_MASK | (col & 0x0F));
    eval_vtbl(self, write_cmd, CMD_ADDR_COL_START_HIGH_MASK | ((col & 0xF0) >> 4));
}

static void set_contrast(SSD1306 *self, uint8_t level) {
    eval_vtbl(self, write_cmd, CMD_DISPLAY_CONTRAST);
    eval_vtbl(self, write_cmd, level);
}

static void set_voltage(SSD1306 *self, uint8_t voltage) {
    eval_vtbl(self, write_cmd, CMD_POWER_VOLTAGE);
    eval_vtbl(self, write_cmd, voltage);

}

static void set_ignore_ram(SSD1306 *self, uint8_t flag) {
    if (flag) {
        eval_vtbl(self, write_cmd, CMD_DISPLAY_ALL);
    } else {
        eval_vtbl(self, write_cmd, CMD_DISPLAY_RAM);
    }
}

static void set_reverse(SSD1306 *self, uint8_t h, uint8_t v) {
    if (v) {
        h = h ? 0 : 1;
        eval_vtbl(self, write_cmd, CMD_HARD_SCAN_DIRECT_INVERSE);
    } else {
        eval_vtbl(self, write_cmd, CMD_HARD_SCAN_DIRECT_NORMAL);
    }
    if (h) {
        eval_vtbl(self, write_cmd, CMD_HARD_MAP_COL_127);
    } else {
        eval_vtbl(self, write_cmd, CMD_HARD_MAP_COL_0);
    }
}

static void set_mapping(SSD1306 *self, uint8_t start_line, uint8_t offset, uint8_t rows) {
    eval_vtbl(self, write_cmd, CMD_HARD_START_LINE_MASK | start_line);
    eval_vtbl(self, write_cmd, CMD_HARD_VERTICAL_OFFSET);
    eval_vtbl(self, write_cmd, offset);
    eval_vtbl(self, write_cmd, CMD_HARD_MUX);
    eval_vtbl(self, write_cmd, rows - 1);
}

static void set_point_invert(SSD1306 *self, uint8_t flag) {
    if (flag) {
        eval_vtbl(self, write_cmd, CMD_DISPLAY_INVERSE);
    } else {
        eval_vtbl(self, write_cmd, CMD_DISPLAY_NORMAL);
    }
}

static void set_frequency(SSD1306 *self, uint8_t rate, uint8_t div) {
    eval_vtbl(self, write_cmd, CMD_TIME_CLOCK);
    eval_vtbl(self, write_cmd, ((div - 1) & 0x0F) | ((rate & 0x0F) << 4));
}

static void set_period_pre_charge(SSD1306 *self, uint8_t phase1, uint8_t phase2) {
    eval_vtbl(self, write_cmd, CMD_POWER_PRECHARGE);
    eval_vtbl(self, write_cmd, ((phase2 & 0x0F) << 4) | (phase1 & 0x0F));
}

static void set_graphic_zoom(SSD1306 *self, uint8_t flag) {
    eval_vtbl(self, write_cmd, CMD_GRAPHIC_ZOOM);
    flag = flag ? ZOOM_ON : ZOOM_OFF;
    eval_vtbl(self, write_cmd, flag);
}

static void set_graphic_fade(SSD1306 *self, uint8_t mode, uint8_t frame) {
    eval_vtbl(self, write_cmd, CMD_GRAPHIC_FADE);
    eval_vtbl(self, write_cmd, mode | frame);
}

static void
set_graphic_scroll_H(SSD1306 *self, uint8_t direct, uint8_t start_page, uint8_t end_page, uint8_t frame) {
    eval_vtbl(self, write_cmd, direct);
    eval_vtbl(self, write_cmd, 0x00);
    eval_vtbl(self, write_cmd, start_page);
    eval_vtbl(self, write_cmd, frame);
    eval_vtbl(self, write_cmd, end_page);
    eval_vtbl(self, write_cmd, 0x00);
    eval_vtbl(self, write_cmd, 0xFF);
}

static void
set_graphic_scroll_HV(SSD1306 *self, uint8_t direct, uint8_t start_page, uint8_t end_page, uint8_t frame,
                      uint8_t offset) {
    eval_vtbl(self, write_cmd, direct);
    eval_vtbl(self, write_cmd, 0x00);
    eval_vtbl(self, write_cmd, start_page);
    eval_vtbl(self, write_cmd, frame);
    eval_vtbl(self, write_cmd, end_page);
    eval_vtbl(self, write_cmd, offset);
}

static void set_graphic_scroll_range_V(SSD1306 *self, uint8_t fix_rows, uint8_t scroll_rows) {
    eval_vtbl(self, write_cmd, CMD_SCROLL_DOWN_AREA);
    eval_vtbl(self, write_cmd, fix_rows);
    eval_vtbl(self, write_cmd, scroll_rows);
}

static void set_graphic_scroll_enable(SSD1306 *self) {
    eval_vtbl(self, write_cmd, CMD_SCROLL_ENABLE);
}

static void set_graphic_scroll_disable(SSD1306 *self) {
    eval_vtbl(self, write_cmd, CMD_SCROLL_DISABLE);
}

static void set_pin_config(SSD1306 *self, uint8_t alternative, uint8_t remap) {
    uint8_t cmd = 0x02;
    cmd |= alternative ? 0x10 : 0x00;
    cmd |= remap ? 0x20 : 0x00;
    eval_vtbl(self, write_cmd, CMD_HARD_COM_CONFIG);
    eval_vtbl(self, write_cmd, cmd);
}

static void
update_screen_range(SSD1306 *self, uint8_t *data, uint8_t start_page, uint8_t end_page, uint8_t start_col,
                    uint8_t end_col) {
    uint8_t r, c;
    set_render_mode(self, MODE_HORIZONTAL);
    set_range(self, start_page, end_page, start_col, end_col);
    for (r = start_page; r <= end_page; r++)
        for (c = start_col; c <= end_col; c++) {
            eval_vtbl(self, write_data, *(data + SCREEN_COLUMNS * r + c));
        }
}

static void update_screen(SSD1306 *self, uint8_t *data) {
    update_screen_range(self, data, 0, 7, 0, 127);
}
// end private

// override
static DisplayInfo _info = {
        .width = SCREEN_COLUMNS,
        .height = SCREEN_ROWS,
        .pixel_format = 1,
        .vendor="SSD1306_128_64",
};

static void _get_info(Display *self, DisplayInfo *info) {
    if (info) {
        memcpy(info, &_info, sizeof(DisplayInfo));
    }
}

static void _begin(Display *self) {
    LOG("%s", __func__);
    // TODO: init gpio & com.
    SSD1306 *_self = subclass(self, SSD1306);
    gpio_begin(_self->gpio);
    eval_vtbl(_self, begin_com);
    GpioInfo info = {
            .pin = _self->pin_reset,
            .mode = GPIO_MODE_OUTPUT,
    };
    gpio_init(_self->gpio, &info);
}

static void _reset(Display *self) {
    LOG("%s", __func__);
    // TODO: init/reset display.
    SSD1306 *_self = subclass(self, SSD1306);
    gpio_write(_self->gpio, &(_self->pin_reset), GPIO_LOW);
    udelay(10);
    gpio_write(_self->gpio, &(_self->pin_reset), GPIO_HIGH);
    //display_clear(self);
    //display_turn_off(self);
    set_reverse(_self, SSD1306_FALSE, SSD1306_TRUE);
    set_mapping(_self, 0, 0, 64);
    set_contrast(_self, 0xFF);
    set_point_invert(_self, SSD1306_FALSE);
    set_ignore_ram(_self, SSD1306_FALSE);
    set_frequency(_self, 8, 1);
    set_period_pre_charge(_self, 2, 2);
    set_pin_config(_self, SSD1306_TRUE, SSD1306_FALSE);
    set_voltage(_self, VOLTAGE_0_DOT_77X);
    set_graphic_scroll_disable(_self);
    set_graphic_fade(_self, FADE_OFF, FADE_FRAME_8);
    set_graphic_zoom(_self, SSD1306_FALSE);
}

static void _turn_on(Display *self) {
    LOG("%s", __func__);
    // TODO: turn on display.
    SSD1306 *_self = subclass(self, SSD1306);
    eval_vtbl(_self, write_cmd, CMD_POWER_CHARGE_PUMP);
    eval_vtbl(_self, write_cmd, CHARGE_PUMP_ON);
    eval_vtbl(_self, write_cmd, CMD_DISPLAY_ON);
}

static void _clear(Display *self) {
    LOG("%s", __func__);
    // TODO: clear display.
    eval_vtbl(self, reset);
    eval_vtbl(self, turn_on);
}

static void _update(Display *self, const void *buffer) {
    // TODO: update display with the buffer.
    if (buffer) {
        SSD1306 *_self = subclass(self, SSD1306);
        update_screen(_self, (uint8_t *) buffer);
    }
}

static void _turn_off(Display *self) {
    LOG("%s", __func__);
    // TODO: turn off display.
    SSD1306 *_self = subclass(self, SSD1306);
    eval_vtbl(_self, write_cmd, CMD_POWER_CHARGE_PUMP);
    eval_vtbl(_self, write_cmd, CHARGE_PUMP_OFF);
    eval_vtbl(_self, write_cmd, CMD_DISPLAY_OFF);
}


static void _end(Display *self) {
    LOG("%s", __func__);
    // TODO: uninit gpio & com.
    SSD1306 *_self = subclass(self, SSD1306);
    eval_vtbl(_self, end_com);
    gpio_end(_self->gpio);
}
// end override

static DisplayVTbl _vtbl = {
        .get_info = _get_info,
        .begin = _begin,
        .reset = _reset,
        .turn_on = _turn_on,
        .clear = _clear,
        .update = _update,
        .turn_off = _turn_off,
        .end = _end,
};

static SSDVTbl _ssd_vtbl = {
        .begin_com = _begin_com,
        .end_com = _end_com,
        .write_cmd = _write_cmd,
        .write_data = _write_data,
};

inline SSD1306 *new_ssd1306(Gpio *gpio, uint8_t reset) {
    LOG("%s", __func__);
    assert(gpio);
    SSD1306 *ssd = (SSD1306 *) malloc(sizeof(SSD1306));
    assert(ssd);
    //memcpy(ssd->gpio, gpio, sizeof(Gpio));
    ssd->gpio = gpio;
    ssd->pin_reset = reset;
    override(ssd, &_ssd_vtbl);
    Display *super = new_display();
    override(super, &_vtbl);
    link2(ssd, super, "SSD1306", del_ssd1306);
    return ssd;
}

inline void del_ssd1306(void *self) {
    LOG("%s", __func__);
    if (self) {
        SSD1306 *_self = (SSD1306 *) self;
        object_delete(_self->obj);
        delete(_self->gpio->obj);
    }
    free(self);
    self = NULL;
}