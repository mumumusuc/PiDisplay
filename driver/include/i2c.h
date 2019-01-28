//
// Created by mumumusuc on 19-1-25.
//

#ifndef PI_DISPLAY_I2C_H
#define PI_DISPLAY_I2C_H

#include "object.h"

#ifdef __cplusplus
extern "C" {
#endif

// define i2c
typedef struct _I2c_VTbl I2cVTbl;

typedef struct _I2cInfo {
    uint8_t address;
    uint32_t baudrate;
} I2cInfo;

typedef struct _I2c {
    I2cVTbl *vtbl;
    Object *obj;
} I2c;

I2c *new_i2c(void);

void del_i2c(void *);

void i2c_begin(I2c *);

void i2c_init(I2c *, const I2cInfo *);

void i2c_write(I2c *, const uint8_t *, size_t);

void i2c_read(I2c *, uint8_t *, size_t);

void i2c_end(I2c *);
// end define i2c

#ifdef __cplusplus
}
#endif

#endif //PI_DISPLAY_I2C_H
