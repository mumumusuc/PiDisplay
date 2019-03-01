//
// Created by mumumusuc on 19-2-10.
//
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fb.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include "display.h"
#include "ili9341.h"

#define GPIO_DC     26
#define GPIO_RST    25
#define GPIO_LED    19

#define DISPLAY_MAX_NUM     2
#define DISPLAY_MAX_DEPTH   DISPLAY_BBP
#define DISPLAY_MAX_WIDTH   DISPLAY_WIDTH
#define DISPLAY_MAX_HEIGHT  DISPLAY_HEIGHT
#define DISPLAY_MAX_LINE    (DISPLAY_MAX_WIDTH*DISPLAY_MAX_DEPTH/8)
#define DISPLAY_MAX_SIZE    (DISPLAY_MAX_LINE*DISPLAY_MAX_HEIGHT)

static struct mutex list_locker;
/* how many displays and drivers */
static LIST_HEAD(display_list);
static DECLARE_BITMAP(display_ids, DISPLAY_MAX_NUM);

typedef struct {
    unsigned user;
    struct fb_info *fb;
    struct display *display;
    struct roi *roi;
    struct surface *surface;
    struct list_head list;
} display_t;

static struct fb_fix_screeninfo display_fb_fix = {
        .id             = DISPLAY_MODULE,
        .type           = FB_TYPE_PACKED_PIXELS,
        .visual         = FB_VISUAL_TRUECOLOR,
        .accel          = FB_ACCEL_NONE,
        .line_length    = DISPLAY_MAX_LINE,
};

static struct fb_var_screeninfo display_fb_var = {
        .bits_per_pixel = DISPLAY_MAX_DEPTH,
        .xres           = DISPLAY_WIDTH,
        .yres           = DISPLAY_HEIGHT,
        .xres_virtual   = DISPLAY_MAX_WIDTH,
        .yres_virtual   = DISPLAY_MAX_HEIGHT,
        .xoffset        = 0,
        .yoffset        = 0,
        .red.length     = 8,
        .red.offset     = 0,
        .green.length   = 8,
        .green.offset   = 8,
        .blue.length    = 8,
        .blue.offset    = 16,
};

static int display_fb_open(struct fb_info *info, int user) {
    display_t *dev = info->par;
    if (dev->user == 0)
        display_turn_on(dev->display);
    dev->user++;
    return 0;
}

static int display_fb_release(struct fb_info *info, int user) {
    display_t *dev = info->par;
    dev->user--;
    if (dev->user == 0) {
        display_turn_off(dev->display);
        //info->fbops->fb_check_var(&display_fb_var, info);
    }
    return 0;
}

static int update_display(struct fb_info *info) {
    /*
    display_t *dev = info->par;
    struct display *display = dev->display;
    mutex_lock(&info->mm_lock);
    display->roi = dev->roi;
    dev->surface->data = (unsigned char *) info->fix.smem_start;
    display_update(display, dev->surface);
    mutex_unlock(&info->mm_lock);
     */
    return 0;
}

static ssize_t display_fb_write(struct fb_info *info, const char __user *buf, size_t count, loff_t *ppos) {
    ssize_t ret = 0;
    ret = fb_sys_write(info, buf, count, ppos);
    if (ret == count)
        update_display(info);
    return ret;
}

static int display_fb_blank(int blank, struct fb_info *info) {
    display_t *dev = info->par;
    struct display *display = dev->display;
    debug("blank[%d]", blank);
    switch (blank) {
        case FB_BLANK_POWERDOWN:
            display_turn_off(display);
            break;
        case FB_BLANK_NORMAL:
            display_reset(display);
            display_clear(display);
            display_turn_on(display);
            break;
        default:
            break;
    }
    return 0;
}

static void display_fb_deferred_io(struct fb_info *info, struct list_head *list) {
    update_display(info);
}

static int display_fb_check_var(struct fb_var_screeninfo *var, struct fb_info *info) {
    int ret = 0;
    size_t bpp, width, height, xoffset, yoffset;
    display_t *dev = info->dev;
    struct display *display = dev->display;

    if (list_empty(&display_list))
        return -ENODEV;

    bpp = var->bits_per_pixel;
    if (bpp > DISPLAY_MAX_DEPTH)
        return -EINVAL;

    width = var->xres;
    height = var->yres;
    xoffset = var->xoffset;
    yoffset = var->yoffset;

    if (width > DISPLAY_MAX_WIDTH || height > DISPLAY_MAX_HEIGHT)
        return -EINVAL;

    if (dev->user > 1)
        return -EBUSY;
    display->roi = dev->roi;
    display_set_roi(display, xoffset, yoffset, width, height);
    dev->surface->bpp = bpp;
    dev->surface->width = width;
    dev->surface->height = height;
    dev->surface->line_length = width * bpp / 8;
    dev->surface->padding = 0;

    info->var.xres = width;
    info->var.yres = height;
    info->var.xoffset = xoffset;
    info->var.yoffset = yoffset;
    info->var.bits_per_pixel = bpp;

    return ret;
}

