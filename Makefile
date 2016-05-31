obj-m += miniuw.o
miniuw-y += drv.o bmi.o
KERNELDIR := $(shell uname -r)
PWD := $(shell pwd)



all:
	$(MAKE) -C /lib/modules/$(KERNELDIR)/build M=$(PWD) modules

clean :
	$(MAKE) -C /lib/modules/$(KERNELDIR)/build M=$(PWD) modules clean
	rm -rf *.o *.ko *.mod.c *.mod *.order *.symvers

	
