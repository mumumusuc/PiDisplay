//
// Created by mumumusuc on 19-2-2.
//
/*
DS18B20控制方法（DS18B20有六条控制命令）：
温度转换 44H 启动DS18B20进行温度转换
读暂存器 BEH 读暂存器9位二进制数字
写暂存器 4EH 将数据写入暂存器的TH、TL字节
复制暂存器 48H 把暂存器的TH、TL字节写到E2RAM中
重新调E2RAM B8H 把E2RAM中的TH、TL字节写到暂存器TH、TL字节
读电源供电方式 B4H 启动DS18B20发送电源供电方式的信号给主CPU
 */

#ifndef PI_DISPLAY_DS18B20_H
#define PI_DISPLAY_DS18B20_H

#include <stdint.h>

void ds18b20_init(void);

void ds18b20_deinit(void);

uint8_t ds18b20_reset(void);

void ds18b20_write(uint8_t data);

uint8_t ds18b20_read(void);

uint8_t ds18b20_convert(void);

int16_t ds18b20_read_temp(void);

#endif //PI_DISPLAY_DS18B20_H
