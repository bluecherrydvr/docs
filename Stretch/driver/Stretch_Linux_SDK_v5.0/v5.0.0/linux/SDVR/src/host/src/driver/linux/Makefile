include ../../../make.cfg
OUTPUT_DIR?=$(shell cd ../../../bin/linux;pwd)
export OUTPUT_DIR
ifdef KERNEL_DIR
export KERNEL_DIR
endif

all:
	make -C ../SCT
ifdef ARCH
	make -C drv_s6 ARCH=$(ARCH) CC=$(CC) LD=$(LD)
else	
	make -C drv_s6
endif	
	make -C test_sct

clean:
	make -C ../SCT clean
	make -C drv_s6 clean
	make -C test_sct clean

load::
	make -C drv_s6 load

unload::
	make -C drv_s6 unload

test::
	make -C test_sct test
