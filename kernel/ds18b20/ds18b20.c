//
// Created by mumumusuc on 19-2-2.
//

#include "ds18b20.h"
#include "gpio_test.h"
#include <zconf.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>

#define OK      (0)
#define ERR     (-1)
#define LOW     GPIO_LOW
#define HI      GPIO_HIGH
#define PIN_DQ  (21)
#define MODE(flag)                                      \
do{                                                     \
        info.mode = (flag);                             \
        if(ioctl(fd, GPIO_IOC_SET_MODE, &info)){        \
            perror("ds18b20 gpio set mode failed.\n");  \
            exit(-1);                                   \
        }                                               \
}while(0)
#define DQW(n)                                          \
do{                                                     \
    if(mode_flag != GPIO_MODE_OUTPUT){                  \
        MODE(GPIO_MODE_OUTPUT);                         \
        mode_flag = GPIO_MODE_OUTPUT;                   \
    }                                                   \
    info.value=(n);                                     \
    if(ioctl(fd, GPIO_IOC_SET_VALUE, &info)){           \
        perror("ds18b20 gpio set value failed.\n");     \
        exit(-1);                                       \
    };                                                  \
}while(0)
#define DQR()                                           \
({                                                      \
    if(mode_flag != GPIO_MODE_INPUT){                   \
        MODE(GPIO_MODE_OUTPUT);                         \
        mode_flag = GPIO_MODE_INPUT;                    \
    };                                                  \
    if(ioctl(fd, GPIO_IOC_GET_VALUE, &info)){           \
            perror("ds18b20 gpio get value failed.\n"); \
            exit(-1);                                   \
    };                                                  \
    info.value;                                         \
})

static int fd = -1;
static int mode_flag = GPIO_MODE_OUTPUT;
static gpio_info_t info = {
        .pin = PIN_DQ,
        .mode = GPIO_MODE_OUTPUT,
};

inline void ds18b20_init(void) {
    fd = open(gpio_node, O_RDWR);
    if (fd < 0) {
        perror("ds18b20 gpio init failed.\n");
        exit(-1);
    }
    if (ioctl(fd, GPIO_IOC_SET_MODE, &info)) {
        perror("ds18b20 gpio set mode failed.\n");
        exit(-1);
    }
}

inline void ds18b20_deinit(void) {
    if (fd < 0) {
        perror("ds18b20 gpio deinit failed.\n");
        return;
    }
    close(fd);
}


inline uint8_t ds18b20_reset(void) {
    printf("[%s] begin.\n", __func__);
    DQW(LOW);
    usleep(750);                //480 ~ 960us
    DQW(HI);
    usleep(20);                 //15 ~ 60us
    while (DQR() == LOW);
    DQW(1);
    printf("[%s] end.\n", __func__);
    return OK;
}

inline void ds18b20_write(uint8_t data) {
    printf("[%s] begin.\n", __func__);
    uint8_t mask;
    for (mask = 0x01; mask != 0; mask <<= 1) {
        DQW(LOW);
        usleep(2);
        if ((mask & data) == 0)
            DQW(LOW);
        else
            DQW(HI);
        usleep(60);     // 15 ~ 60us
        DQW(HI);
        usleep(2);
    }
    printf("[%s] end.\n", __func__);
}


inline uint8_t ds18b20_read(void) {
    printf("[%s] begin.\n", __func__);
    uint8_t data = 0, mask;
    for (mask = 0x01; mask != 0; mask <<= 1) {
        DQW(LOW);
        usleep(2);
        DQW(HI);
        usleep(12);
        if (DQR() == LOW)
            data &= ~mask;
        else
            data |= mask;
        usleep(50);
        DQW(HI);
        usleep(1);
    }
    printf("[%s] end.\n", __func__);
    return data;
}

inline uint8_t ds18b20_convert(void) {
    if (ds18b20_reset() == OK) {
        printf("[%s] begin.\n", __func__);
        usleep(1000);
        ds18b20_write(0xCC);        //skip rom
        ds18b20_write(0x44);        //convert
        printf("[%s] end.\n", __func__);
        return OK;
    }
    return ERR;
}


inline int16_t ds18b20_read_temp(void) {
    printf("[%s] end.\n", __func__);
    uint8_t lsb, msb;
    int16_t tmp = 0xffff;
    if (ds18b20_reset() == 0) {
        usleep(1000);
        ds18b20_write(0xCC);  //skip rom
        ds18b20_write(0xBE);  //read scratchpad
        lsb = ds18b20_read();  //读出第一个字节暂存于LSB
        msb = ds18b20_read();  //读出第二个字节暂存于MSB
        tmp = (msb << 8) | lsb;
    }
    printf("[%s] end.(%x,%x)\n", __func__, msb, lsb);
    return tmp;
}

static int check() {
    size_t retry = 0;
    while (DQR() && retry < 200) {
        retry++;
        usleep(1);
    }
    if (retry >= 200) return ERR;
    retry = 0;
    while (!DQR() && retry < 240) {
        retry++;
        usleep(1);
    }
    if (retry >= 240) return ERR;
    return OK;
}

int main(int argc, char *argv[]) {
    ds18b20_init();
    ds18b20_reset();
    ds18b20_write(0xCC);
    ds18b20_write(0x44);
    ds18b20_reset();
    if (ERR == check()) {
        perror("ds18b20 not detected.\n");
        exit(-1);
    }
    int16_t temp;
    float result;
    usleep(10 * 1000);
    ds18b20_convert();
    sleep(1);
    temp = ds18b20_read_temp();
    result = 0.0625 * temp;
    printf("read temp = %.1f \n", result);
    ds18b20_deinit();
    return 0;
}