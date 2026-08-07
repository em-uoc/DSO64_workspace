ifneq ($(KERNELRELEASE),)
	obj-m := sysdemo.o 
else
	KERNELDIR := ../../../$(LINUX_VERSION)
	PWD := $(shell pwd)

.PHONY: all modules clean

all: modules test_nch

LDFLAGS=-static

PROGS = test_nch
test_nch: test_nch.o get_nch.o
test_nch.o: get_nch.h

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -rf $(PROGS)

endif
