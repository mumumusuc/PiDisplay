obj-m := ssd1306_spi.o
ssd1306_spi-objs := ssd1306.o ssd1306_spi4.o

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
	-rmmod ssd1306_spi
	-insmod ssd1306_spi.ko
uninstall:
	rmmod ssd1306_spi
test:
	$(CC) -o ssd1306_spi4_test ssd1306_spi4_test.c -std=gnu99
