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
#include "ssd1306.h"
#include "pi_logo.h"

#define DISPLAY_MULTI   2
//#define DISPLAY_DIV   2

#ifdef DISPLAY_MULTI
#define DISPLAY_MULTI_H_2
//#define DISPLAY_MULTI_V_2
#endif
#ifdef DISPLAY_DIV
//#define DISPLAY_DIV_H_2
//#define DISPLAY_DIV_V_2
#endif

#define DISPLAY_NUM         2
#define DISPLAY_MAX_NUM     4
#define DISPLAY_DEPTH_1     1
#define DISPLAY_DEPTH_8     8
#define DISPLAY_DEPTH_16    16
#define DISPLAY_MAX_DEPTH   DISPLAY_DEPTH_8

#ifdef DISPLAY_MULTI_H_2
#define DISPLAY_MAX_WIDTH   (DISPLAY_WIDTH*2)
#elif   defined(DISPLAY_DIV_H_2)
#define DISPLAY_MAX_WIDTH   (DISPLAY_WIDTH/2)
#else
#define DISPLAY_MAX_WIDTH   DISPLAY_WIDTH
#endif

#ifdef DISPLAY_MULTI_V_2
#define DISPLAY_MAX_HEIGHT  (DISPLAY_HEIGHT*2)
#elif   defined(DISPLAY_DIV_V_2)
#define DISPLAY_MAX_HEIGHT   (DISPLAY_HEIGHT/2)
#else
#define DISPLAY_MAX_HEIGHT  DISPLAY_HEIGHT
#endif

#define DISPLAY_MAX_SIZE    (DISPLAY_NUM*DISPLAY_WIDTH*DISPLAY_HEIGHT*DISPLAY_MAX_DEPTH/8)
#define DISPLAY_STRIP_SIZE  (DISPLAY_MAX_WIDTH*DISPLAY_MAX_HEIGHT*DISPLAY_MAX_DEPTH/8)
#define DISPLAY_MAX_LINE    (DISPLAY_MAX_WIDTH*DISPLAY_MAX_DEPTH/8)

static struct mutex list_locker;
/* how many displays and drivers */
static LIST_HEAD(display_list);
/* how many framebuffers */
static LIST_HEAD(fb_list);
static DECLARE_BITMAP(display_ids, DISPLAY_MAX_NUM);

struct display_wrapper {
    unsigned user;
    unsigned sub_id;
    struct fb_info *fbinfo;
    struct display *display;
    struct roi *roi;
    struct surface *surface;
    struct list_head list;
};

struct fb_wrapper {
    unsigned id;
    struct display_wrapper *displays[DISPLAY_MAX_NUM];
    struct fb_info *fb;
    struct list_head list;
};

static struct fb_fix_screeninfo ssd1306_fb_fix = {
        .id             = DISPLAY_MODULE,
        .type           = FB_TYPE_PACKED_PIXELS,
        .visual         = FB_VISUAL_MONO10,
        .xpanstep       = 0,
        .ypanstep       = 0,
        .ywrapstep      = 0,
        .accel          = FB_ACCEL_NONE,
        .line_length    = DISPLAY_MAX_LINE,
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
    struct display_wrapper *wrapper = info->par;
    wrapper->user++;
    debug("user[%d]", wrapper->user);
    return 0;
}

static int ssd1306_fb_release(struct fb_info *info, int user) {
    struct display_wrapper *wrapper = info->par;
    debug("user[%d]", wrapper->user);
    if (user == 1) {
        info->fbops->fb_check_var(&ssd1306_fb_var, info);
    }
    wrapper->user--;
    return 0;
}

static int update_display(struct fb_info *info) {
    int index;
    struct fb_wrapper *fb_wrapper = info->par;
    struct display_wrapper *wrapper;
    struct display *display;
    for (index = 0; index < DISPLAY_MAX_NUM; index++) {
        wrapper = fb_wrapper->displays[index];
        if (wrapper) {
            display = wrapper->display;
            display->roi = wrapper->roi;
            wrapper->surface->data = (unsigned char *) info->fix.smem_start;
            display_update(display, wrapper->surface);
        }
    }
    /* need locker? */
    /*
    mutex_lock(&list_locker);
    list_for_each_entry(wrapper, &display_list, list) {
        if (info->node == wrapper->fbinfo->node) {
            display = wrapper->display;
            display->roi = wrapper->roi;
            wrapper->surface->data = (unsigned char *) info->fix.smem_start;
            display_update(display, wrapper->surface);
        }
    }
    mutex_unlock(&list_locker);
*/
    return 0;
}

