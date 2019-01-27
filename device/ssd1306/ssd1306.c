//
// Created by mumumusuc on 19-1-19.
//

#include <assert.h>
#include <string.h>
#include "common.h"
#include "ssd1306_def.h"
#include "ssd1306_priv.h"

#define LOG_TAG "SSD1306"

// private methods
static void set_render_mode(Ssd1306 *self, uint8_t mode) {
    self->ops.write_cmd(self, CMD_ADDR_MODE);
    self->ops.write_cmd(self, mode);
}


static void set_range(Ssd1306 *self, uint8_t start_page, uint8_t end_page, uint8_t start_col, uint8_t end_col) {
    self->ops.write_cmd(self, CMD_ADDR_PAGE_RANGE);
    self->ops.write_cmd(self, start_page);
    self->ops.write_cmd(self, end_page);
    self->ops.write_cmd(self, CMD_ADDR_COL_RANGE);
    self->ops.write_cmd(self, start_col);
    self->ops.write_cmd(self, end_col);

}


static void set_pos(Ssd1306 *self, uint8_t page, uint8_t col) {
    self->ops.write_cmd(self, CMD_ADDR_PAGE_START_MASK | page);
    self->ops.write_cmd(self, CMD_ADDR_COL_START_LOW_MASK | (col & 0x0F));
    self->ops.write_cmd(self, CMD_ADDR_COL_START_HIGH_MASK | ((col & 0xF0) >> 4));
}

static void set_contrast(Ssd1306 *self, uint8_t level) {
    self->ops.write_cmd(self, CMD_DISPLAY_CONTRAST);
    self->ops.write_cmd(self, level);
}

static void set_voltage(Ssd1306 *self, uint8_t voltage) {
    self->ops.write_cmd(self, CMD_POWER_VOLTAGE);
    self->ops.write_cmd(self, voltage);

}

static void set_ignore_ram(Ssd1306 *self, uint8_t flag) {
    if (flag) {
        self->ops.write_cmd(self, CMD_DISPLAY_ALL);
    } else {
        self->ops.write_cmd(self, CMD_DISPLAY_RAM);
    }
}

static void set_reverse(Ssd1306 *self, uint8_t h, uint8_t v) {
    if (v) {
        h = h ? 0 : 1;
        self->ops.write_cmd(self, CMD_HARD_SCAN_DIRECT_INVERSE);
    } else {
        self->ops.write_cmd(self, CMD_HARD_SCAN_DIRECT_NORMAL);
    }
    if (h) {
        self->ops.write_cmd(self, CMD_HARD_MAP_COL_127);
    } else {
        self->ops.write_cmd(self, CMD_HARD_MAP_COL_0);
    }
}

static void set_mapping(Ssd1306 *self, uint8_t start_line, uint8_t offset, uint8_t rows) {
    self->ops.write_cmd(self, CMD_HARD_START_LINE_MASK | start_line);
    self->ops.write_cmd(self, CMD_HARD_VERTICAL_OFFSET);
    self->ops.write_cmd(self, offset);
    self->ops.write_cmd(self, CMD_HARD_MUX);
    self->ops.write_cmd(self, rows - 1);
}

static void set_point_invert(Ssd1306 *self, uint8_t flag) {
    if (flag) {
        self->ops.write_cmd(self, CMD_DISPLAY_INVERSE);
    } else {
        self->ops.write_cmd(self, CMD_DISPLAY_NORMAL);
    }
}

static void set_frequency(Ssd1306 *self, uint8_t rate, uint8_t div) {
    self->ops.write_cmd(self, CMD_TIME_CLOCK);
    self->ops.write_cmd(self, ((div - 1) & 0x0F) | ((rate & 0x0F) << 4));
}

static void set_period_pre_charge(Ssd1306 *self, uint8_t phase1, uint8_t phase2) {
    self->ops.write_cmd(self, CMD_POWER_PRECHARGE);
    self->ops.write_cmd(self, ((phase2 & 0x0F) << 4) | (phase1 & 0x0F));
}

