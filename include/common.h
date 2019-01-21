//
// Created by mumumusuc on 19-1-19.
//

#ifndef PI_DISPLAY_COMMON_H
#define PI_DISPLAY_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define offsetof(type, member) ((size_t) &((type *)0)->member)

#define container_of(ptr, type, member) ({                                      \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);                        \
    (type *)( (char *)__mptr - offsetof(type,member) );})


#define LOG_TAG               "PI_DISPLAY"

#define LOG(format, ...) do{                                                    \
    fprintf(stdout, "(LOG)"LOG_TAG" : "format".\n", ##__VA_ARGS__);             \
} while(0)

#define ERROR(format, ...) do{                                                  \
      fprintf(stderr,"(ERR)"LOG_TAG" : "format".\n",##__VA_ARGS__);             \
} while(0)

#define METHOD_NOT_IMPLEMENTED(name) do{                                        \
      ERROR("Method(%s) not implemented, exit",(name));                         \
      exit(-1);                                                                 \
} while(0)

#endif //PI_DISPLAY_COMMON_H