static ssize_t ssd1306_fb_write(struct fb_info *info, const char __user *buf, size_t count, loff_t *ppos) {
    ssize_t ret = 0;
    //struct display_wrapper *wrapper = info->par;
    //struct display *display = wrapper->display;
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
        update_display(info);
        //wrapper->surface->data = info->fix.smem_start;
        //display->roi = wrapper->roi;
        //display_update(display, wrapper->surface);
    }
    return ret;
}

static int ssd1306_fb_sync(struct fb_info *info){
    update_display(info);
    return 0;
}

static int ssd1306_fb_blank(int blank, struct fb_info *info) {
    struct display_wrapper *wrapper;
    struct display *display;
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
        surface.padding = 0;
        surface.line_length = logo.width;
        //if (size > info->fix.smem_len) {
        //    surface.data = (u8 *) logo.data;
        //} else {
        mutex_lock(&info->mm_lock);
        memmove(info->screen_buffer, logo.data, size);
        //surface.data = info->screen_buffer;
        mutex_unlock(&info->mm_lock);
        //}
        //display_clear(display);
        update_display(info);
    } else if (blank == FB_BLANK_POWERDOWN) {
        mutex_lock(&list_locker);
        list_for_each_entry(wrapper, &display_list, list) {
            if (info->node != wrapper->fbinfo->node)
                continue;
            display = wrapper->display;
            display_turn_off(display);
        }
        mutex_unlock(&list_locker);
    } else if (blank == FB_BLANK_NORMAL) {
        int index;
        struct display_wrapper *wrapper;
        struct display *display;
        struct fb_wrapper *fb_wrapper = info->par;
        for (index = 0; index < DISPLAY_MAX_NUM; index++) {
            wrapper = fb_wrapper->displays[index];
            if (wrapper) {
                display = wrapper->display;
                display_reset(display);
                display_turn_on(display);
                display_clear(display);
            }
        }
        /*
        mutex_lock(&list_locker);
        list_for_each_entry(wrapper, &display_list, list) {
            if (info->node != wrapper->fbinfo->node)
                continue;
            display = wrapper->display;
            display_reset(display);
            display_turn_on(display);
            display_clear(display);
        }
        mutex_unlock(&list_locker);
         */
    }
    return 0;
}

static void ssd1306_fb_deferred_io(struct fb_info *info, struct list_head *list) {
    update_display(info);
    /*
    struct display_wrapper *wrapper = info->par;
    wrapper->surface->data = info->fix.smem_start;
    wrapper->display->roi = wrapper->roi;
    display_update(wrapper->display, wrapper->surface);
    */
}

static int ssd1306_fb_check_var(struct fb_var_screeninfo *var, struct fb_info *info) {
    int ret = 0, index;
    u8 bpp;
    size_t width, height, xoffset, yoffset;
    struct display_wrapper *wrapper;
    struct display *display;
    struct fb_wrapper *fb_wrapper = info->par;
    if (list_empty(&display_list))
        return -ENODEV;

    bpp = var->bits_per_pixel;
    debug("requre bpp %d", bpp);
    if (bpp != DISPLAY_DEPTH_1 && bpp != DISPLAY_DEPTH_8 && bpp != DISPLAY_DEPTH_16)
        return -EINVAL;

    width = var->xres;
    height = var->yres;
    xoffset = var->xoffset;
    yoffset = var->yoffset;
    debug("requre resolution[%d , %d]", width, height);
    if (width > DISPLAY_MAX_WIDTH || height > DISPLAY_MAX_HEIGHT)
        return -EINVAL;

#ifdef DISPLAY_MULTI
    for (index = 0; index < DISPLAY_MAX_NUM; index++) {
        wrapper = fb_wrapper->displays[index];
        if (wrapper) {
            display = wrapper->display;
            if (wrapper->user > 1)
                return -EBUSY;
            display->roi = wrapper->roi;
            display_set_roi(
                    display,
                    xoffset - index * DISPLAY_WIDTH,
                    yoffset,
                    DISPLAY_WIDTH,
                    height);
            wrapper->surface->bpp = bpp;
            wrapper->surface->width = width;
            wrapper->surface->height = height;
            wrapper->surface->padding = index * (xoffset < DISPLAY_WIDTH ? (DISPLAY_WIDTH - xoffset) : 0);
            wrapper->surface->line_length = width;
        }
    }
    info->var.bits_per_pixel = bpp;
    info->var.xoffset = xoffset;
    info->var.yoffset = yoffset;
    info->var.xres = width;
    info->var.yres = height;
#endif
    return ret;
    /*
    mutex_lock(&list_locker);
    list_for_each_entry(wrapper, &display_list, list) {
        if (wrapper->fbinfo->node != info->node)
            continue;

        display = wrapper->display;
        if (wrapper->user > 1) {
            ret = -EBUSY;
            break;
        }
        bpp = var->bits_per_pixel;
        if (bpp != DISPLAY_DEPTH_1 && bpp != DISPLAY_DEPTH_8) {
            ret = -EINVAL;
            break;
        }

        width = var->xres;
        height = var->yres;
        xoffset = var->xoffset;
        yoffset = var->yoffset;

        if (width > DISPLAY_MAX_WIDTH || height > DISPLAY_MAX_HEIGHT) {
            ret = -EINVAL;
            break;
        }
        display->roi = wrapper->roi;
        ret = display_set_roi(display, xoffset, yoffset, width, height);
        if (ret != 0)
            break;
        wrapper->surface->bpp = bpp;
        wrapper->surface->width = width;
        wrapper->surface->height = height;
        wrapper->surface->padding = 0;
        wrapper->surface->line_length = width;

        info->var.bits_per_pixel = bpp;
        info->var.xoffset = xoffset;
        info->var.yoffset = yoffset;
        info->var.xres = width;
        info->var.yres = height;
    }
    mutex_unlock(&list_locker);
*/
    return ret;
}

