#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include "ssd1306_dev.h"
#include "display.h"

#define TRESHOLD                            0
#define roi(dev)                            (dev)->roi
#define write_cmd(dev, cmd)                 (dev)->interface->write_cmd((dev), (cmd));
#define write_data(dev, data)               (dev)->interface->write_data((dev), (data));
#define write_data_buffer(dev, buf, size)   (dev)->interface->write_data_buffer((dev),(buf),(size));

static struct surface blank = {
        .depth       = 1,
        .data        = NULL,
        .width       = DISPLAY_WIDTH,
        .height      = DISPLAY_HEIGHT,
        .line_length = DISPLAY_LINE,
};

static struct roi entire = {
        .dirty      = true,
        .padding    = 0,
        .col_start  = 0,
        .row_start  = 0,
        .col_end    = DISPLAY_WIDTH - 1,
        .row_end    = DISPLAY_HEIGHT - 1,
        .row_offset = 0,
};

// private
static void set_render_mode(struct display *dev, uint8_t mode) {
    write_cmd(dev, CMD_ADDR_MODE);
    write_cmd(dev, mode);
}

static void set_range(struct display *dev, uint8_t start_page, uint8_t end_page, uint8_t start_col, uint8_t end_col) {
    write_cmd(dev, CMD_ADDR_PAGE_RANGE);
    write_cmd(dev, start_page);
    write_cmd(dev, end_page);
    write_cmd(dev, CMD_ADDR_COL_RANGE);
    write_cmd(dev, start_col);
    write_cmd(dev, end_col);
}

/*
static void set_pos(struct display *dev, uint8_t page, uint8_t col) {
    write_cmd(dev, CMD_ADDR_PAGE_START_MASK | page);
    write_cmd(dev, CMD_ADDR_COL_START_LOW_MASK | (col & 0x0F));
    write_cmd(dev, CMD_ADDR_COL_START_HIGH_MASK | ((col & 0xF0) >> 4));
}
*/
static void set_contrast(struct display *dev, uint8_t level) {
    write_cmd(dev, CMD_DISPLAY_CONTRAST);
    write_cmd(dev, level);
}

static void set_voltage(struct display *dev, uint8_t voltage) {
    write_cmd(dev, CMD_POWER_VOLTAGE);
    write_cmd(dev, voltage);

}

static void set_ignore_ram(struct display *dev, uint8_t flag) {
    if (flag) {
        write_cmd(dev, CMD_DISPLAY_ALL);
    } else {
        write_cmd(dev, CMD_DISPLAY_RAM);
    }
}

static void set_reverse(struct display *dev, uint8_t h, uint8_t v) {
    if (v) {
        h = h ? 0 : 1;
        write_cmd(dev, CMD_HARD_SCAN_DIRECT_INVERSE);
    } else {
        write_cmd(dev, CMD_HARD_SCAN_DIRECT_NORMAL);
    }
    if (h) {
        write_cmd(dev, CMD_HARD_MAP_COL_127);
    } else {
        write_cmd(dev, CMD_HARD_MAP_COL_0);
    }
}

static void set_mapping(struct display *dev, uint8_t start_line, uint8_t offset, uint8_t rows) {
    write_cmd(dev, CMD_HARD_START_LINE_MASK | start_line);
    write_cmd(dev, CMD_HARD_VERTICAL_OFFSET);
    write_cmd(dev, offset);
    write_cmd(dev, CMD_HARD_MUX);
    write_cmd(dev, rows - 1);
}

static void set_point_invert(struct display *dev, uint8_t flag) {
    if (flag) {
        write_cmd(dev, CMD_DISPLAY_INVERSE);
    } else {
        write_cmd(dev, CMD_DISPLAY_NORMAL);
    }
}

static void set_frequency(struct display *dev, uint8_t rate, uint8_t div) {
    write_cmd(dev, CMD_TIME_CLOCK);
    write_cmd(dev, ((div - 1) & 0x0F) | ((rate & 0x0F) << 4));
}

