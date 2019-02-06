obj-m := hello.o

CC=arm-linux-gnueabihf-gcc

HEAD := $(shell uname -r)
KERNEL := /usr/src/linux-headers-$(HEAD)/
PWD := $(shell pwd)

all:
	make -C $(KERNEL) M=$(PWD) modules
clean:
	make -C $(KERNEL) M=$(PWD)  clean
	-rm ./*.o test
install:
	insmod hello.ko
uninstall:
	rmmod hello
test:
	$(CC) -o test hello_test.c -std=gnu99