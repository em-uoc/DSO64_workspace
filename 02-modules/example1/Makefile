ifneq ($(KERNELRELEASE),)
	obj-m := hw.o 
else
	KERNELDIR := ../../../$(LINUX_VERSION)
	PWD := $(shell pwd)

.PHONY: all modules clean

all: modules

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

endif
