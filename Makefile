KERNEL_LOCATION := /lib/modules/$(shell uname -r)/build
#KERNEL_LOCATION := ~/work/MVV210_TVB/microvision/V210/Linux/kernel/kernel_2.6.32_v210_main
ifeq ($(findstring version.h,$(wildcard ($KERNEL_LOCATION)/include/linux/%.h))),version.h)
	KERNEL_VERSION_CODE := $(shell awk '/LINUX_VERSION_CODE/ {print $$3}' $(KERNEL_LOCATION)/include/linux/version.h)
else
	KERNEL_VERSION_CODE := $(shell awk '/LINUX_VERSION_CODE/ {print $$3}' /usr/include/linux/version.h)
endif
MODULE_NAME := tvb380v4

#Test if the kernel if older than 2.5.51
#ifeq ($(shell expr ${KERNEL_VERSION_CODE} '<=' 132403),1)
#MODULES := $(MODULE_NAME).o
#else
# we are using kernel version 2.6.32 
MODULES := $(MODULE_NAME).ko
#endif

#EXTRA_CFLAGS += -DTVB380
EXTRA_CFLAGS += -DTVB380V4
EXTRA_CFLAGS += -DTVB390V8
#EXTRA_CFLAGS += -DTVB595V1

all: $(MODULES)

obj-m := $(MODULE_NAME).o
$(MODULES): $(MODULE_NAME).c tsp100.h 
	$(MAKE) -C $(KERNEL_LOCATION) SUBDIRS=$(PWD) modules
	
install: $(MODULES) 
	install -d $(DESTDIR)/lib/modules/$(shell uname -r)/$(MODULE_NAME)
	install $(MODULES) $(DESTDIR)/lib/modules/$(shell uname -r)/$(MODULE_NAME)

tar: clean
	tar -czf $(MODULE_NAME)_$(shell uname -r).tgz *.c *.h Makefile Makefile.hld l u lld hld include rbf README *.pdf license.dat libusb-0.1.12 PlxSdk p build.sh buildall.sh shmem

clean:
#	(cd ./lld; make -f Makefile.380 clean)
#	 -rm -f *.o *.ko *.mod.c .*.cmd .*.o.flags *~ core p380* p390* mod_* lld_tse110
	-rm -f *.o *.ko *.mod.c .*.cmd .*.o.flags *~ core

