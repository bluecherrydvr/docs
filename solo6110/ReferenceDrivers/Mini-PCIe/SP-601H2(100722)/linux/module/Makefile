KERNELDIR	= /usr/src/kernels/2.6.21-1.3194.fc7-i686

PWD		= $(shell pwd)

EXTRA_CFLAGS	= -I$(M)/include
EXTRA_CFLAGS	+= -Wall

solo6x10-objs	:= main.o device.o sl_thread.o sl_queue.o p2m.o vin.o disp.o fb_common.o fb_2.6.o enc.o dec.o g723.o uart_2.6.o i2c.o saa7128.o tw2815.o tw2865.o
obj-m		:= solo6x10.o

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
