//
// Created by mumumusuc on 19-1-25.
//

#ifndef PI_DISPLAY_SPI_H
#define PI_DISPLAY_SPI_H

#include "object.h"
#include "gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SPI_MODE_0 (0|0)//SCLK空闲时为低电平，第一个时间沿采样
#define SPI_MODE_1 (0|SPI_CPHA)//SCLK空闲时为高电平，第一个时间沿采样
#define SPI_MODE_2 (SPI_CPOL|0)//SCLK空闲时为低电平，第二个时间沿采样
#define SPI_MODE_3 (SPI_CPOL|SPI_CPHA)//SCLK空闲时为高电平，第二个时间沿采样

// define spi
typedef struct _Spi_VTbl SpiVTbl;

typedef struct _SpiInfo {
    uint8_t mode;
    uint8_t cs;
    uint32_t speed;
} SpiInfo;

typedef struct _Spi {
    SpiVTbl *vtbl;
    Object *obj;
} Spi;

Spi *new_spi();

void spi_begin(Spi *,Gpio*);

void spi_init(Spi *, const SpiInfo *);

void spi_write(Spi *, const uint8_t *, size_t);

void spi_read(Spi *, uint8_t *, size_t);

void spi_end(Spi *);

#ifdef __cplusplus
}
#endif

#endif //PI_DISPLAY_SPI_H
