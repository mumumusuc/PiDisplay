//
// Created by mumumusuc on 19-2-9.
//

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <sys/time.h>
#include <signal.h>
#include "ssd1306_user.h"

static const char *node = "/dev/ssd1306_spi4";
static int fd = -1;
static int interrupted = 0;

static void clean_up(int sig) {
    interrupted = 1;
    if (fd >= 0) {
        close(fd);
    }
}

int main(int argc, char *argv[]) {
    fd = open(node, O_WRONLY);
    if (fd < 0) {
        perror("node open failed.\n");
        exit(1);
    }
    signal(SIGINT, clean_up);
    display_info_t info;
    if (ioctl(fd, SSD_IOC_INFO, &info)) {
        perror("get info error.\n");
        exit(1);
    }
    const size_t size = info.size;
    uint8_t screen[size];
    printf("buffer sizer = %d.\n", size);

    if (ioctl(fd, SSD_IOC_RSET)) {
        perror("reset error.\n");
        exit(1);
    }
    if (ioctl(fd, SSD_IOC_ON)) {
        perror("turn on error.\n");
        exit(1);
    }

    struct timeval begin;
    struct timeval end;
    uint32_t time;
    uint8_t value = 0xFF;
    while (!interrupted) {
        gettimeofday(&begin, NULL);
        if (value > 0)
            value >>= 1;
        else
            value = 0xFF;
        memset(screen, value, size);
        write(fd, screen, size);
        //ioctl(fd, SSD_IOC_DISPLAY, screen);
        gettimeofday(&end, NULL);
        time = 1000000 * (end.tv_sec - begin.tv_sec) + (end.tv_usec - begin.tv_usec);
        printf("fps = %2.1f .\n", 1000000.f / time);
    }

    return 0;
}