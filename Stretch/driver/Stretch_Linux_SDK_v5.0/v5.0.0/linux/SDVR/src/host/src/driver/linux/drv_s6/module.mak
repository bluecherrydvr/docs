TARGET = s6stretch
ifneq ($(KERNELRELEASE),)
#first pass
obj-m := $(TARGET).o
s6stretch-objs := ipcqueue.o \
                  stretch_main.o \
                  pci_resources.o \
                  shared_memory.o \
                  dma_channels.o \
                  msg_pool.o \
                  dispatch.o \
                  control.o
                  #msg_queues.o 
DBGFLAGS = -Werror
EXTRA_CFLAGS += $(patsubst %, -I%,$(subst :, ,$(EXTRA_INC)))
EXTRA_CFLAGS += $(DBGFLAGS)
VER_COMP=$(shell expr $(SUBLEVEL) \< 20)
ifeq ($(VER_COMP),1)
EXTRA_CFLAGS += -DKVER_LESS_2620
endif
VER_COMP=$(shell expr $(SUBLEVEL) \< 17)
ifeq ($(VER_COMP),1)
EXTRA_CFLAGS += -DKVER_LESS_2617
endif
else
#second pass
KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build/
MODULE_DIR=$(CURDIR)

all::
	echo $(VER_COMP)
	exit 1
	@echo "Building and Linking object code for kernel module"
	$(MAKE) -C $(KERNEL_DIR) V=1 M=$(MODULE_DIR) modules
	if [ $$? != 0 ] ; then exit 1 ; fi 

clean::
	@echo "Cleaning object code for kernel module"
	$(MAKE) -C $(KERNEL_DIR) V=1 M=$(MODULE_DIR) clean

endif	
