OUTPUT_DIR?=../../../../bin/linux
include ../../../../make.cfg
TARGET=s6stretch
KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build/
#KERNEL_DIR = 
MODULE_DIR=$(shell cd $(OUTPUT_MOD);pwd)
COMMON_DIR=$(shell cd ../../driver_common;pwd)
EXTRA_INC=$(shell cd ../include;pwd)
EXTRA_INC += $(shell cd ../../SCT;pwd)
export EXTRA_INC

FILES=$(wildcard *.c) $(wildcard *.h)
COMMON_FILES=$(wildcard $(COMMON_DIR)/*.c) $(wildcard $(COMMON_DIR)/*.h)

all:: $(OUTPUT_BIN) $(OUTPUT_MOD) $(OUTPUT_BIN)/$(TARGET).ko 

$(OUTPUT_BIN)/$(TARGET).ko : $(FILES)
	$(foreach file,$(FILES),$(shell cp -pf $(file) $(addprefix $(MODULE_DIR)/,$(file))))
	$(foreach file,$(COMMON_FILES),$(shell cp -pf $(file) $(MODULE_DIR)/.))
	cp -pf module.mak $(OUTPUT_MOD)/Makefile
ifdef ARCH	
	$(MAKE) -C $(KERNEL_DIR) V=1 M=$(MODULE_DIR) ARCH=$(ARCH) CC=$(CC) LD=$(LD) modules
else	
	$(MAKE) -C $(KERNEL_DIR) V=1 M=$(MODULE_DIR) modules
endif	
	if [ $$? != 0 ] ; then exit 1 ; fi
	cp -fp $(OUTPUT_MOD)/$(TARGET).ko $(OUTPUT_BIN)
	cp -fp load $(OUTPUT_BIN)
	chmod +x load
	cp -fp unload $(OUTPUT_BIN)
	chmod +x unload
ifeq ($(ARCH),powerpc)	
	sudo cp -r $(OUTPUT_BIN) /nfsroot/root
endif

clean::
	$(RM) -r $(OUTPUT_MOD)
	$(RM) $(OUTPUT_BIN)/$(TARGET).ko
	$(RM) $(OUTPUT_BIN)/load
	$(RM) $(OUTPUT_BIN)/unload

load::
	sudo /sbin/insmod $(OUTPUT_BIN)/$(TARGET).ko || exit 1
	@grep $(TARGET) /proc/devices >/dev/null; if [ $$? != 0 ] ; then echo Device not found; exit 1; fi
	sudo rm -f /dev/$(TARGET)*
	sudo mknod /dev/$(TARGET)0 c `grep $(TARGET) /proc/devices | head -n 1 | cut -d ' ' -f 1` 0
	sudo mknod /dev/$(TARGET)1 c `grep $(TARGET) /proc/devices | head -n 1 | cut -d ' ' -f 1` 1
	sudo mknod /dev/$(TARGET)2 c `grep $(TARGET) /proc/devices | head -n 1 | cut -d ' ' -f 1` 2
	sudo mknod /dev/$(TARGET)3 c `grep $(TARGET) /proc/devices | head -n 1 | cut -d ' ' -f 1` 3
	sudo chmod 666 /dev/$(TARGET)*

unload::
	sudo rm -f /dev/$(TARGET)*
	sudo /sbin/rmmod $(TARGET)

include ../../../../rule.cfg
