//
// Created by mumumusuc on 19-1-19.
//

#ifndef PI_DISPLAY_COMMON_H
#define PI_DISPLAY_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <zconf.h>
#include <errno.h>

#define TEST
#define DEBUG
#define PI_3

#ifdef DEBUG
#define DEFAULT_METHOD()    ({ERROR("%s",__func__);})
#else
#define DEFAULT_METHOD()    ({METHOD_NOT_IMPLEMENTED();})
#endif

#define LOG_TAG  "PI_DISPLAY"

#ifndef offsetof
#define offsetof(type, member) ((size_t) &((type *)0)->member)
#endif

#ifndef container_of
#define container_of(ptr, type, member)                                         \
({  const typeof( ((type *)0)->member ) *__mptr = (ptr);                        \
    (type *)( (char *)__mptr - offsetof(type,member) );})
#endif


#define LOG(format, ...)                                                        \
do{                                                                             \
    fprintf(stdout, "(LOG)"LOG_TAG" : "format".\n", ##__VA_ARGS__);             \
} while(0)

#define ERROR(format, ...)                                                      \
do{                                                                             \
      fprintf(stderr,"(ERR)"LOG_TAG" : "format".(%s)\n",##__VA_ARGS__,strerror(errno));             \
} while(0)

#define METHOD_NOT_IMPLEMENTED()                                                \
do{                                                                             \
      ERROR("Method(%s) not implemented, exit",__func__);                       \
      exit(-1);                                                                 \
} while(0)

#define eval_vtbl(class, member, ...)  do{(class)->vtbl->member((class),##__VA_ARGS__);}while(0)

#define udelay(u_sec)    usleep((u_sec))

#define delay(m_sec)    usleep(1000*(m_sec))

#define API_HIDE        __attribute__((unavailable("Private member.")))

#define API_WEAK        __attribute__((weak))

#define API_OVERLOAD    __attribute__((overloadable))

#endif //PI_DISPLAY_COMMON_H

