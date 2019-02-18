//
// Created by mumumusuc on 19-2-9.
//

#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include "ssd1306.h"

#define roi(dev)                            (dev)->roi
#define write_cmd(dev, cmd)                 (dev)->interface->write_cmd((dev), (cmd));
#define write_data(dev, data)               (dev)->interface->write_data((dev), (data));
#define write_data_buffer(dev, buf, size)   (dev)->interface->write_data_buffer((dev),(buf),(size));

struct roi {
    struct spinlock locker;
    int row_offset;
    int col_start;
    int row_start;
    int col_end;
    int row_end;
};


static struct surface blank = {
        .bpp         = 1,
        .padding     = 0,
        .data        = NULL,
        .width       = DISPLAY_WIDTH,
        .height      = DISPLAY_HEIGHT,
        .line_length = DISPLAY_LINE,
};
static struct roi entire = {
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
    uint8_t p = 0;
    int i, j, k;
    /*
    debug("pages[%d], cols[%d], line_length[%d]", pages, cols, line_length);
    */
    for (i = 0; i < pages; i++) {
        for (j = 0; j < cols; j++) {
            p = 0;
            for (k = 0; k < 8; k++) {
                index = (i * 8 + k) * line_length + j;
                p |= (src[index] > 100 ? 1 : 0) << k;
            }
            dst[i * page_length + j] = p;
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
    size_t sf_bpp = surface->bpp;
    size_t sf_width = surface->width;
    size_t sf_padding = surface->padding;
    size_t sf_interval = sf_width - sf_padding;
    size_t sf_line_len = surface->line_length;
    /* valid page length (byte) */
    size_t page_len = col_end - col_start + 1;
    /* valid page count */
    size_t page_num = page_end - page_start + 1;
    /* roi capacity */
    size_t roi_size = page_num * page_len;
    size_t trans_size;
    ssize_t row_offset = roi->row_offset;
    /* valid cols for transferring  */
    size_t cols = min(page_len, sf_interval);
    data = surface->data + sf_padding * bit_per_pixel(sf_bpp) / 8;
    /* if bpp == 1, line_length >= width, no convert */
    if (sf_bpp == 1) {
        if (sf_padding == 0 && sf_interval == page_len) {
            split_flag = 0;
            trans_size = roi_size;
        } else if (sf_padding == 0 && sf_interval < page_len) {
            split_flag = 0;
            trans_size = cols * page_num;
            col_end = col_start + cols - 1;
        } else {
            split_flag = 1;
            trans_size = cols;
        }
    } else {
        /* if bpp == 8, convert depth to 1 (1 copy)*/
        if (!blank.data)
            blank.data = vzalloc(DISPLAY_SIZE);
        split_flag = 0;
        trans_size = roi_size;
        convert(data, blank.data, sf_bpp, cols, page_num, sf_line_len, page_len);
        data = blank.data;
    }
/*
    debug("bpp[%d] page[%u,%u] col[%u,%u] line_length[%d,%d] trans_size[%d] split[%d]",
          sf_bpp, page_start, page_end, col_start, col_end,
          sf_line_len, page_len,
          trans_size, split_flag);
*/
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
// end private

int display_init(struct display *dev) {
    int ret = 0;
    char gpio_tmp[16];
    debug();
    if (!dev->gpio_reset)
        return -EFAULT;
    sprintf(gpio_tmp, "ssd1306_rst_%u", dev->gpio_reset);
    debug("init %s", gpio_tmp);
    ret = gpio_request_one(dev->gpio_reset, GPIOF_OUT_INIT_HIGH, gpio_tmp);
    if (ret < 0) {
        pr_err("gpio_request(%s)failed with %d\n", gpio_tmp, ret);
        return ret;
    }
    if (!dev->roi) {
        dev->roi = kmalloc(sizeof(struct roi), GFP_KERNEL);
        spin_lock_init(&roi(dev)->locker);
    }
    mutex_init(&dev->mem_mutex);
    return ret;
}

void display_deinit(struct display *dev) {
    debug();
    gpio_free(dev->gpio_reset);
    if (dev->roi)
        kfree(dev->roi);
    dev->roi = NULL;
    if (blank.data)
        vfree(blank.data);
    mutex_destroy(&dev->mem_mutex);
}

void display_reset(struct display *dev) {
    debug();
    gpio_set_value(dev->gpio_reset, 0);
    udelay(5);
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
    debug();
    write_cmd(dev, CMD_POWER_CHARGE_PUMP);
    write_cmd(dev, CHARGE_PUMP_ON);
    write_cmd(dev, CMD_DISPLAY_ON);
}

void display_clear(struct display *dev) {
    debug();
    if (blank.data)
        memset(blank.data, 0, DISPLAY_SIZE);
    else
        blank.data = vzalloc(DISPLAY_SIZE);
    update_screen_dirty(dev, &blank, &entire);
}

void display_update(struct display *dev, struct surface *surface) {
    mutex_lock(&dev->mem_mutex);
    update_screen_dirty(dev, surface, dev->roi);
    mutex_unlock(&dev->mem_mutex);
}

void display_turn_off(struct display *dev) {
    write_cmd(dev, CMD_POWER_CHARGE_PUMP);
    write_cmd(dev, CHARGE_PUMP_OFF);
    write_cmd(dev, CMD_DISPLAY_OFF);
}

int display_set_roi(struct display *dev, int xoffset, int yoffset, size_t width, size_t height) {
    size_t col_end, row_end;
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
    return 0;
}