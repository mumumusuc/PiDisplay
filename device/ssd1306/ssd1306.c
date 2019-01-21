//
// Created by mumumusuc on 19-1-19.
//

#include <assert.h>
#include <string.h>
#include "common.h"
#include "ssd1306_def.h"
#include "ssd1306_priv.h"

// private methods

static void set_render_mode(Ssd1306Display *self, uint8_t mode) {
    self->write_cmd(self, CMD_ADDR_MODE);
    self->write_cmd(self, mode);
}


static void set_range(Ssd1306Display *self, uint8_t start_page, uint8_t end_page, uint8_t start_col, uint8_t end_col) {
    self->write_cmd(self, CMD_ADDR_PAGE_RANGE);
    self->write_cmd(self, start_page);
    self->write_cmd(self, end_page);
    self->write_cmd(self, CMD_ADDR_COL_RANGE);
    self->write_cmd(self, start_col);
    self->write_cmd(self, end_col);

}


static void set_pos(Ssd1306Display *self, uint8_t page, uint8_t col) {
    self->write_cmd(self, CMD_ADDR_PAGE_START_MASK | page);
    self->write_cmd(self, CMD_ADDR_COL_START_LOW_MASK | (col & 0x0F));
    self->write_cmd(self, CMD_ADDR_COL_START_HIGH_MASK | ((col & 0xF0) >> 4));
}

static void set_contrast(Ssd1306Display *self, uint8_t level) {
    self->write_cmd(self, CMD_DISPLAY_CONTRAST);
    self->write_cmd(self, level);
}

static void set_voltage(Ssd1306Display *self, uint8_t voltage) {
    self->write_cmd(self, CMD_POWER_VOLTAGE);
    self->write_cmd(self, voltage);

}

static void set_ignore_ram(Ssd1306Display *self, uint8_t flag) {
    if (flag) {
        self->write_cmd(self, CMD_DISPLAY_ALL);
    } else {
        self->write_cmd(self, CMD_DISPLAY_RAM);
    }
}

static void set_reverse(Ssd1306Display *self, uint8_t h, uint8_t v) {
    if (v) {
        h = h ? 0 : 1;
        self->write_cmd(self, CMD_HARD_SCAN_DIRECT_INVERSE);
    } else {
        self->write_cmd(self, CMD_HARD_SCAN_DIRECT_NORMAL);
    }
    if (h) {
        self->write_cmd(self, CMD_HARD_MAP_COL_127);
    } else {
        self->write_cmd(self, CMD_HARD_MAP_COL_0);
    }
}

static void set_mapping(Ssd1306Display *self, uint8_t start_line, uint8_t offset, uint8_t rows) {
    self->write_cmd(self, CMD_HARD_START_LINE_MASK | start_line);
    self->write_cmd(self, CMD_HARD_VERTICAL_OFFSET);
    self->write_cmd(self, offset);
    self->write_cmd(self, CMD_HARD_MUX);
    self->write_cmd(self, rows - 1);
}

static void set_point_invert(Ssd1306Display *self, uint8_t flag) {
    if (flag) {
        self->write_cmd(self, CMD_DISPLAY_INVERSE);
    } else {
        self->write_cmd(self, CMD_DISPLAY_NORMAL);
    }
}

static void set_frequency(Ssd1306Display *self, uint8_t rate, uint8_t div) {
    self->write_cmd(self, CMD_TIME_CLOCK);
    self->write_cmd(self, ((div - 1) & 0x0F) | ((rate & 0x0F) << 4));
}

static void set_period_pre_charge(Ssd1306Display *self, uint8_t phase1, uint8_t phase2) {
    self->write_cmd(self, CMD_POWER_PRECHARGE);
    self->write_cmd(self, ((phase2 & 0x0F) << 4) | (phase1 & 0x0F));
}

static void set_graphic_zoom(Ssd1306Display *self, uint8_t flag) {
    self->write_cmd(self, CMD_GRAPHIC_ZOOM);
    flag = flag ? ZOOM_ON : ZOOM_OFF;
    self->write_cmd(self, flag);
}

static void set_graphic_fade(Ssd1306Display *self, uint8_t mode, uint8_t frame) {
    self->write_cmd(self, CMD_GRAPHIC_FADE);
    self->write_cmd(self, mode | frame);
}

static void
set_graphic_scroll_H(Ssd1306Display *self, uint8_t direct, uint8_t start_page, uint8_t end_page, uint8_t frame) {
    self->write_cmd(self, direct);
    self->write_cmd(self, 0x00);
    self->write_cmd(self, start_page);
    self->write_cmd(self, frame);
    self->write_cmd(self, end_page);
    self->write_cmd(self, 0x00);
    self->write_cmd(self, 0xFF);
}

static void
set_graphic_scroll_HV(Ssd1306Display *self, uint8_t direct, uint8_t start_page, uint8_t end_page, uint8_t frame,
                      uint8_t offset) {
    self->write_cmd(self, direct);
    self->write_cmd(self, 0x00);
    self->write_cmd(self, start_page);
    self->write_cmd(self, frame);
    self->write_cmd(self, end_page);
    self->write_cmd(self, offset);
}

static void set_graphic_scroll_range_V(Ssd1306Display *self, uint8_t fix_rows, uint8_t scroll_rows) {
    self->write_cmd(self, CMD_SCROLL_DOWN_AREA);
    self->write_cmd(self, fix_rows);
    self->write_cmd(self, scroll_rows);
}

