//
// Created by mumumusuc on 19-2-8.
//

#ifndef PI_DISPLAY_SSD_1036_H
#define PI_DISPLAY_SSD_1036_H

#include <linux/types.h>
#include <linux/ioctl.h>

#define SSD_IOC_MAGIC   's'
#define SSD_IOC_MAXNR   6

#define SSD_IOC_RSET    _IOW(SSD_IOC_MAGIC,0,void*)
#define SSD_IOC_INFO    _IOR(SSD_IOC_MAGIC,1,void*)
#define SSD_IOC_CLEAR   _IO(SSD_IOC_MAGIC,2)
#define SSD_IOC_ON      _IO(SSD_IOC_MAGIC,3)
#define SSD_IOC_OFF     _IO(SSD_IOC_MAGIC,4)
#define SSD_IOC_DISPLAY _IOW(SSD_IOC_MAGIC,5,void*)

typedef struct {
    __u16 width;
    __u16 height;
    __u8 format;
    size_t size;
    char vendor[16];
} display_info_t;

typedef struct {
    __u8 brightness;
    __u8 inverse;
    // TODO: other configs
} display_config_t;

#endif //PI_DISPLAY_SSD_1036_H
