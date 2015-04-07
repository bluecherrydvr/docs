TOPDIR ?= $(shell /bin/pwd)


#PLATFORM = ppc
#PLATFORM = arm
PLATFORM  = x86

ifeq ($(PLATFORM) ,x86)
PREFIX = 
else
ifeq ($(PLATFORM) ,ppc)
PREFIX = powerpc-linux-
else
ifeq ($(PLATFORM) ,arm)
PREFIX = arm-linux-
endif
endif
endif


LIBEVENTINCPATH = $(TOPDIR)/libevent/$(PLATFORM)/include 
LIBEVENTLIBPATH += $(TOPDIR)/libevent/$(PLATFORM)/lib

TOPHPATH   = $(TOPDIR)/include
TOPLPATH   = $(TOPDIR)/lib
TOPBINPATH = $(TOPDIR)/bin
TOPSRCPATH = $(TOPDIR)/src
TOPTESTPATH = $(TOPDIR)/test
TOPDOCPATH = $(TOPDIR)/doc


ISILSDKPATH = $(TOPDIR)/

EVENTCOREINCPATH = $(TOPHPATH)/eventcore

EVENTCORESLIBNAME =  libevcore.a
EVENTCOREMLIBNAME =  libevcore.so

MEDIASDKINCPATH = $(TOPHPATH)/mediasdk


MEDIASDKSLIBNAME =  libbasemediasdk.a
MEDIASDKMLIBNAME =  libbasemediasdk.so

ISILSDKINCPATH = $(TOPDIR)/src/isilsdk

ISILSDKSLIBNAME         = libisilsdk.a
ISILSDKMLIBNAME         = libisilsdk.so

ISILSDKLIBPATH          = $(TOPLPATH)


ISILMEDIASLIBNAME         =  libisilmediasdk.a
ISILMEDIAMLIBNAME        = libisilmediasdk.so
LINKISILMEDIASLIBNAME    = isilmediasdk



EXECFILENAME = sdktest

#SHELL  = /bin/shell



CC =$(PREFIX)gcc -lpthread
AR =$(PREFIX)ar
RANLIB =$(PREFIX)ranlib
LINK =$(PREFIX)ld
STRIP = $(PREFIX)strip
LIBS = -lpthread -levent -lrt
LIBPATH	= $(addprefix -L,$(TOPLPATH))


CFLAGS =   -static -Wall  -W -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-unused-variable $(INCPATH)   #-g


ifeq ($(PLATFORM) ,x86)
CFLAGS += -D__LITTLE_ENDIAN_BITFIELD
endif


#DVMFFMPEGLIB        = /opt/powerpc/ffmpeg/lib










	


