//
// Created by mumumusuc on 19-2-10.
//
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fb.h>
#include <linux/uaccess.h>
#include "ssd1306.h"
#include "pi_logo.h"

#define DISPLAY_DEPTH_1     1
#define DISPLAY_DEPTH_8     8
#define DISPLAY_MAX_WIDTH   DISPLAY_WIDTH
#define DISPLAY_MAX_HEIGHT  DISPLAY_HEIGHT
#define DISPLAY_MAX_SIZE    (DISPLAY_MAX_WIDTH*DISPLAY_MAX_HEIGHT*DISPLAY_DEPTH_8/8)
#define DISPLAY_MAX_LINE    (DISPLAY_MAX_WIDTH*DISPLAY_DEPTH_8/8)


static struct fb_fix_screeninfo ssd1306_fb_fix = {
        .id             = "SSD306_128_64",
        .type           = FB_TYPE_PACKED_PIXELS,
        .visual         = FB_VISUAL_MONO10,
        .xpanstep       = 0,
        .ypanstep       = 0,
        .ywrapstep      = 0,
        .accel          = FB_ACCEL_NONE,
        .line_length    = DISPLAY_MAX_LINE,
        .smem_len       = DISPLAY_MAX_SIZE,
};

static struct fb_var_screeninfo ssd1306_fb_var = {
        .bits_per_pixel = DISPLAY_DEPTH_1,
        .xres           = DISPLAY_WIDTH,
        .yres           = DISPLAY_HEIGHT,
        .xres_virtual   = DISPLAY_MAX_WIDTH,
        .yres_virtual   = DISPLAY_MAX_HEIGHT,
        .xoffset        = 0,
        .yoffset        = 0,
        .red.length     = 1,
        .red.offset     = 0,
        .green.length   = 1,
        .green.offset   = 0,
        .blue.length    = 1,
        .blue.offset    = 0,
};

// NOTICE: user is always 1
static int ssd1306_fb_open(struct fb_info *info, int user) {
    struct display *display = info->par;
    display->user++;
    debug("user[%d]", display->user);
    return 0;
}

static int ssd1306_fb_release(struct fb_info *info, int user) {
    struct display *display = info->par;
    debug("user[%d]", display->user);
    if (user == 1) {
        info->fbops->fb_check_var(&ssd1306_fb_var, info);
    }
    display->user--;
    return 0;
}

static ssize_t ssd1306_fb_write(struct fb_info *info, const char __user *buf, size_t count, loff_t *ppos) {
    ssize_t ret = 0;
    struct display *display = info->par;
    ret = fb_sys_write(info, buf, count, ppos);
    /*
    unsigned long p = *ppos;
    void *dst;
    int err = 0;
    unsigned long total_size;
    if (info->state != FBINFO_STATE_RUNNING)
        return -EPERM;
    total_size = info->screen_size;
    if (total_size == 0)
        total_size = info->fix.smem_len;
    if (p > total_size)
        return -EFBIG;
    if (count > total_size) {
        err = -EFBIG;
        count = total_size;
    }
    if (count + p > total_size) {
        if (!err)
            err = -ENOSPC;

        count = total_size - p;
    }
    dst = (void __force *) (info->screen_base + p);

    if (info->fbops->fb_sync)
        info->fbops->fb_sync(info);
    if (copy_from_user(dst, buf, count))
        err = -EFAULT;
    if (!err)
        *ppos += count;
    ret = (err) ? err : count;
     */
    if (ret == count) {
        display->surface.data = info->screen_buffer;
        display_update(display, &display->surface);
    }
    return ret;
}

static int ssd1306_fb_blank(int blank, struct fb_info *info) {
    struct display *display = info->par;
    debug("blank[%d]", blank);
    if (blank == FB_BLANK_UNBLANK) {
        size_t size;
        struct linux_logo logo;
        struct surface surface;
#ifdef CONFIG_LOGO_LINUX_MONO
        logo = logo_raspberry_mono;
        surface.bpp  = 1;
#endif
#ifdef CONFIG_LOGO_LINUX_CLUT224
        logo = logo_raspberry_clut224;
        surface.bpp  = 8;
#endif
        size = logo.width * logo.height * surface.bpp / 8;
        surface.width = logo.width;
        surface.height = logo.width;
        surface.padding = 0;//logo.width / 2;
        surface.line_length = logo.width;
        if (size > info->fix.smem_len) {
            surface.data = (u8 *) logo.data;
        } else {
            mutex_lock(&display->mem_mutex);
            memmove(info->screen_buffer, logo.data, size);
            surface.data = info->screen_buffer;
            mutex_unlock(&display->mem_mutex);
        }
        display_clear(display);
        display_update(display, &surface);
    } else if (blank == FB_BLANK_POWERDOWN) {
        display_turn_off(display);
    } else if (blank == FB_BLANK_NORMAL) {
        display_reset(display);
        display_turn_on(display);
        display_clear(display);
    }
    return 0;
}