static struct fb_deferred_io ssd1306_defio = {
        .delay          = HZ / 60,
        .deferred_io    = ssd1306_fb_deferred_io,
};

static struct fb_ops ssd1306_fbops = {
        .owner          = THIS_MODULE,
        .fb_fillrect    = cfb_fillrect,
        .fb_copyarea    = cfb_copyarea,
        .fb_imageblit   = cfb_imageblit,
        .fb_sync        = ssd1306_fb_sync,
        .fb_write       = ssd1306_fb_write,
        .fb_blank       = ssd1306_fb_blank,
        .fb_open        = ssd1306_fb_open,
        .fb_release     = ssd1306_fb_release,
        .fb_check_var   = ssd1306_fb_check_var,
};

static int prepare_display(struct display *display, struct display_wrapper **par) {
    int ret = 0;
    unsigned long id;
    struct display_wrapper *wrapper;
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
    wrapper = kzalloc(sizeof(struct display_wrapper), GFP_KERNEL);
    if (!wrapper) {
        ret = -ENOMEM;
        goto deinit;
    }
    wrapper->user = 0;
    wrapper->fbinfo = NULL;
    wrapper->display = display;
    wrapper->surface = kzalloc(sizeof(struct surface), GFP_KERNEL);
    if (!wrapper->surface) {
        ret = -ENOMEM;
        goto deinit;
    }
    wrapper->roi = kzalloc(sizeof(struct roi), GFP_KERNEL);
    if (!wrapper->roi) {
        ret = -ENOMEM;
        goto free_surface;
    }
    spin_lock_init(&wrapper->roi->locker);
    set_bit(id, display_ids);
    *par = wrapper;
    return ret;

    /* failure */
    free_surface:
    kfree(wrapper->surface);
    deinit:
    display_deinit(display);
    return ret;
}

static int prepare_framebuffer(struct device *device, struct fb_wrapper **info) {
    int ret = 0;
    struct fb_wrapper *fb_wrapper;
    struct fb_info *fbinfo;
    debug();
    /* alloc framebuffer */
    fbinfo = framebuffer_alloc(sizeof(struct fb_wrapper), device);
    if (!fbinfo)
        return -ENOMEM;
    fb_wrapper = fbinfo->par;
    fb_wrapper->fb = fbinfo;
    /* init framebuffer info */
    fbinfo->var = ssd1306_fb_var;
    fbinfo->fix = ssd1306_fb_fix;
    fbinfo->fbops = &ssd1306_fbops;
    fbinfo->fbdefio = &ssd1306_defio;
    fb_deferred_io_init(fbinfo);
    /* register framebuffer */
    ret = register_framebuffer(fbinfo);
    if (ret == 0)
        *info = fb_wrapper;
    return ret;
}