static void set_graphic_scroll_enable(Ssd1306Display *self) {
    self->write_cmd(self, CMD_SCROLL_ENABLE);
}

static void set_graphic_scroll_disable(Ssd1306Display *self) {
    self->write_cmd(self, CMD_SCROLL_DISABLE);
}

static void set_pin_config(Ssd1306Display *self, uint8_t alternative, uint8_t remap) {
    uint8_t cmd = 0x02;
    cmd |= alternative ? 0x10 : 0x00;
    cmd |= remap ? 0x20 : 0x00;
    self->write_cmd(self, CMD_HARD_COM_CONFIG);
    self->write_cmd(self, cmd);
}

static void
update_screen_range(Ssd1306Display *self, uint8_t *data, uint8_t start_page, uint8_t end_page, uint8_t start_col,
                    uint8_t end_col) {
    uint8_t r, c;
    set_render_mode(self, MODE_HORIZONTAL);
    set_range(self, start_page, end_page, start_col, end_col);
    for (r = start_page; r <= end_page; r++)
        for (c = start_col; c <= end_col; c++) {
            self->write_data(self, *(data + SCREEN_COLUMNS * r + c));
        }
}

static void update_screen(Ssd1306Display *self, uint8_t *data) {
    update_screen_range(self, data, 0, 7, 0, 127);
}

static void clean(Ssd1306Display *self) {
    LOG("clean");
    /*
    _driver->gpio->write(HW_RST_PIN_1, GPIO_LOW);
    _driver->gpio->write(HW_RST_PIN_2, GPIO_LOW);
    DELAY(10);
    _driver->gpio->write(HW_RST_PIN_1, GPIO_HIGH);
    _driver->gpio->write(HW_RST_PIN_2, GPIO_HIGH);
     */
};

// implement methods

static void init(void *self) {
    LOG("init");
    /*
    assert(_driver->gpio);
    _gpio_info.mode = GPIO_MODE_OUTPUT;
    _gpio_info.pin = HW_RST_PIN_1;
    _driver->gpio->init(_gpio_info);
    _gpio_info.pin = HW_RST_PIN_2;
    _driver->gpio->init(_gpio_info);
    init_com();
     */
}

static void reset(void *self) {
    LOG("reset");
    Ssd1306Display *_self = (Ssd1306Display *) self;
    //_self->turn_off(self);
    clean(self);
    set_reverse(self, SSD1306_FALSE, SSD1306_TRUE);
    set_mapping(self, 0, 0, 64);
    set_contrast(self, 127);
    set_point_invert(self, SSD1306_FALSE);
    set_ignore_ram(self, SSD1306_FALSE);
    set_frequency(self, 8, 1);
    set_period_pre_charge(self, 2, 2);
    set_pin_config(self, SSD1306_TRUE, SSD1306_FALSE);
    set_voltage(self, VOLTAGE_0_DOT_77X);
    set_graphic_scroll_disable(self);
    set_graphic_fade(self, FADE_OFF, FADE_FRAME_8);
    set_graphic_zoom(self, SSD1306_FALSE);
}

static void turn_on(void *self) {
    LOG("turn_on");
    Ssd1306Display *_self = (Ssd1306Display *) self;
    _self->write_cmd(self, CMD_POWER_CHARGE_PUMP);
    _self->write_cmd(self, CHARGE_PUMP_ON);
    _self->write_cmd(self, CMD_DISPLAY_ON);
}

static void turn_off(void *self) {
    LOG("turn_off");
    Ssd1306Display *_self = (Ssd1306Display *) self;
    _self->write_cmd(self, CMD_POWER_CHARGE_PUMP);
    _self->write_cmd(self, CHARGE_PUMP_OFF);
    _self->write_cmd(self, CMD_DISPLAY_OFF);
}

static void update(void *self, void *buffer) {
    LOG("update");
    assert(buffer);
    Ssd1306Display *_self = (Ssd1306Display *) self;
    update_screen(_self, buffer);
}

static void write_data(Ssd1306Display *self, uint8_t data) {
    METHOD_NOT_IMPLEMENTED("write_data");
}

static void write_cmd(Ssd1306Display *self, uint8_t cmd) {
    METHOD_NOT_IMPLEMENTED("write_cmd");
}

// constructor & destructor
Ssd1306Display *new_Ssd1306Display(GPIO *gpio) {
    assert(gpio);
    Ssd1306Display *display = (Ssd1306Display *) malloc(sizeof(Ssd1306Display));
    assert(display);
    display->gpio = new_Driver();
    free(display->gpio);
    display->gpio->init = gpio->init;
    display->gpio->uninit = gpio->uninit;
    display->gpio->write = gpio->write;
    display->gpio->read = gpio->read;
    DisplayOps ops = {
            .init = init,
            .reset = reset,
            .turn_on = turn_on,
            .turn_off = turn_off,
            .update = update,
    };
    display->base = new_Display();
    init_Display(display->base, &ops);
    display->write_cmd = write_cmd;
    display->write_data = write_data;
}

void delete_Ssd1306Display(Ssd1306Display *self) {
    if (self) {
        delete_Display(self->base);
        self->base = NULL;
    }
    free(self);
    self = NULL;
}
