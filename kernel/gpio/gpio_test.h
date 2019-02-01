//
// Created by mumumusuc on 19-2-1.
//

#ifndef PI_DISPLAY_GPIO_H
#define PI_DISPLAY_GPIO_H

//#include <linux/ioctl.h>
#include <sys/ioctl.h>

#define OK          (0)
#define ERR         (-1)

#define GPIO_HIGH           (0x01)
#define GPIO_LOW            (0x00)

#define GPIO_MODE_INPUT     (0x00)
#define GPIO_MODE_OUTPUT    (0x01)
#define GPIO_MODE_ALT0      (0x04)
#define GPIO_MODE_ALT1      (0x05)
#define GPIO_MODE_ALT2      (0x06)
#define GPIO_MODE_ALT3      (0x07)
#define GPIO_MODE_ALT4      (0x03)
#define GPIO_MODE_ALT5      (0x02)

#define IOC_MAGIC           'g'
#define IOC_MAXNR           4
#define GPIO_IOC_SET_MODE   _IOW(IOC_MAGIC,0,void*)
#define GPIO_IOC_GET_MODE   _IOR(IOC_MAGIC,1,void*)
#define GPIO_IOC_SET_VALUE  _IOW(IOC_MAGIC,2,void*)
#define GPIO_IOC_GET_VALUE  _IOR(IOC_MAGIC,3,void*)

const char *node = "/dev/gpio";

typedef struct gpio_info {
    unsigned char pin;
    union {
        unsigned char mode;
        unsigned char value;
    };
} gpio_info_t;

#endif //PI_DISPLAY_GPIO_H