static void set_period_pre_charge(struct display *dev, uint8_t phase1, uint8_t phase2) {
    write_cmd(dev, CMD_POWER_PRECHARGE);
    write_cmd(dev, ((phase2 & 0x0F) << 4) | (phase1 & 0x0F));
}

static void set_graphic_zoom(struct display *dev, uint8_t flag) {
    write_cmd(dev, CMD_GRAPHIC_ZOOM);
    flag = flag ? ZOOM_ON : ZOOM_OFF;
    write_cmd(dev, flag);
}

static void set_graphic_fade(struct display *dev, uint8_t mode, uint8_t frame) {
    write_cmd(dev, CMD_GRAPHIC_FADE);
    write_cmd(dev, mode | frame);
}

/*
static void set_graphic_scroll_H(struct display *dev, uint8_t direct, uint8_t start_page, uint8_t end_page, uint8_t frame) {
    write_cmd(dev, direct);
    write_cmd(dev, 0x00);
    write_cmd(dev, start_page);
    write_cmd(dev, frame);
    write_cmd(dev, end_page);
    write_cmd(dev, 0x00);
    write_cmd(dev, 0xFF);
}

static void
set_graphic_scroll_HV(struct display *dev, uint8_t direct, uint8_t start_page, uint8_t end_page, uint8_t frame,
                      uint8_t offset) {
    write_cmd(dev, direct);
    write_cmd(dev, 0x00);
    write_cmd(dev, start_page);
    write_cmd(dev, frame);
    write_cmd(dev, end_page);
    write_cmd(dev, offset);
}

static void set_graphic_scroll_range_V(struct display *dev, uint8_t fix_rows, uint8_t scroll_rows) {
    write_cmd(dev, CMD_SCROLL_DOWN_AREA);
    write_cmd(dev, fix_rows);
    write_cmd(dev, scroll_rows);
}

static void set_graphic_scroll_enable(struct display *dev) {
    write_cmd(dev, CMD_SCROLL_ENABLE);
}
*/
static void set_graphic_scroll_disable(struct display *dev) {
    write_cmd(dev, CMD_SCROLL_DISABLE);
}

static void set_pin_config(struct display *dev, uint8_t alternative, uint8_t remap) {
    uint8_t cmd = 0x02;
    cmd |= alternative ? 0x10 : 0x00;
    cmd |= remap ? 0x20 : 0x00;
    write_cmd(dev, CMD_HARD_COM_CONFIG);
    write_cmd(dev, cmd);
}

static void
update_screen_range(struct display *dev, const uint8_t *data, uint8_t start_page, uint8_t end_page, uint8_t start_col,
                    uint8_t end_col) {
    uint8_t r, c;
    set_range(dev, start_page, end_page, start_col, end_col);
    for (r = start_page; r <= end_page; r++)
        for (c = start_col; c <= end_col; c++) {
            write_data(dev, *(data + SCREEN_COLUMNS * r + c));
        }
}

#define update(dev, data, size, ps, pe, cs, ce)             \
do{                                                         \
    set_render_mode(dev, MODE_HORIZONTAL);                  \
    if((dev)->interface->write_data_buffer) {               \
        set_range(dev, ps, pe, cs, ce);                     \
        write_data_buffer(dev, data, size);                 \
    } else                                                  \
        update_screen_range(dev, data, ps, pe, cs, ce);     \
}while(0)


static void
convert(const u8 *src, u8 *dst, u8 depth, unsigned cols, unsigned pages, unsigned line_length, unsigned page_length) {
    size_t index = 0;
    u8 strip = depth / 8;
    int i, j, k;
    /*
    debug("pages[%u], cols[%u], line_len[%u], page_len[%u]", pages, cols, line_length, page_length);
    //*/
    for (i = 0; i < pages; i++) {
        for (j = 0; j < cols; j++) {
            dst[i * page_length + j] = 0;
            for (k = 0; k < 8; k++) {
                index = (i * 8 + k) * line_length + j * strip;
                dst[i * page_length + j] |= (src[index] > TRESHOLD ? 1 : 0) << k;
            }
            //dst[i * page_length + j] = p;
        }
    }
}

