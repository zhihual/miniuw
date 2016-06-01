ROOTOBJ := drv.o


OBJS := $(ROOTOBJ) 


obj-m += miniuw.o
miniuw-y += $(OBJS)
KERNELDIR := $(shell uname -r)
PWD := $(shell pwd)



all:
	$(MAKE) -C /lib/modules/$(KERNELDIR)/build M=$(PWD) modules

clean :
	$(MAKE) -C /lib/modules/$(KERNELDIR)/build M=$(PWD) modules clean
	rm -rf *.o *.ko *.mod.c *.mod *.order *.symvers

	
