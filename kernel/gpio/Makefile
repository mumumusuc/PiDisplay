obj-m := gpiomem.o
gpiomem-objs := gpio.o bcm2835_gpio.o

HEAD := $(shell uname -r)
KERNEL := /usr/src/linux-headers-$(HEAD)/
PWD := $(shell pwd)

all:
	make -C $(KERNEL) M=$(PWD) modules
clean:
	make -C $(KERNEL) M=$(PWD)  clean
install:
	-rmmod gpiomem
	-insmod gpiomem.ko
	-mknod /dev/gpio c 232 0
	-chmod 660 /dev/gpio
	-chgrp pi /dev/gpio
uninstall:
	rmmod gpiomem
test:
	cc -o gpio_test gpio_test.c