#define bit_per_pixel(bpp)  (bpp==1?8:bpp)

static void update_screen_dirty(struct display *dev, struct surface *surface, struct roi *roi) {
    int split_flag;
    u8 *data;
    u8 page_start = roi->row_start / 8;
    u8 page_end = roi->row_end / 8;
    u8 col_start = roi->col_start;
    u8 col_end = roi->col_end;
    size_t padding = roi->padding;
    size_t sf_bpp = surface->depth;
    size_t sf_width = surface->width;
    size_t sf_height = surface->height;
    size_t sf_interval = sf_width - padding;
    size_t sf_line_len = surface->line_length;
    /* valid page length (byte) */
    size_t page_len = col_end - col_start + 1;
    /* valid page count */
    size_t page_num = page_end - page_start + 1;
    /* roi capacity */
    //size_t roi_size = page_num * page_len;
    size_t trans_size;
    ssize_t row_offset = roi->row_offset;
    /* valid cols for transferring  */
    size_t cols = min(page_len, sf_interval);
    data = surface->data + padding * bit_per_pixel(sf_bpp) / 8;
    //debug();
    /* if bpp == 1, line_length >= width, no convert */
    if (sf_bpp == 1) {
        if (padding == 0 && sf_interval <= page_len) {
            split_flag = 0;
            trans_size = cols * page_num;
            col_end = col_start + cols - 1;
        } else {
            split_flag = 1;
            trans_size = cols;
        }
    } else {
        /* if bpp == 8, convert depth to 1 (1 copy)*/
        if (unlikely(!blank.data))
            blank.data = vzalloc(DISPLAY_SIZE);
        split_flag = 0;
        page_num = min(page_num, sf_height / 8);
        trans_size = cols * page_num;
        col_end = col_start + cols - 1;
        convert(data, blank.data, sf_bpp, cols, page_num, sf_line_len, cols);
        data = blank.data;
    }
/*
    debug("bpp[%d] page[%u,%u] col[%u,%u] line_length[%d,%d] trans_size[%d] split[%d]",
          sf_bpp, page_start, page_end, col_start, col_end,
          sf_line_len, page_len,
          trans_size, split_flag);
//*/
    if (split_flag) {
        u8 page;
        for (page = page_start; page <= page_end; page++)
            update(dev, data + sf_line_len * (page - page_start), trans_size, page, page, col_start, col_end);
    } else
        update(dev, data, trans_size, page_start, page_end, col_start, col_end);

    if (row_offset)
        set_mapping(dev, 0, 0, 64 - row_offset);
    else
        set_mapping(dev, -row_offset, -row_offset, 64 + row_offset);

}

int display_init(struct display *dev, unsigned id) {
    int ret = 0;
    debug();
    dev->id = id;
    spin_lock_init(&dev->roi.locker);
    mutex_init(&dev->dev_locker);
    mutex_lock(&dev->dev_locker);
    ret = gpio_request_one(dev->gpio.reset, GPIOF_DIR_OUT, "display-reset");
    if (ret == -EPROBE_DEFER)
        ret = 0;
    if (ret < 0)
        goto done;
    ret = gpio_request_one(dev->gpio.spi_dc, GPIOF_DIR_OUT, "display-dc");
    if (ret == -EPROBE_DEFER)
        ret = 0;
    if (ret < 0)
        goto free;
    mutex_unlock(&dev->dev_locker);
    return ret;

    free:
    gpio_free(dev->gpio.reset);
    done:
    mutex_unlock(&dev->dev_locker);
    mutex_destroy(&dev->dev_locker);
    return ret;
}

void display_deinit(struct display *dev) {
    debug();
    mutex_lock(&dev->dev_locker);
    gpio_free(dev->gpio.reset);
    gpio_free(dev->gpio.spi_dc);
    if (blank.data) {
        vfree(blank.data);
        blank.data = NULL;
    }
    mutex_unlock(&dev->dev_locker);
    mutex_destroy(&dev->dev_locker);
}

