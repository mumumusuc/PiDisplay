//
// Created by mumumusuc on 19-2-9.
//

#ifndef PI_DISPLAY_SSD1306_USER_H
#define PI_DISPLAY_SSD1306_USER_H

#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#define SSD_IOC_MAGIC   's'
#define SSD_IOC_MAXNR   6

#define SSD_IOC_RSET    _IOW(SSD_IOC_MAGIC,0,void*)
#define SSD_IOC_INFO    _IOR(SSD_IOC_MAGIC,1,void*)
#define SSD_IOC_CLEAR   _IO(SSD_IOC_MAGIC,2)
#define SSD_IOC_ON      _IO(SSD_IOC_MAGIC,3)
#define SSD_IOC_OFF     _IO(SSD_IOC_MAGIC,4)
#define SSD_IOC_DISPLAY _IOW(SSD_IOC_MAGIC,5,void*)

typedef struct display_info {
    uint16_t width;
    uint16_t height;
    uint8_t format;
    size_t size;
    char vendor[16];
} display_info_t;

typedef struct display_config {
    uint8_t brightness;
    uint8_t inverse;
    // TODO: other configs
} display_config_t;

#endif //PI_DISPLAY_SSD1306_USER_H
