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

static const char *node = "/dev/ssd1306_spi4";
static const char *fb_node = "/dev/fb1";
static char *file = "../BadApple.mp4";
static int interrupted = 0;
static size_t width;
static size_t height;
static size_t size;
static uint8_t *fb_mem = MAP_FAILED;

static void clean_up(int sig) {
    interrupted = 1;
    if (fb_mem != MAP_FAILED && size > 0) {
        munmap(fb_mem, size);
    }
}

int main(int argc, char *argv[]) {
    int ret = 0;
    int fd = open(fb_node, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "open %d failed\n", fd);
        return -1;
    }
    printf("open %d success \n", fd);
    close(fd);
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    ret = ioctl(fd, FBIOGET_FSCREENINFO, &finfo);
    if (ret < 0) {
        perror("ioctl FBIOGET_FSCREENINFO\n");
        return -1;
    }
    ret = ioctl(fd, FBIOGET_VSCREENINFO, &vinfo);
    if (ret < 0) {
        perror("ioctl FBIOGET_VSCREENINFO\n");
        return -1;
    }

    width = vinfo.xres_virtual;
    height = vinfo.yres_virtual;
    size = finfo.smem_len;
    printf("w = %d, h = %d, s = %d\n", width, height, size);
    fb_mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (fb_mem == MAP_FAILED) {
        perror("mmap failed\n");

    }

    if (argc > 1) {
        file = argv[1];
    }



    clean_up(0);
    return 0;
}