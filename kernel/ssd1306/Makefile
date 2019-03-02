obj-m := ssd1306.o
ssd1306-objs := ssd1306_dev.o display_fb.o ssd1306_spi.o ssd1306_i2c.o

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
	-rmmod ssd1306_drv
	-insmod ssd1306_drv.ko
uninstall:
	-rmmod ssd1306_drv
test:
	$(CC) -o ssd1306_test ssd1306_test.c -std=gnu99
	chgrp video ssd1306_test
