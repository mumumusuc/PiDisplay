//
// Created by mumumusuc on 19-2-7.
//

#ifndef PI_DISPLAY_HC_SR04_H
#define PI_DISPLAY_HC_SR04_H

#include <linux/ioctl.h>

#define IOC_MAGIC           'h'
#define IOC_MAXNR           2
#define HC_SR_IOC_TRIG      _IO(IOC_MAGIC,0)
#define HC_SR_IOC_READ      _IOR(IOC_MAGIC,1,u32*)
#define HC_SR_IOC_TRIG_READ _IOR(IOC_MAGIC,2,u32*)

#endif //PI_DISPLAY_HC_SR04_H
