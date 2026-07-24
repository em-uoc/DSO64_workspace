

ifneq ($(KERNELRELEASE),)

obj-m := driver.o 
 
else
	KERNELDIR := ../../../$$LINUX_VERSION
	PWD := $(shell pwd)
	USERPROG := testdriver

all: modules $(USERPROG)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

testdriver: testdriver.c
	gcc -static -D__KERNEL__ -I../../$(KERNEL_VERSION)/include testdriver.c -o testdriver

clean:
	rm -rf *.[oas] .*.flags *.ko .*.cmd .*.d .*.tmp *.mod.c .tmp_versions testdriver Module.symvers *.mod *.order $(USERPROG)
endif

