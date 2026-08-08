ifneq ($(KERNELRELEASE),)
	obj-m := abc.o 
else
	KERNELDIR := ../../../$$LINUX_VERSION
	PWD := $(shell pwd)

.PHONY: all modules clean

PROGS = test3
LDFLAGS = -static

all: modules $(PROGS)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -rf $(PROGS)
endif
