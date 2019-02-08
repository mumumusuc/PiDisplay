obj-m := ssd1306_spi4.o

HEAD := $(shell uname -r)
KERNEL := /usr/src/linux-headers-$(HEAD)/
PWD := $(shell pwd)

CC=arm-linux-gnueabihf-gcc

all:
	make -C $(KERNEL) M=$(PWD) modules
clean:
	make -C $(KERNEL) M=$(PWD)  clean
install:
	-rmmod ssd1306_spi4
	-insmod ssd1306_spi4.ko
uninstall:
	rmmod ssd1306_spi4
test:
	$(CC) -o ssd1306_spi4_test ssd1306_spi4_test.c -std=gnu99
