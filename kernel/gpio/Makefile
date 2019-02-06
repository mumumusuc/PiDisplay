obj-m := gpiomem.o
gpiomem-objs := gpio.o bcm2835_gpio.o

HEAD := $(shell uname -r)
KERNEL := /usr/src/linux-headers-$(HEAD)/
PWD := $(shell pwd)

CC=arm-linux-gnueabihf-gcc

all:
	make -C $(KERNEL) M=$(PWD) modules
clean:
	make -C $(KERNEL) M=$(PWD)  clean
install:
	-rmmod gpiomem
	-insmod gpiomem.ko
uninstall:
	rmmod gpiomem
test:
	$(CC) -o gpio_test gpio_test.c -std=gnu99
