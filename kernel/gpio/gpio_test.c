//
// Created by mumumusuc on 19-1-31.
//

#include "gpio_test.h"
//#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>


int main(int argc, char *argv[]) {
    int fd = open(gpio_node, O_RDWR);
    if (fd < 0) {
        perror("file open error.\n");
        exit(-1);
    }
    gpio_info_t info = {
            .pin = 21,
            .mode = -1,
    };
    if (ioctl(fd, GPIO_IOC_GET_MODE, &info)) {
        perror("ioctl : ");
        exit(-2);
    }
    printf("read mode : pin = %d , mode = %d \n", info.pin, info.mode);
    info.mode = GPIO_MODE_OUTPUT;
    if (ioctl(fd, GPIO_IOC_SET_MODE, &info)) {
        perror("ioctl : ");
        exit(-2);
    }
    info.mode = -1;
    if (ioctl(fd, GPIO_IOC_GET_MODE, &info)) {
        perror("ioctl : ");
        exit(-2);
    }
    printf("read mode : pin = %d , mode = %d \n", info.pin, info.mode);

    uint8_t buffer[] = {0, info.pin, 0};
    size_t cnt = 5;
    while (cnt--) {/*
        info.value = GPIO_HIGH;
        if (ioctl(fd, GPIO_IOC_SET_VALUE, &info)) {
            perror("ioctl : ");
            exit(-3);
        }
        usleep(500 * 1000);
        info.value = GPIO_LOW;
        if (ioctl(fd, GPIO_IOC_SET_VALUE, &info)) {
            perror("ioctl : ");
            exit(-3);
        }
        usleep(500 * 1000);*/
        buffer[2] = GPIO_HIGH;
        write(fd, buffer, 3);
        usleep(500 * 1000);
        buffer[2] = GPIO_LOW;
        write(fd, buffer, 3);
        usleep(500 * 1000);
    }
    return 0;
}