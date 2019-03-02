
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fb.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include "ssd1306_dev.h"

#define SINGLE_FB
#define DISPLAY_FPS         60

#ifdef SINGLE_FB
#define DISPLAY_NUM_H       2
#define DISPLAY_NUM_V       1
#else
#define DISPLAY_NUM_H       1
#define DISPLAY_NUM_V       1
#endif

#define DISPLAY_DEPTH_1     1
#define DISPLAY_DEPTH_8     8
#define DISPLAY_MAX_DEPTH   DISPLAY_DEPTH_8
#define DISPLAY_MAX_WIDTH   (DISPLAY_WIDTH*DISPLAY_NUM_H)
#define DISPLAY_MAX_HEIGHT  (DISPLAY_HEIGHT*DISPLAY_NUM_V)
#define DISPLAY_MAX_LINE    (DISPLAY_MAX_WIDTH*DISPLAY_MAX_DEPTH/8)
#define DISPLAY_MAX_SIZE    (DISPLAY_MAX_HEIGHT*DISPLAY_MAX_LINE)

static bool config_spi = true;

module_param(config_spi, bool, S_IRUGO);
static bool config_i2c = false;

module_param(config_i2c, bool, S_IRUGO);

struct fb_dev {
    struct fb_info *fb;
    struct list_head list;
};

struct dirty_area {
    size_t dx;
    size_t dy;
    size_t w;
    size_t h;
    size_t offset;
};

struct display_head {
    unsigned users;             /* how many users opened this node */
    struct surface surface;
    struct dirty_area dirty;
    struct mutex lock;
    struct list_head list;
    struct backlight_device *bl_dev;
};

struct display_node {
    unsigned id;                /* witch display we are using */
    struct display *display;    /* display instance */
    struct list_head list;
};

/* backlight ops */
int bl_update_status(struct backlight_device *bl_dev) {
    struct display_head *hook = bl_get_data(bl_dev);
    struct display_node *node;
    int brightness = bl_dev->props.brightness;
    debug("set brightness[%d]", brightness);
    mutex_lock(&hook->lock);
    list_for_each_entry(node, &hook->list, list) {
        display_set_option(node->display, &(struct option) {.brightness = brightness});
    }
    mutex_unlock(&hook->lock);
    return 0;
}

int bl_get_brightness(struct backlight_device *bl_dev) {
    int brightness = bl_dev->props.brightness;
    debug("read brightness[%d]", brightness);
    return brightness;
}

/*
int bl_check_fb(struct backlight_device *bl_dev, struct fb_info *info) {
    debug();
    return true;
}
*/
static struct backlight_ops display_bl_ops = {
        .update_status  = bl_update_status,
        .get_brightness = bl_get_brightness,
        //.check_fb       = bl_check_fb,
};

static struct backlight_properties display_bl_prop = {
        .type = BACKLIGHT_RAW,
        .brightness = DEFAULT_BRIGHTNESS,
        .max_brightness = MAX_BRIGHTNESS,
};

/* how many framebuffers */
static LIST_HEAD(fb_list);
static DEFINE_MUTEX(fb_list_lock);

/* fb ops */
static int display_fb_open(struct fb_info *, int);

static int display_fb_release(struct fb_info *, int);

static int display_fb_blank(int, struct fb_info *);

static int display_fb_check_var(struct fb_var_screeninfo *, struct fb_info *);

static void display_fb_fillrect(struct fb_info *, const struct fb_fillrect *);

static void display_fb_copyarea(struct fb_info *, const struct fb_copyarea *);

static void display_fb_imageblit(struct fb_info *, const struct fb_image *);

static void display_fb_deferred_io(struct fb_info *, struct list_head *);

static ssize_t display_fb_write(struct fb_info *, const char __user *, size_t, loff_t *);


