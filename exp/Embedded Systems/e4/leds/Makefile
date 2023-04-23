obj-m +=leds.o
KERNELDIR ?= /home/pc/文档/workspace/linux-3.5
PWD :=$(shell pwd)
all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)

clean:	
	rm -rf *.o *~ core .depend .*.cmd *.ko.* *.mod.c .tem_versions
