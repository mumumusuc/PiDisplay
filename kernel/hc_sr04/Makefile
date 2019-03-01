obj-m := hc_sr04.o

HEAD := $(shell uname -r)
#KERNEL := /usr/src/linux-headers-$(HEAD)/
KERNEL := /home/mumumusuc/raspberry/linux/
PWD := $(shell pwd)

CC=arm-linux-gnueabihf-gcc

all:
	make -C $(KERNEL) M=$(PWD) modules
clean:
	make -C $(KERNEL) M=$(PWD)  clean
install:
	-rmmod hc_sr04
	-insmod hc_sr04.ko
uninstall:
	rmmod gpiomem
test:
	$(CC) -o hc_sr04_test hc_sr04_test.c -std=gnu99