void display_reset(struct display *dev) {
    debug();
    //set_reverse(dev, SSD1306_FALSE, SSD1306_TRUE);
    //set_mapping(dev, 0, 0, 64);
    //set_contrast(dev, dev->brightness);
    //set_point_invert(dev, SSD1306_FALSE);
    //set_ignore_ram(dev, SSD1306_FALSE);
    //set_frequency(dev, 8, 1);
    //set_period_pre_charge(dev, 2, 2);
    //set_pin_config(dev, SSD1306_TRUE, SSD1306_FALSE);
    //set_voltage(dev, VOLTAGE_0_DOT_65X);
    //set_graphic_scroll_disable(dev);
    //set_graphic_fade(dev, FADE_OFF, FADE_FRAME_8);
    //set_graphic_zoom(dev, SSD1306_FALSE);
}

void display_turn_on(struct display *dev) {
    debug();
    gpio_set_value(dev->gpio.reset, 0);
    usleep_range(5, 10);
    gpio_set_value(dev->gpio.reset, 1);
    set_reverse(dev, SSD1306_FALSE, SSD1306_TRUE);
    set_mapping(dev, 0, 0, 64);
    set_contrast(dev, DEFAULT_BRIGHTNESS);
    set_point_invert(dev, SSD1306_FALSE);
    set_ignore_ram(dev, SSD1306_FALSE);
    set_frequency(dev, 8, 1);
    set_period_pre_charge(dev, 2, 2);
    set_pin_config(dev, SSD1306_TRUE, SSD1306_FALSE);
    set_voltage(dev, VOLTAGE_0_DOT_65X);
    set_graphic_scroll_disable(dev);
    set_graphic_fade(dev, FADE_OFF, FADE_FRAME_8);
    set_graphic_zoom(dev, SSD1306_FALSE);
    write_cmd(dev, CMD_POWER_CHARGE_PUMP);
    write_cmd(dev, CHARGE_PUMP_ON);
    write_cmd(dev, CMD_DISPLAY_ON);
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
    update_screen_dirty(dev, &blank, &entire);
}

void display_update(struct display *dev, struct surface *surface, struct roi *roi) {
    if (!roi)
        roi = &entire;
    if (unlikely(!roi->dirty))
        return;
    mutex_lock(&dev->dev_locker);
    update_screen_dirty(dev, surface, roi);
    mutex_unlock(&dev->dev_locker);
}

void display_turn_off(struct display *dev) {
    write_cmd(dev, CMD_POWER_CHARGE_PUMP);
    write_cmd(dev, CHARGE_PUMP_OFF);
    write_cmd(dev, CMD_DISPLAY_OFF);
}

int display_set_option(struct display *dev, struct option *opt) {
    set_contrast(dev, opt->brightness);
    return 0;
}

int display_set_roi(struct display *dev, int xoffset, int yoffset, size_t width, size_t height, size_t padding) {
    struct roi *roi = &dev->roi;
    size_t col_end, row_end;
    //debug();
    if (!roi)
        return -EINVAL;
    roi->dirty = false;
    if (xoffset > DISPLAY_WIDTH - 1 || yoffset > DISPLAY_HEIGHT - 1)
        return -EINVAL;
    if (padding >= width)
        return -EINVAL;
    col_end = xoffset + width - 1;
    row_end = yoffset + height - 1;
    if (col_end < 0 || row_end < 0)
        return -EINVAL;
    spin_lock(&roi->locker);
    roi->padding = padding;
    roi->col_start = xoffset < 0 ? 0 : xoffset;
    roi->row_start = yoffset < 0 ? 0 : yoffset;
    roi->col_end = col_end > (DISPLAY_WIDTH - 1) ? (DISPLAY_WIDTH - 1) : col_end;
    roi->row_end = row_end > (DISPLAY_HEIGHT - 1) ? (DISPLAY_HEIGHT - 1) : row_end;
    roi->row_offset = yoffset % 8;
    /*
    debug("col[%u,%u] row[%u,%u] offset[%d]",
          roi->col_start, roi->col_end, roi->row_start, roi->row_end, roi->row_offset);
    */
    roi->dirty = true;
    spin_unlock(&roi->locker);
    return 0;
}
