obj-m := mlc.o
mlc-objs := kernel.o flash_dram.o cache.o pm.o ftl.o host.o

KDIR := /lib/modules/$(shell uname -r)/build
#KDIR := /home/key7/Working/2012_MLCSSD/linux-2.6-xlnx
PWD := $(shell pwd)

EXTRA_CFLAGS := -g

all: modules tags

modules:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean

tags:
	@ctags -R > /dev/null 2>&1 || true
