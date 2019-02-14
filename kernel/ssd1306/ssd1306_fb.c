//
// Created by mumumusuc on 19-2-10.
//
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fb.h>
#include "ssd1306.h"


static int ssd1306_fb_open(struct fb_info *info, int user) {
    struct display *display = info->par;
    printk(KERN_INFO "[%s] \n", __func__);
    display_reset(display);
    display_turn_on(display);
    return 0;
}

static int ssd1306_fb_blank(int blank, struct fb_info *info) {
    printk(KERN_INFO "[%s] \n", __func__);
    return 0;
}

static void ssd1306_fb_deferred_io(struct fb_info *info, struct list_head *list) {
    struct display *display = info->par;
    display_update(display, display->vmem_size);
}

static struct fb_deferred_io ssd1306_defio = {
        .delay = HZ / 30,
        .deferred_io = ssd1306_fb_deferred_io,
};

static struct fb_ops ssd1306_fbops = {
        .owner          = THIS_MODULE,
        .fb_fillrect    = cfb_fillrect,
        .fb_copyarea    = cfb_copyarea,
        .fb_imageblit   = cfb_imageblit,
        .fb_blank       = ssd1306_fb_blank,
        .fb_open        = ssd1306_fb_open,
};

static struct fb_fix_screeninfo ssd1306_fb_fix = {
        .id             = "SSD306_128_64",
        .type           = FB_TYPE_PACKED_PIXELS,
        .visual         = FB_VISUAL_MONO10,
        .xpanstep       = 0,
        .ypanstep       = 0,
        .ywrapstep      = 0,
        .accel          = FB_ACCEL_NONE,
};

static struct fb_var_screeninfo ssd1306_fb_var = {
        .bits_per_pixel = 1,
};


int display_driver_probe(struct device *device, struct display *display, struct interface *interface) {
    int ret = 0;
    char *vmem;
    struct fb_info *fbinfo = framebuffer_alloc(0, device);
    printk(KERN_DEBUG "[%s]\n", __func__);
    if (IS_ERR_OR_NULL(fbinfo)) {
        printk(KERN_ALERT "[%s] fbinfo alloc error\n", __func__);
        return -ENOMEM;
    }
    vmem = vzalloc(DISPLAY_SIZE);
    if (IS_ERR_OR_NULL(vmem)) {
        ret = -ENOMEM;
        goto alloc_failed;
    }
    // init fb data
    fbinfo->var = ssd1306_fb_var;
    fbinfo->fix = ssd1306_fb_fix;
    fbinfo->screen_buffer = vmem;
    fbinfo->fix.smem_start = (unsigned long) vmem;
    fbinfo->fix.smem_len = DISPLAY_SIZE;
    fbinfo->fix.line_length = DISPLAY_WIDTH;
    fbinfo->fbops = &ssd1306_fbops;
    fbinfo->var.xres = DISPLAY_WIDTH;
    fbinfo->var.yres = DISPLAY_HEIGHT;
    fbinfo->var.xres_virtual = DISPLAY_WIDTH;
    fbinfo->var.yres_virtual = DISPLAY_HEIGHT;
    fbinfo->var.red.length = 1;
    fbinfo->var.red.offset = 0;
    fbinfo->var.green.length = 1;
    fbinfo->var.green.offset = 0;
    fbinfo->var.blue.length = 1;
    fbinfo->var.blue.offset = 0;
    fbinfo->fbdefio = &ssd1306_defio;
    fb_deferred_io_init(fbinfo);
    display->vmem = vmem;
    ret = display_init(display, interface);
    if (ret < 0) {
        goto alloc_failed;
    }
    // register
    fbinfo->par = display;
    display->par = fbinfo;
    ret = register_framebuffer(fbinfo);
    if (ret < 0) {
        dev_err(device, "[%s] register framebuffer failed \n", __func__);
        goto register_failed;
    }
    printk(KERN_DEBUG "[%s] register framebuffer%d\n", __func__, fbinfo->node);
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
    printk(KERN_DEBUG "[%s]\n", __func__);
    fb_deferred_io_cleanup(fbinfo);
    unregister_framebuffer(fbinfo);
    framebuffer_release(fbinfo);
    display->vmem = NULL;
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
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("mumumusuc");