static void set_graphic_zoom(Ssd1306 *self, uint8_t flag) {
    self->ops.write_cmd(self, CMD_GRAPHIC_ZOOM);
    flag = flag ? ZOOM_ON : ZOOM_OFF;
    self->ops.write_cmd(self, flag);
}

static void set_graphic_fade(Ssd1306 *self, uint8_t mode, uint8_t frame) {
    self->ops.write_cmd(self, CMD_GRAPHIC_FADE);
    self->ops.write_cmd(self, mode | frame);
}

static void
set_graphic_scroll_H(Ssd1306 *self, uint8_t direct, uint8_t start_page, uint8_t end_page, uint8_t frame) {
    self->ops.write_cmd(self, direct);
    self->ops.write_cmd(self, 0x00);
    self->ops.write_cmd(self, start_page);
    self->ops.write_cmd(self, frame);
    self->ops.write_cmd(self, end_page);
    self->ops.write_cmd(self, 0x00);
    self->ops.write_cmd(self, 0xFF);
}

static void
set_graphic_scroll_HV(Ssd1306 *self, uint8_t direct, uint8_t start_page, uint8_t end_page, uint8_t frame,
                      uint8_t offset) {
    self->ops.write_cmd(self, direct);
    self->ops.write_cmd(self, 0x00);
    self->ops.write_cmd(self, start_page);
    self->ops.write_cmd(self, frame);
    self->ops.write_cmd(self, end_page);
    self->ops.write_cmd(self, offset);
}

static void set_graphic_scroll_range_V(Ssd1306 *self, uint8_t fix_rows, uint8_t scroll_rows) {
    self->ops.write_cmd(self, CMD_SCROLL_DOWN_AREA);
    self->ops.write_cmd(self, fix_rows);
    self->ops.write_cmd(self, scroll_rows);
}

static void set_graphic_scroll_enable(Ssd1306 *self) {
    self->ops.write_cmd(self, CMD_SCROLL_ENABLE);
}

static void set_graphic_scroll_disable(Ssd1306 *self) {
    self->ops.write_cmd(self, CMD_SCROLL_DISABLE);
}

static void set_pin_config(Ssd1306 *self, uint8_t alternative, uint8_t remap) {
    uint8_t cmd = 0x02;
    cmd |= alternative ? 0x10 : 0x00;
    cmd |= remap ? 0x20 : 0x00;
    self->ops.write_cmd(self, CMD_HARD_COM_CONFIG);
    self->ops.write_cmd(self, cmd);
}

static void
update_screen_range(Ssd1306 *self, uint8_t *data, uint8_t start_page, uint8_t end_page, uint8_t start_col,
                    uint8_t end_col) {
    uint8_t r, c;
    set_render_mode(self, MODE_HORIZONTAL);
    set_range(self, start_page, end_page, start_col, end_col);
    for (r = start_page; r <= end_page; r++)
        for (c = start_col; c <= end_col; c++) {
            self->ops.write_data(self, *(data + SCREEN_COLUMNS * r + c));
        }
}

static void update_screen(Ssd1306 *self, uint8_t *data) {
    update_screen_range(self, data, 0, 7, 0, 127);
}

// implement base methods
static void begin(Display *self) {
    LOG("begin");
    // TODO: init gpio & com.
    Ssd1306 *_self = container_of(self, Ssd1306, base);
    _self->ops.init_com(_self);
    GPIOInfo info = {
            .pin = _self->pin_reset,
            .mode = GPIO_MODE_OUTPUT,
    };
    GPIO gpio = _self->gpio;
    gpio.ops.init(&gpio, &info);
}

static void clear(Display *self) {
    LOG("clear");
    // TODO: clear display.
    Ssd1306 *_self = container_of(self, Ssd1306, base);
    GPIO gpio = _self->gpio;
    gpio.ops.write(&gpio, &(_self->pin_reset), GPIO_LOW);
    delay(16);
    gpio.ops.write(&gpio, &(_self->pin_reset), GPIO_HIGH);
    self->ops.turn_on(self);
};

