//
// Created by mumumusuc on 19-1-25.
//

#ifndef PI_DISPLAY_SPI_H
#define PI_DISPLAY_SPI_H

#include "object.h"

#ifdef __cplusplus
extern "C" {
#endif

// define spi
typedef struct _Spi_VTbl SpiVTbl;

typedef struct _SpiInfo {
    uint8_t cs;
    uint32_t speed;
} SpiInfo;

typedef struct _Spi {
    SpiVTbl *vtbl;
    Object *obj;
} Spi;

Spi *new_spi();

void del_spi(void *);

void spi_begin(Spi *);

void spi_init(Spi *, const SpiInfo *);

void spi_write(Spi *, const uint8_t *, size_t);

void spi_read(Spi *, uint8_t *, size_t);

void spi_end(Spi *);

#ifdef __cplusplus
}
#endif

#endif //PI_DISPLAY_SPI_H
