ifneq ($(KERNELRELEASE),)

obj-m := abc.o 
 
else
	KERNELDIR := ../../../$$LINUX_VERSION
	PWD := $(shell pwd)
	USERPROG := test3

all: modules $(USERPROG)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

test3: test3.c
	gcc -static test3.c -o test3

clean:
	rm -rf *.[oas] .*.flags *.ko .*.cmd .*.d .*.tmp *.mod.c .tmp_versions test6 Module.symvers *.mod *.order $(USERPROG)
endif
