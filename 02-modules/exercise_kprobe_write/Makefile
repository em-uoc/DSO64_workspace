ifneq ($(KERNELRELEASE),)
	obj-m := kprobe.o 
else
	KERNELDIR := ../../../$(LINUX_VERSION)
	PWD := $(shell pwd)

.PHONY: all modules clean

all: modules test_wr

CFLAGS=-static

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

endif
