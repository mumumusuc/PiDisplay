obj-m := ili9341.o
ssd1306-objs := ili9341_dev.o ili9341_fb.o
#ili9341_spi.o

HEAD := $(shell uname -r)
#KERNEL := /usr/src/linux-headers-$(HEAD)/
KERNEL := /home/mumumusuc/raspberry/linux/
PWD := $(shell pwd)

CC=arm-linux-gnueabihf-gcc

all:
	make -C $(KERNEL) M=$(PWD) modules
clean:
	make -C $(KERNEL) M=$(PWD) clean
install:
	-rmmod ili9341
	-insmod ili9341.ko
uninstall:
	-rmmod ili9341
test:
	$(CC) -o ili9341_test ili9341_test.c -std=gnu99
	chgrp video ili9341_test
