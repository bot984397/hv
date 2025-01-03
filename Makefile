obj-m += mod.o

mod-objs := lkm.o cpu.o vmm.o mem.o vmcs.o hotplug.o vmx.o
mod-objs += vmasm.o

PWD := $(CURDIR)

all:
	#$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	$(MAKE) -C /lib/modules/6.12.7-artix1-1/build M=$(PWD) modules

clean:
	#$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	$(MAKE) -C /lib/modules/6.12.7-artix1-1/build M=$(PWD) clean
	sudo rmmod mod.ko
	sudo dmesg -c

install:
	sudo insmod mod.ko; sudo dmesg -c
