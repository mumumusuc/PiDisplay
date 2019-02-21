obj-m := dht11.o

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
	-rmmod dht11
	insmod dht11.ko
uninstall:
	rmmod dht11
test:
	$(CC) -o dht11_test dht11_test.c -std=gnu99