static void ssd1306_fb_deferred_io(struct fb_info *info, struct list_head *list) {
    struct display *display = info->par;
    display->surface.data = info->screen_buffer;
    display_update(display, &display->surface);
}

static int ssd1306_fb_check_var(struct fb_var_screeninfo *var, struct fb_info *info) {
    int ret = 0;
    u8 bpp;
    size_t width, height, xoffset, yoffset;
    struct display *display = info->par;
    if (display->user > 1)
        return -EBUSY;
    bpp = var->bits_per_pixel;
    if (bpp != DISPLAY_DEPTH_1 && bpp != DISPLAY_DEPTH_8)
        return -EINVAL;
    width = var->xres;
    height = var->yres;
    xoffset = var->xoffset;
    yoffset = var->yoffset;
    if (width > DISPLAY_MAX_WIDTH || height > DISPLAY_MAX_HEIGHT)
        return -EINVAL;
    ret = display_set_roi(display, xoffset, yoffset, width, height);
    if (ret != 0)
        return ret;

    display->surface.bpp = bpp;
    display->surface.width = width;
    display->surface.height = height;
    display->surface.padding = 0;
    display->surface.line_length = width;

    info->var.bits_per_pixel = bpp;
    info->var.xoffset = xoffset;
    info->var.yoffset = yoffset;
    info->var.xres = width;
    info->var.yres = height;
    return ret;
}

static struct fb_deferred_io ssd1306_defio = {
        .delay          = HZ / 30,
        .deferred_io    = ssd1306_fb_deferred_io,
};

static struct fb_ops ssd1306_fbops = {
        //.owner          = THIS_MODULE,
        .fb_fillrect    = cfb_fillrect,
        .fb_copyarea    = cfb_copyarea,
        .fb_imageblit   = cfb_imageblit,
        .fb_write       = ssd1306_fb_write,
        .fb_blank       = ssd1306_fb_blank,
        .fb_open        = ssd1306_fb_open,
        .fb_release     = ssd1306_fb_release,
        .fb_check_var   = ssd1306_fb_check_var,
};

int display_driver_probe(struct device *device, struct display *display) {
    int ret = 0;
    unsigned char *vmem;
    struct fb_info *fbinfo = framebuffer_alloc(0, device);
    debug();
    if (IS_ERR_OR_NULL(fbinfo))
        return -ENOMEM;
    vmem = vzalloc(ssd1306_fb_fix.smem_len);
    if (IS_ERR_OR_NULL(vmem)) {
        ret = -ENOMEM;
        goto alloc_failed;
    }
    fbinfo->var = ssd1306_fb_var;
    fbinfo->fix = ssd1306_fb_fix;
    fbinfo->screen_buffer = vmem;
    fbinfo->fix.smem_start = (unsigned long) vmem;

    fbinfo->fbops = &ssd1306_fbops;
    fbinfo->fbdefio = &ssd1306_defio;
    fb_deferred_io_init(fbinfo);
    ret = display_init(display);
    if (ret < 0) {
        goto alloc_failed;
    }
    fbinfo->par = display;
    display->par = fbinfo;
    fbinfo->fbops->fb_check_var(&fbinfo->var, fbinfo);

    ret = register_framebuffer(fbinfo);
    if (ret < 0)
        goto register_failed;
    return ret;

    register_failed:
    pr_err("register failed\n");
    display_deinit(display);
    alloc_failed:
    pr_err("alloc failed\n");
    framebuffer_release(fbinfo);
    return ret;
}

void dislay_driver_remove(struct display *display) {
    struct fb_info *fbinfo = display->par;
    debug();
    fb_deferred_io_cleanup(fbinfo);
    unregister_framebuffer(fbinfo);
    framebuffer_release(fbinfo);
    display_deinit(display);
}

// module methods
static int __init ssd1306_init(void) {
    int ret = 0;
    ret = display_driver_spi_init();
    //ret = display_driver_i2c_init();
    return ret;
}

static void __exit ssd1306_exit(void) {
    display_driver_spi_exit();
    //display_driver_i2c_exit();
}

module_init(ssd1306_init)
module_exit(ssd1306_exit)
MODULE_LICENSE("GPL");
MODULE_AUTHOR("mumumusuc@gmail.com");