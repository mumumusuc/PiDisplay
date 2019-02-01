//
// Created by mumumusuc on 19-1-31.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <zconf.h>

#define DATA_NUM    (10)

static const char *_node = "/dev/hello_char";

int main(int argc, char *argv[]) {

    uint8_t buffer[DATA_NUM] = {0};
    int fd = open(_node, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "file %s open error.\n", _node);
        exit(-1);
    }

    int rd = read(fd, buffer, DATA_NUM);
    printf("read buffer = %s .\n", buffer);
    strcpy(buffer, "I AM OK");
    int wt = write(fd, buffer, DATA_NUM);
    rd = read(fd, buffer, DATA_NUM);
    printf("read buffer = %s .\n", buffer);
    return 0;
}