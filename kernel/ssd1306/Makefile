obj-m := ssd1306_drv.o
ssd1306_drv-objs := ssd1306.o ssd1306_fb.o ssd1306_spi.o

HEAD := $(shell uname -r)
KERNEL := /usr/src/linux-headers-$(HEAD)/
PWD := $(shell pwd)

CC=arm-linux-gnueabihf-gcc

all:
	make -C $(KERNEL) M=$(PWD) modules
clean:
	make -C $(KERNEL) M=$(PWD)  clean
install:
	-rmmod spidev
	-rmmod ssd1306_drv
	-insmod ssd1306_drv.ko
uninstall:
	-rmmod ssd1306_drv
test:
	$(CC) -o ssd1306_test ssd1306_test.c -std=gnu99
	chgrp video ssd1306_test
