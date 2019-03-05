//
// Created by mumumusuc on 19-2-9.
//

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <sys/time.h>
#include <linux/fb.h>
#include <signal.h>
#include <sys/mman.h>
#include "ssd1306_user.h"


static const char *fb_node = "/dev/fb1";
static int interrupted = 0;
static size_t width;
static size_t height;
static size_t size;
static uint8_t *fb_mem = MAP_FAILED;
static uint8_t block_size = 32;

static void clean_up(int sig) {
    interrupted = 1;
    if (fb_mem != MAP_FAILED && size > 0) {
        munmap(fb_mem, size);
    }
}

int main(int argc, char *argv[]) {
    int fd = open(fb_node, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "open %d failed\n", fd);
        return -1;
    }
    printf("open %d success \n", fd);

    if (ioctl(fd, FBIOBLANK, FB_BLANK_NORMAL) < 0) {
        perror("ioctl FBIOBLANK");
    }
    //if (ioctl(fd, FBIOBLANK, FB_BLANK_UNBLANK) < 0) {
    //    perror("ioctl FBIOBLANK");
    //}

    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo) < 0) {
        perror("ioctl FBIOGET_FSCREENINFO");
        return -1;
    }
    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        perror("ioctl FBIOGET_VSCREENINFO");
        return -1;
    }
    width = vinfo.xres_virtual;
    height = vinfo.yres_virtual;
    size = width * height * vinfo.bits_per_pixel / 8;
    printf("w = %d, h = %d, s = %d, ox = %d, oy = %d\n", width, height, size, vinfo.xoffset, vinfo.yoffset);

    vinfo.xres = width / 2;
    //vinfo.yres /= 2;
    vinfo.xoffset = width / 4;
    //vinfo.yoffset = 22;
    if (ioctl(fd, FBIOPUT_VSCREENINFO, &vinfo) < 0) {
        perror("ioctl FBIOPUT_VSCREENINFO");
    } else {
        width = vinfo.xres;
        height = vinfo.yres;
        size = width * height * vinfo.bits_per_pixel / 8;
        printf("w = %d, h = %d, s = %d, ox = %d, oy = %d\n", width, height, size, vinfo.xoffset, vinfo.yoffset);
    }

    sleep(1);
/*
    if (ioctl(fd, FBIOBLANK, FB_BLANK_UNBLANK) < 0) {
        perror("ioctl FBIOBLANK");
    }
*/

    fb_mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (fb_mem == MAP_FAILED) {
        perror("mmap failed\n");
    } else {
        //uint8_t value = 1;
        size_t line_size = width * vinfo.bits_per_pixel / 8;
        size_t set_size = line_size, clear_size = 0;
        block_size = height/3;
        while (!interrupted) {
            memset(fb_mem+set_size, 1, set_size);
            memset(fb_mem, 0, clear_size);

            set_size += line_size;
            if (set_size >= size) {
                set_size = 0;
                //value = 1 - value;
            }
            if (abs(set_size - clear_size) > block_size * line_size ||
                clear_size >= (height - block_size) * line_size) {
                clear_size += line_size;
            }
            if (clear_size >= size) {
                clear_size = 0;
                //value = 1 - value;
            }
            usleep(10 * 1000);
        }
    }
    clean_up(0);
    close(fd);
    return 0;
}