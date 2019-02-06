//
// Created by mumumusuc on 19-1-31.
//

#include <stdio.h>
//#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    printf("%s.\n", __func__);
    sleep(1);
    printf("%s.\n", __func__);
    return 0;
}