static void reset(Display *self) {
    LOG("reset");
    // TODO: init/reset display.
    Ssd1306 *_self = container_of(self, Ssd1306, base);
    self->ops.clear(self);
    self->ops.turn_off(self);
    set_reverse(_self, SSD1306_FALSE, SSD1306_TRUE);
    set_mapping(_self, 0, 0, 64);
    set_contrast(_self, 0x7F);
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

static void turn_on(Display *self) {
    LOG("turn_on");
    // TODO: turn on display.
    Ssd1306 *_self = (Ssd1306 *) self;
    _self->ops.write_cmd(_self, CMD_POWER_CHARGE_PUMP);
    _self->ops.write_cmd(_self, CHARGE_PUMP_ON);
    _self->ops.write_cmd(_self, CMD_DISPLAY_ON);
}

static void update(Display *self, void *buffer) {
   // LOG("update");
    // TODO: update display with the buffer.
    if (buffer){
        Ssd1306 *_self = (Ssd1306 *) self;
        update_screen(_self, (uint8_t *) buffer);
    }
}

static void turn_off(Display *self) {
    LOG("turn_off");
    // TODO: turn off display.
    Ssd1306 *_self = (Ssd1306 *) self;
    _self->ops.write_cmd(_self, CMD_POWER_CHARGE_PUMP);
    _self->ops.write_cmd(_self, CHARGE_PUMP_OFF);
    _self->ops.write_cmd(_self, CMD_DISPLAY_OFF);
}

static void end(Display *self) {
    LOG("end");
    // TODO: uninit gpio & com.
    Ssd1306 *_self = (Ssd1306 *) self;
    _self->ops.uninit_com(_self);
}

// protected methods
static void init_com(Ssd1306 *self) {
#ifdef DEBUG
    ERROR("init_com");
#else
    METHOD_NOT_IMPLEMENTED("init_com");
#endif
}

static void uninit_com(Ssd1306 *self) {
#ifdef DEBUG
    ERROR("uninit_com");
#else
    METHOD_NOT_IMPLEMENTED("uninit_com");
#endif
}

static void write_data(Ssd1306 *self, uint8_t data) {
#ifdef DEBUG
    ERROR("write_data");
#else
    METHOD_NOT_IMPLEMENTED("write_data");
#endif
}

static void write_cmd(Ssd1306 *self, uint8_t cmd) {
#ifdef DEBUG
    ERROR("write_cmd");
#else
    METHOD_NOT_IMPLEMENTED("write_cmd");
#endif
}

// constructor & destructor
Ssd1306 *new_Ssd1306(GPIO *gpio, uint8_t rst) {
    assert(gpio);
    Ssd1306 *ssd = (Ssd1306 *) malloc(sizeof(Ssd1306));
    assert(ssd);
    SsdOps ssd_ops = {
            .init_com = init_com,
            .uninit_com = uninit_com,
            .write_cmd = write_cmd,
            .write_data = write_data,
    };
    init_Ssd1306(ssd, gpio, &ssd_ops, rst);
    return ssd;
}

void init_Ssd1306(Ssd1306 *display, GPIO *gpio, SsdOps *ops, uint8_t rst) {
    assert(display && gpio && ops);
    display->pin_reset = rst;
    // init gpio
    init_GPIO(&(display->gpio), gpio);
    // init base members
    DisplayInfo dsp_info = {
            .width = SCREEN_COLUMNS,
            .height = SCREEN_ROWS,
            .pixel_format = 1,
            .vendor = "SSD1306_128_64",
    };
    DisplayOps dsp_ops = {
            .begin = begin,
            .reset = reset,
            .turn_on = turn_on,
            .clear = clear,
            .update = update,
            .turn_off = turn_off,
            .end = end,
    };
    init_Display(&(display->base), &dsp_ops, &dsp_info);
    init_Base(&(display->base), display, del_Ssd1306);
    // init private members
    display->ops.init_com = ops->init_com;
    display->ops.uninit_com = ops->uninit_com;
    display->ops.write_cmd = ops->write_cmd;
    display->ops.write_data = ops->write_data;
}

void del_Ssd1306(Ssd1306 *self) {
    if (self) {
        // del_Display(&(self->base));
    }
    free(self);
    self = NULL;
}
