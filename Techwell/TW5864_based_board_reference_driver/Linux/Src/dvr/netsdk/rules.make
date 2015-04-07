
TOPDIR ?= $(shell /bin/pwd)


#PLATFORM ?= ppc
#PLATFORM ?= arm
PLATFORM ?= x86

ifeq ($(PLATFORM) ,x86)
PREFIX ?= 
else
ifeq ($(PLATFORM) ,ppc)
PREFIX ?= powerpc-linux-
else
ifeq ($(PLATFORM) ,arm)
PREFIX ?= arm-linux-
endif
endif
endif

LIBEVINCPATH   = $(TOPDIR)/libev/$(PLATFORM)/include
LIBEVLIBPATH   = $(TOPDIR)/libev/$(PLATFORM)/lib

TOPHPATH   ?= $(TOPDIR)/include
TOPLPATH   ?= $(TOPDIR)/lib
TOPBINPATH ?= $(TOPDIR)/bin
TOPSRCPATH ?= $(TOPDIR)/src
TOPTESTPATH ?= $(TOPDIR)/test
TOPDOCPATH ?= $(TOPDIR)/doc


NETCOREINCPATH ?= $(TOPHPATH)/include

NETCORESLIBNAME ?=  libnetevcore.a
NETCOREMLIBNAME ?= libevcore.so



EXECFILENAME ?= netsdktest

#SHELL  = /bin/shell



CC ?=$(PREFIX)gcc -lpthread
AR ?=$(PREFIX)ar
RANLIB ?=$(PREFIX)ranlib
LINK ?=$(PREFIX)ld
STRIP ?= $(PREFIX)strip
LIBS ?= -lpthread -lev -lrt
LIBPATH	?= $(addprefix -L,$(TOPLPATH))


CFLAGS ?=   -static -Wall  -W -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-unused-variable $(INCPATH)   #-g


ifeq ($(PLATFORM) ,x86)
CFLAGS ?+= -D__LITTLE_ENDIAN_BITFIELD
endif

#DVMFFMPEGLIB        = /opt/powerpc/ffmpeg/lib










	


