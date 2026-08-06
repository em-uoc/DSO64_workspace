ifneq ($(KERNELRELEASE),)
	obj-m := sysdemo.o 
else
	KERNELDIR := ../../../$(LINUX_VERSION)
	PWD := $(shell pwd)

.PHONY: all modules clean

all: modules test_nch

CFLAGS=-static

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

endif