static struct fb_deferred_io display_defio = {
        .delay          = HZ / 60,
        .deferred_io    = display_fb_deferred_io,
};

static struct fb_ops display_fbops = {
        .owner          = THIS_MODULE,
        .fb_fillrect    = cfb_fillrect,
        .fb_copyarea    = cfb_copyarea,
        .fb_imageblit   = cfb_imageblit,
        .fb_open        = display_fb_open,
        .fb_release     = display_fb_release,
        .fb_check_var   = display_fb_check_var,
        .fb_blank       = display_fb_blank,
        .fb_write       = display_fb_write,
};

static int prepare_display(struct display *display, display_t **par) {
    int ret = 0;
    unsigned long id;
    display_t *dev;
    debug();
    /* init display (one driver pairs one display)*/
    id = find_first_zero_bit(display_ids, DISPLAY_MAX_NUM);
    if (id > DISPLAY_MAX_NUM - 1) {
        pr_err("out of display max range(max:%u , req:%ld)\n", DISPLAY_MAX_NUM, id);
        return -ENODEV;
    }
    ret = display_init(display, id);
    if (ret < 0) {
        pr_err("display(%ld) init failed\n", id);
        return ret;
    }
    dev = kzalloc(sizeof(display_t), GFP_KERNEL);
    if (!dev) {
        ret = -ENOMEM;
        goto deinit;
    }
    dev->user = 0;
    dev->fb = NULL;
    dev->display = display;
    dev->surface = kzalloc(sizeof(struct surface), GFP_KERNEL);
    if (!dev->surface) {
        ret = -ENOMEM;
        goto deinit;
    }
    dev->roi = kzalloc(sizeof(struct roi), GFP_KERNEL);
    if (!dev->roi) {
        ret = -ENOMEM;
        goto free_surface;
    }
    spin_lock_init(&dev->roi->locker);
    set_bit(id, display_ids);
    *par = dev;
    return ret;

    /* failure */
    free_surface:
    kfree(dev->surface);
    deinit:
    display_deinit(display);
    return ret;
}

static int prepare_framebuffer(struct device *device, void *par) {
    struct fb_info *fbinfo;
    debug();
    /* alloc framebuffer */
    fbinfo = framebuffer_alloc(0, device);
    if (!fbinfo)
        return -ENOMEM;
    /* init framebuffer info */
    fbinfo->par = par;
    fbinfo->var = display_fb_var;
    fbinfo->fix = display_fb_fix;
    fbinfo->fbops = &display_fbops;
    fbinfo->fbdefio = &display_defio;
    fb_deferred_io_init(fbinfo);
    /* register framebuffer */
    return register_framebuffer(fbinfo);
}

int display_driver_probe(struct device *device, struct display *display) {
    int ret = 0;
    size_t vmem_len;
    u8 *vmem;
    display_t *dev;
    debug();
    /* init display*/
    ret = prepare_display(display, &dev);
    if (ret != 0) {
        dev_dbg(device, "init display failed\n");
        goto free_mem;
    }
    /* add to list */
    INIT_LIST_HEAD(&dev->list);
    mutex_lock(&list_locker);
    list_add_tail(&dev->list, &display_list);
    mutex_unlock(&list_locker);
    /* alloc video memory*/
    vmem_len = DISPLAY_MAX_SIZE;
    vmem = vzalloc(vmem_len);
    if (!vmem) {
        dev_dbg(device, "alloc video memory failed\n");
        return -ENOMEM;
    }
    ret = prepare_framebuffer(device, dev);
    if (ret != 0) {
        pr_err("prepare framebuffer failed\n");
        goto free_mem;
    }
    dev->fb->screen_buffer = vmem;
    dev->fb->fix.smem_len = vmem_len;
    dev->fb->fix.smem_start = (unsigned long) vmem;
    /* link fb & display */
    display_fb_check_var(&dev->fb->var, dev->fb);
    return ret;

    /* failure */
    free_mem:
    vfree(vmem);
    return ret;
}

void dislay_driver_remove(struct display *display) {
    u8 id = display->id;
    struct fb_info *info;
    display_t *dev = NULL, *next = NULL;
    debug();
    mutex_lock(&list_locker);
    list_for_each_entry_safe(dev, next, &display_list, list) {
        if (dev->display->id != id)
            continue;
        debug("remove display[%u]", id);
        kfree(dev->roi);
        kfree(dev->surface);
        info = dev->fb;
        debug("unregister fb[%d]", info->node);
        fb_deferred_io_cleanup(info);
        unregister_framebuffer(info);
        framebuffer_release(info);
        list_del(&dev->list);
        clear_bit(id, display_ids);
    }
    mutex_unlock(&list_locker);
    display_deinit(display);
}

// module methods
static int __init display_module_init(void) {
    int ret = 0;
    mutex_init(&list_locker);
    //ret = display_driver_spi_init();
    return ret;
}

static void __exit display_module_exit(void) {
    //display_driver_spi_exit();
    mutex_destroy(&list_locker);
}

module_init(display_module_init)
module_exit(display_module_exit)
MODULE_LICENSE("GPL");
MODULE_AUTHOR("mumumusuc@gmail.com");