/* fb structs */
static struct fb_fix_screeninfo display_fb_fix = {
        .id             = DISPLAY_MODULE,
        .type           = FB_TYPE_PACKED_PIXELS,
        .visual         = FB_VISUAL_MONO10,
        .accel          = FB_ACCEL_NONE,
        .line_length    = DISPLAY_MAX_LINE,
};
static struct fb_var_screeninfo display_fb_var = {
        .bits_per_pixel = DISPLAY_DEPTH_8,
        .xres           = DISPLAY_MAX_WIDTH,
        .yres           = DISPLAY_MAX_HEIGHT,
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
static struct fb_deferred_io display_defio = {
        .delay          = HZ / DISPLAY_FPS,
        .deferred_io    = display_fb_deferred_io,
};
static struct fb_ops display_fbops = {
        .owner          = THIS_MODULE,
        .fb_fillrect    = display_fb_fillrect,
        .fb_copyarea    = display_fb_copyarea,
        .fb_imageblit   = display_fb_imageblit,
        .fb_write       = display_fb_write,
        .fb_blank       = display_fb_blank,
        .fb_open        = display_fb_open,
        .fb_release     = display_fb_release,
        .fb_check_var   = display_fb_check_var,
};

static __always_inline void make_dirty(struct fb_info *info, ssize_t dx, ssize_t dy, ssize_t width, ssize_t height) {
    struct display_head *hook = info->par;
    hook->dirty.dx = dx < 0 ? 0 : dx;
    hook->dirty.dy = dy < 0 ? 0 : dy;
    hook->dirty.w = width < 0 ? hook->surface.width : width;
    hook->dirty.h = height < 0 ? hook->surface.height : height;
    hook->dirty.offset = (hook->dirty.dx + hook->dirty.dy * hook->surface.line_length) * hook->surface.depth / 8;
    hook->surface.data = info->screen_buffer + hook->dirty.offset;
}

static __always_inline void make_clean(struct fb_info *info) {
    struct display_head *hook = info->par;
    hook->dirty.dx = 0;
    hook->dirty.dy = 0;
    hook->dirty.w = hook->surface.width;
    hook->dirty.h = hook->surface.height;
    hook->dirty.offset = 0;
    hook->surface.data = info->screen_buffer;
}

static int update_roi(struct fb_info *info, size_t dx, size_t dy, size_t width, size_t height) {
    int ret, index = 0;
    struct display_head *hook = info->par;
    struct display_node *node;
    dx += info->var.xoffset;
    dy += info->var.yoffset;
#ifdef SINGLE_FB
    list_for_each_entry(node, &hook->list, list) {
#else
        node = list_first_entry(&hook->list, struct display_node, list);
#endif
        ret = display_set_roi(
                node->display,
                dx - index * DISPLAY_WIDTH,
                dy,
                width,
                height,
                index * (dx < DISPLAY_WIDTH ? (DISPLAY_WIDTH - dx) : 0)
        );
#ifdef SINGLE_FB
        index++;
    }
#endif
    return ret;
}

static __always_inline void update_display_now(struct fb_info *info) {
    struct display_head *hook = info->par;
    struct display_node *node;
    mutex_lock(&hook->lock);
    update_roi(info, hook->dirty.dx, hook->dirty.dy, hook->dirty.w, hook->dirty.h);
    list_for_each_entry(node, &hook->list, list) {
        display_update(node->display, &hook->surface, &node->display->roi);
    }
    make_clean(info);
    mutex_unlock(&hook->lock);
}

static __always_inline void update_display(struct fb_info *info) {
    schedule_delayed_work(&info->deferred_work, info->fbdefio->delay);
}

static int display_fb_open(struct fb_info *info, int user) {
    struct display_head *hook = info->par;
    debug();
    mutex_lock(&hook->lock);
    hook->users++;
    mutex_unlock(&hook->lock);
    if (likely(hook->users == 1)) {
        struct display_node *node;
        info->fbops->fb_check_var(&display_fb_var, info);
        memset(info->screen_buffer, 0, info->screen_size);
        mutex_lock(&hook->lock);
        list_for_each_entry(node, &hook->list, list) {
            display_turn_on(node->display);
            display_clear(node->display);
        }
        mutex_unlock(&hook->lock);
        backlight_device_set_brightness(hook->bl_dev, DEFAULT_BRIGHTNESS);
    }
    return 0;
}

static int display_fb_release(struct fb_info *info, int user) {
    struct display_head *hook = info->par;
    debug();
    mutex_lock(&hook->lock);
    hook->users--;
    mutex_unlock(&hook->lock);
    return 0;
}

static ssize_t display_fb_write(struct fb_info *info, const char __user *buf, size_t count, loff_t *ppos) {
    ssize_t ret;
    ret = fb_sys_write(info, buf, count, ppos);
    update_display(info);
    return ret;
}

static int display_fb_blank(int blank, struct fb_info *info) {
    struct display_head *hook = info->par;
    //struct display_node *node;
    debug();
    switch (blank) {
        case FB_BLANK_NORMAL:
            backlight_device_set_brightness(hook->bl_dev, DEFAULT_BRIGHTNESS);
            break;
        case FB_BLANK_UNBLANK:
            break;
        case FB_BLANK_VSYNC_SUSPEND:
        case FB_BLANK_HSYNC_SUSPEND:
        case FB_BLANK_POWERDOWN:
            backlight_device_set_brightness(hook->bl_dev, 0);
            break;
        default:
            break;
    }
    return 0;
}


static void display_fb_fillrect(struct fb_info *info, const struct fb_fillrect *rect) {
    //debug("dx=%d, dy=%d, width=%d, height=%d", rect->dx, rect->dy, rect->width, rect->height);
    sys_fillrect(info, rect);
    make_dirty(info, rect->dx, rect->dy, rect->width, rect->height);
    //update_display(info);
    update_display_now(info);
}

static void display_fb_copyarea(struct fb_info *info, const struct fb_copyarea *area) {
    //debug("dx=%d, dy=%d, width=%d, height=%d", area->dx, area->dy, area->width, area->height);
    sys_copyarea(info, area);
    make_dirty(info, area->dx, area->dy, area->width, area->height);
    //update_display(info);
    update_display_now(info);
}

static void display_fb_imageblit(struct fb_info *info, const struct fb_image *image) {
    //debug("dx=%d, dy=%d, width=%d, height=%d", image->dx, image->dy, image->width, image->height);
    sys_imageblit(info, image);
    make_dirty(info, image->dx, image->dy, image->width, image->height);
    //update_display(info);
    update_display_now(info);
}

static void display_fb_deferred_io(struct fb_info *info, struct list_head *pagelist) {
    unsigned int dirty_lines_start, dirty_lines_end;
    struct display_head *hook = info->par;
    struct page *page;
    unsigned long index;
    unsigned int y_low = 0, y_high = 0;
    int count = 0;
    dirty_lines_start = hook->dirty.dy;
    dirty_lines_end = hook->dirty.dy + hook->dirty.h - 1;
    list_for_each_entry(page, pagelist, lru) {
        count++;
        index = page->index << PAGE_SHIFT;
        y_low = index / info->fix.line_length;
        y_high = (index + PAGE_SIZE - 1) / info->fix.line_length;
        //debug("index=%lu y_low=%d y_high=%d", index, y_low, y_high);
        if (y_high > hook->surface.height - 1)
            y_high = hook->surface.height - 1;
        if (y_low < dirty_lines_start)
            dirty_lines_start = y_low;
        if (y_high > dirty_lines_end)
            dirty_lines_end = y_high;
    }
    make_dirty(info, -1, dirty_lines_start, -1, dirty_lines_start - dirty_lines_end + 1);
    update_display_now(info);
}

static int display_fb_check_var(struct fb_var_screeninfo *var, struct fb_info *info) {
    int ret = 0;
    size_t bpp, width, height, xoffset, yoffset;
    struct display_head *hook = info->par;
    debug();
    if (unlikely(list_empty(&hook->list)))
        return -ENODEV;
    bpp = var->bits_per_pixel;
    if (bpp != DISPLAY_DEPTH_1 && bpp != DISPLAY_DEPTH_8)
        return -EINVAL;
    width = var->xres;
    height = var->yres;
    xoffset = var->xoffset;
    yoffset = var->yoffset;
    if (width > DISPLAY_MAX_WIDTH || height > DISPLAY_MAX_HEIGHT)
        return -EINVAL;
    mutex_lock(&hook->lock);
    if (hook->users > 1) {
        ret = -EBUSY;
        debug("current user[%u], busy", hook->users);
        goto done;
    }
    hook->surface.data = info->screen_buffer;
    hook->surface.depth = bpp;
    hook->surface.width = width;
    hook->surface.height = height;
    hook->surface.line_length = width;

    info->var.bits_per_pixel = bpp;
    info->var.xoffset = xoffset;
    info->var.yoffset = yoffset;
    info->var.xres = width;
    info->var.yres = height;
    info->fix.line_length = width;
    goto done;

    done:
    mutex_unlock(&hook->lock);
    return ret;
}

static int prepare_fb(struct device *device, struct fb_dev **dev) {
    int ret = 0;
    struct fb_info *info;
    struct fb_dev *fbdev;
    struct display_head *hook;
    debug();
    /* alloc framebuffer */
    info = framebuffer_alloc(sizeof(struct display_head), device);
    if (!info)
        return -ENOMEM;
    hook = info->par;
    INIT_LIST_HEAD(&hook->list);
    mutex_init(&hook->lock);
    /* alloc fb wrapper */
    fbdev = kzalloc(sizeof(struct fb_dev), GFP_KERNEL);
    if (!fbdev) {
        ret = -ENOMEM;
        goto free_fb;
    }
    INIT_LIST_HEAD(&fbdev->list);
    fbdev->fb = info;
    /* init fb info */
    info->var = display_fb_var;
    info->fix = display_fb_fix;
    info->flags = FBINFO_FLAG_DEFAULT | FBINFO_VIRTFB;
    info->fbops = &display_fbops;
    info->fbdefio = &display_defio;
    fb_deferred_io_init(info);
    /* register framebuffer */
    ret = register_framebuffer(info);
    if (ret != 0)
        goto free_all;
    /* add to fb list */
    mutex_lock(&fb_list_lock);
    list_add_tail(&fbdev->list, &fb_list);
    mutex_unlock(&fb_list_lock);
    *dev = fbdev;
    return ret;

    free_all:
    fb_deferred_io_cleanup(info);
    kfree(fbdev);
    free_fb:
    framebuffer_release(info);
    return ret;
}

int display_driver_probe(struct device *device, struct display *display) {
    int ret = 0;
    u8 *vmem;
    size_t vmem_size;
    char bl_name[16];
    struct fb_dev *dev = NULL;
    struct display_head *hook;
    struct display_node *node;
    struct backlight_device *bl_dev = NULL;
    debug();
    node = kzalloc(sizeof(struct display_node), GFP_KERNEL);
    if (!node)
        return -ENOMEM;
    node->id = display->id;
    node->display = display;
    INIT_LIST_HEAD(&node->list);
#ifdef SINGLE_FB
    if (list_empty(&fb_list)) {
#endif
        vmem_size = DISPLAY_MAX_SIZE;
        vmem = vzalloc(vmem_size);
        debug("alloc vmem %u bytes", vmem_size);
        if (!vmem) {
            ret = -ENOMEM;
            goto free_node;
        }
        ret = prepare_fb(device, &dev);
        if (ret < 0)
            goto free_mem;
        dev->fb->screen_buffer = vmem;
        dev->fb->screen_size = vmem_size;
        dev->fb->fix.smem_start = (unsigned long) vmem;
        dev->fb->fix.smem_len = vmem_size;
        dev->fb->fbops->fb_check_var(&dev->fb->var, dev->fb);
        sprintf(bl_name, "bl-fb%d", dev->fb->node);
        bl_dev = backlight_device_register(bl_name, dev->fb->dev, dev->fb->par, &display_bl_ops, &display_bl_prop);
        if (IS_ERR_OR_NULL(bl_dev))
            pr_err("register backlight device failed\n");
        hook = dev->fb->par;
        hook->bl_dev = bl_dev;
#ifdef SINGLE_FB
    }
#endif
    mutex_lock(&fb_list_lock);
    dev = list_last_entry(&fb_list, struct fb_dev, list);
    mutex_unlock(&fb_list_lock);
    hook = dev->fb->par;
    mutex_lock(&hook->lock);
    list_add_tail(&node->list, &hook->list);
    mutex_unlock(&hook->lock);
    return ret;

    /* failure */
    free_mem:
    vfree(vmem);

    free_node:
    kfree(node);
    return ret;
}

void display_driver_remove(struct display *display) {
    unsigned id = display->id;
    struct fb_dev *fbdev, *fbnext;
    struct display_head *hook;
    struct display_node *node, *next;
    debug();
    mutex_lock(&fb_list_lock);
    list_for_each_entry_safe(fbdev, fbnext, &fb_list, list) {
        debug("check fbdev[%d]", fbdev->fb->node);
        hook = fbdev->fb->par;
        mutex_lock(&hook->lock);
        list_for_each_entry_safe(node, next, &hook->list, list) {
            debug("check display_node[%d]", node->id);
            if (id == node->id) {
                list_del(&node->list);
                kfree(node);
            }
        }
        mutex_unlock(&hook->lock);
        if (list_empty(&hook->list)) {
            debug("del fbdev[%d]", fbdev->fb->node);
            list_del(&fbdev->list);
            if (hook->bl_dev) {
                debug("unregister backlight");
                backlight_device_unregister(hook->bl_dev);
            }
            mutex_destroy(&hook->lock);
            fb_deferred_io_cleanup(fbdev->fb);
            unregister_framebuffer(fbdev->fb);
            framebuffer_release(fbdev->fb);
        }
    }
    mutex_unlock(&fb_list_lock);
}

// module methods
static int __init display_fb_init(void) {
    int ret;
    if (config_spi)
        ret = display_driver_spi_init();
    if (config_i2c)
        ret = display_driver_i2c_init();
    return ret;
}

static void __exit display_fb_exit(void) {
    if (config_spi)
        display_driver_spi_exit();
    if (config_i2c)
        display_driver_i2c_exit();
    if (unlikely(!list_empty(&fb_list))) {
        struct fb_dev *fbdev;
        struct display_head *hook;
        struct display_node *node;
        pr_err("fb list not empty, memory leak occured.");
        list_for_each_entry(fbdev, &fb_list, list) {
            hook = fbdev->fb->par;
            pr_err("fbdev[%d] not deleted.", fbdev->fb->node);
            list_for_each_entry(node, &hook->list, list) {
                pr_err("display_node[%d] not deleted.", node->id);
            }
        }
    }
    mutex_destroy(&fb_list_lock);
}

module_init(display_fb_init)
module_exit(display_fb_exit)
MODULE_LICENSE("GPL");
MODULE_AUTHOR("mumumusuc@gmail.com");