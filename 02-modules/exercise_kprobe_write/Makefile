ifneq ($(KERNELRELEASE),)
	obj-m := kprobe.o 
else
	KERNELDIR := ../../../$(LINUX_VERSION)
	PWD := $(shell pwd)

.PHONY: all modules clean

PROGS = test_wr
LDFLAGS = -static

all: modules $(PROGS)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -rf $(PROGS)
endif
