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
	insmod gpiomem.ko
uninstall:
	rmmod gpiomem
test:
	cc -o gpio_test gpio_test.c