int display_driver_probe(struct device *device, struct display *display) {
    int ret = 0;
    unsigned fb_id;
    size_t vmem_len;
    unsigned char *vmem;
    struct fb_wrapper *fb_wrapper;
    struct display_wrapper *disp_wrapper;
    debug();
    /* init display*/
    ret = prepare_display(display, &disp_wrapper);
    if (ret != 0) {
        dev_dbg(device, "init display failed\n");
        goto free_mem;
    }
    /* add to list */
    INIT_LIST_HEAD(&disp_wrapper->list);
    mutex_lock(&list_locker);
    list_add_tail(&disp_wrapper->list, &display_list);
    mutex_unlock(&list_locker);
    /*
    * NOTICE: multi-display
    * if defined multi display, create 1 fb instance
    * if defined divide display, create several fb instance
    */
#ifdef DISPLAY_MULTI
    fb_id = display->id / DISPLAY_MULTI;
    if (list_empty(&fb_list) || list_last_entry(&fb_list, struct fb_wrapper, list)->id < fb_id) {
        /* alloc video memory*/
        vmem_len = DISPLAY_MAX_SIZE;
        vmem = vzalloc(vmem_len);
        if (!vmem) {
            dev_dbg(device, "alloc video memory failed\n");
            return -ENOMEM;
        }
        /* create fb */
        //fb_wrapper = kzalloc(sizeof(struct fb_wrapper), GFP_KERNEL);
        //if (!fb_wrapper)
        //    return -ENOMEM;
        ret = prepare_framebuffer(device, &fb_wrapper);
        if (ret != 0) {
            pr_err("prepare framebuffer failed\n");
            goto free_mem;
        }
        INIT_LIST_HEAD(&fb_wrapper->list);
        fb_wrapper->fb->screen_buffer = vmem;
        fb_wrapper->fb->fix.smem_len = vmem_len;
        fb_wrapper->fb->fix.smem_start = (unsigned long) vmem;
        fb_wrapper->id = fb_id;
        mutex_lock(&list_locker);
        list_add_tail(&fb_wrapper->list, &fb_list);
        mutex_unlock(&list_locker);
    } else {
        fb_wrapper = list_last_entry(&fb_list, struct fb_wrapper, list);
    }
    /* link fb & display */
    disp_wrapper->sub_id = display->id % DISPLAY_MULTI;
    disp_wrapper->fbinfo = fb_wrapper->fb;
    fb_wrapper->displays[display->id] = disp_wrapper;
    ssd1306_fb_check_var(&fb_wrapper->fb->var, fb_wrapper->fb);
#endif

#ifdef DISPLAY_DIV
#endif

#ifdef DEBUG
    mutex_lock(&list_locker);
    list_for_each_entry(disp_wrapper, &display_list, list) {
        debug("display[%u,%u] <---> fb[%d]",
              disp_wrapper->display->id,
              disp_wrapper->sub_id,
              disp_wrapper->fbinfo->node);
    }
    mutex_unlock(&list_locker);
#endif
    debug("end");
    return ret;

    /* failure */
    free_mem:
    vfree(vmem);
    return ret;
}

void dislay_driver_remove(struct display *display) {
    u8 id = display->id;
    struct display_wrapper *wrapper = NULL, *next = NULL;
    debug();
    mutex_lock(&list_locker);
    list_for_each_entry_safe(wrapper, next, &display_list, list) {
        if (wrapper->display->id != id)
            continue;
        debug("remove display[%u]", id);
        kfree(wrapper->roi);
        kfree(wrapper->surface);
        list_del(&wrapper->list);
        clear_bit(id, display_ids);
    }
    mutex_unlock(&list_locker);
    display_deinit(display);
}

// module methods
static int __init ssd1306_init(void) {
    int ret = 0;
    mutex_init(&list_locker);
    ret = display_driver_spi_init();
    //ret = display_driver_i2c_init();
    return ret;
}

static void __exit ssd1306_exit(void) {
    struct fb_info *info;
    struct fb_wrapper *fb_wrapper, *next;
    display_driver_spi_exit();
    //display_driver_i2c_exit();
    mutex_lock(&list_locker);
    list_for_each_entry_safe(fb_wrapper, next, &fb_list, list) {
        info = fb_wrapper->fb;
        debug("unregister fb[%d]", info->node);
        fb_deferred_io_cleanup(info);
        unregister_framebuffer(info);
        list_del(&fb_wrapper->list);
        framebuffer_release(info);
    }
    mutex_unlock(&list_locker);
    if (!list_empty(&display_list))
        pr_err("list should be empty!");
    mutex_destroy(&list_locker);
}

module_init(ssd1306_init)
module_exit(ssd1306_exit)
MODULE_LICENSE("GPL");
MODULE_AUTHOR("mumumusuc@gmail.com");