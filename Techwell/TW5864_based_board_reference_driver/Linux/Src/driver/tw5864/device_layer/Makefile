KSRC=/home/techwell/platform/ARM920/kernel/Linux2.6.20/linux-2.6.20.21

tc_core-objs := 	tc.o  sample/my_test.o tc_audio_de.o  tc_audio_en.o tc_buffer.o  tc_helper.o  audio_en_driver.o  audio_de_driver.o \
                                ../tw5864/chip_bus_scheduler.o ../tw5864/dvm_new_buf.o 									\
				../tw5864/tw_devices_machine.o ../tw5864/golomb.o ../tw5864/algo_h264_driver.o ../tw5864/dvm_rc.o			\
				../tw5864/tw_h264_video_buf.o	../tw5864/tw_h264_encode.o  \
				../tw5864/tw_vb_common.o \
				../tw5864/bar_ctl.o \
				../tw5864/tw_audio_iop.o ../tw5864/tw_audio_buf.o	\
				../tw5864/tw_audio_chan_driver.o ../tw5864/tw_avSync_device_buf.o ../tw5864/tw_avSync_device.o	\
				../tw5864/tw_kthread.o ../tw5864/tw_devices_machine.o ../tw5864/ddr_burst_interface.o 			\
		 		../tw5864/tw_chip.o ../tw5864/dma.o  ../tw5864/config.o ../tw5864/tw_char_lib12.o ../tw5864/tw_char_lib24.o \
				../tw5864/frame.o ../tw5864/proc_entry.o
ifeq ($(ARCH), arm)
tc_core-objs += ../tw5864/tw_porting_arm.o
endif

ifeq ($(ARCH), powerpc)
tc_core-objs += ../tw5864/tw_ppc_porting.o
endif
#obj-$(CONFIG-DVMICRO)	+= tw5864.o
obj-m	+= tc_core.o
#obj-m	+= tw5864.o

all:
	make -C $(KSRC) M=$(PWD) modules
clean:
	make -C $(KSRC) M=$(PWD) clean
	rm -rf Module.markers  Module.symvers modules.order
