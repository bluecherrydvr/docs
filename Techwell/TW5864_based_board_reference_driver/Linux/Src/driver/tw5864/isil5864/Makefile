#
# Makefile for isilicro h264 codec
#

#PLATFORM =ppc
PLATFORM =x86
#PLATFORM =nxp
#PLATFORM =hisilicon

ifeq ($(PLATFORM), )

endif

ifeq ($(PLATFORM), ppc)
#Powerpc platform
#KSRC=/home/techwell/platform/PPC/kernel/Linux2.6.28/for_tw5866/
#DEST=/home/tftp
#KSRC=/mnt/work/devel/code/ppckernel
#DEST=/rootfs/powerpc/software
#KSRC=${FreescaleKernelPath}
#DEST=/home/dynasty/tftpboot
KSRC=/home/dynasty/workstation/svn/develop/platform/freeScale/kernel/linux-2.6.28.10/ppc-linux-2.6.28.10
DEST=/home/dynasty/nfs/tftpboot
endif

ifeq ($(PLATFORM), x86)
KSRC=/lib/modules/$(shell uname -r)/build
DEST=/rootfs/x86/software
endif

ifeq ($(PLATFORM), nxp)
KSRC=
DEST=/rootfs/nxp/software
endif

ifeq ($(PLATFORM), hisilicon)
KSRC=/mnt/work/devel/Hi3520_SDK_V1.0.6.2.c1/source/os/linux-2.6.24
DEST=/rootfs/hisilicon/software
endif

svn_version=`svn info|grep Version|awk '{print $2}'` 
driver_platfrom=$(PLATFORM)

EXTRA_CFLAGS += -I$(PWD)/../include -D__ISIL_ARCH__=\"$(driver_platfrom)\"

isil5864-objs := 	chip_bus_scheduler.o isil_new_buf.o 									\
				isil_devices_machine.o golomb.o algo_h264_driver.o isil_rc.o			\
				isil_h264_video_buf.o isil_osd_engine.o isil_h264_encode.o  \
				isil_vb_common.o \
				isil_jpeg_encode.o bar_ctl.o \
				isil_audio_iop.o isil_audio_buf.o	\
				isil_audio_chan_driver.o isil_avSync_device_buf.o isil_avSync_device.o	\
				isil_kthread.o isil_devices_machine.o ddr_burst_interface.o 			\
		 		isil_chip.o isil_chip_driver.o dma.o config.o isil_char_lib12.o isil_char_lib24.o \
				isil_chip_ai.o isil_chip_vi.o isil_chip_ao.o isil_chip_vo.o\
				isil_preview.o isil_push_buf_mgt.o\
				frame.o proc_entry.o  \
                                ../device_layer/tc.o  ../device_layer/tc_chip.o ../device_layer/tc_audio_en.o ../device_layer/tc_audio_de.o ../device_layer/tc_video_en.o ../device_layer/tc_jpeg_en.o\
                                ../device_layer/tc_buffer.o ../device_layer/tc_helper.o ../device_layer/audio_en_driver.o ../device_layer/audio_de_driver.o ../device_layer/jpeg_en_driver.o\
                                ../device_layer/video_en_driver.o ../device_layer/tc_chip_ai.o ../device_layer/tc_chip_vi.o

ifeq ($(PLATFORM), ppc)
isil5864-objs += isil_ppc_porting.o
EXTRA_CFLAGS += -DPOWERPC_PLATFORM -DPCI_PLATFORM -DBURST_INTERFACE -DDMA_INTERFACE -DPLATFORM_ENDIAN_DIFFERENT -DINTERFACE_BY_PCI
endif

ifeq ($(PLATFORM), hisilicon)
isil5864-objs += isil_porting_arm.o
EXTRA_CFLAGS += -DHISILICON_PLATFORM -DPCI_PLATFORM -DBURST_INTERFACE -DDMA_INTERFACE -DPLATFORM_ENDIAN_SAME -DINTERFACE_BY_DDR
endif

ifeq ($(PLATFORM), x86)
isil5864-objs += isil_porting_arm.o
EXTRA_CFLAGS += -DX86_PLATFORM -DPCI_PLATFORM -DPIO_INTERFACE -DPLATFORM_ENDIAN_SAME -DINTERFACE_BY_PCI
endif

ifeq ($(PLATFORM), nxp)
isil5864-objs += isil_porting_arm.o
EXTRA_CFLAGS += -DNXP_PLATFORM -DPCI_PLATFORM -DPIO_INTERFACE -DPLATFORM_ENDIAN_SAME -DINTERFACE_BY_PCI
endif

obj-m	+= isil5864.o

all:
	make -C $(KSRC) M=$(PWD) modules
	cp -rf isil5864.ko isil5864_$(PLATFORM).ko
	cp -rf isil5864.ko $(DEST)/isil5864_$(PLATFORM).ko
	cp -rf isil5864.ko /tftpboot/isil5864_$(PLATFORM).ko
clean:
	make -C $(KSRC) M=$(PWD) clean
	rm -rf Module.markers  Module.symvers modules.order
	rm -rf ../device_layer/*.o ../device_layer/.*.cmd ../device_layer/sample/*.o ../device_layer/sample/.*.cmd  
