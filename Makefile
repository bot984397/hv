obj-m += mod.o

mod-objs := lkm.o cpu.o vmm.o mem.o vmcs.o hotplug.o

PWD := $(CURDIR)

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	sudo rmmod mod.ko
	sudo dmesg -c

install:
	sudo insmod mod.ko; sudo dmesg -c
