DVRDIR ?= $(shell /bin/pwd)


#PLATFORM = ppc
#PLATFORM = arm
PLATFORM = x86




ifeq ($(PLATFORM) ,x86)
PREFIX = 
else
ifeq ($(PLATFORM) ,ppc)
PREFIX = powerpc-linux-
ISLOCALGUI = 0
else
ifeq ($(PLATFORM) ,arm)
PREFIX = arm-linux-
endif
endif
endif


LIBEVINCPATH   = $(NETSDKPATH)libev/$(PLATFORM)/include
LIBEVLIBPATH   = $(NETSDKPATH)libev/$(PLATFORM)/lib

LIBEVENTINCPATH   = $(ISILSDKPATH)libevent/$(PLATFORM)/include
LIBEVENTLIBPATH   = $(ISILSDKPATH)libevent/$(PLATFORM)/lib





ISILSDKDIRNAME  =  isilsdk
NETSDKDIRNAME =    netsdk
NETAPPDIRNAME =    app
FILESDKDIRNAME =   filesdk

ISILSDKPATH= $(DVRDIR)/$(ISILSDKDIRNAME)/

NETSDKPATH= $(DVRDIR)/$(NETSDKDIRNAME)/

NETAPPPATH = $(DVRDIR)/$(NETAPPDIRNAME)/

FILESDKPATH= $(DVRDIR)/$(FILESDKDIRNAME)/


ISILSDKLIBPATH   = $(ISILSDKPATH)lib
ISILNETSDKLIBPATH   = $(NETSDKPATH)lib
FILESDKLIBPATH   = $(FILESDKPATH)lib





EVENTCOREINCPATH = $(ISILSDKPATH)include/eventcore

EVENTCORESLIBNAME = libevcore.a
EVENTCOREMLIBNAME = libevcore.so

MEDIASDKINCPATH = $(ISILSDKPATH)include/mediasdk

MEDIASDKSLIBNAME =  libbasemediasdk.a
MEDIASDKMLIBNAME =  libbasemediasdk.so

ISILSDKINCPATH = $(ISILSDKPATH)src/isilsdk

ISILSDKSLIBNAME         = libisilsdk.a
ISILSDKMLIBNAME         = libisilsdk.so


ISILMEDIASLIBNAME        = libisilmediasdk.a
ISILMEDIAMLIBNAME        = libisilmediasdk.so
LINKISILMEDIASLIBNAME    = isilmediasdk



NETCOREINCPATH = $(NETSDKPATH)include

NETCORESLIBNAME = libnetevcore.a
NETCOREMLIBNAME = libnetevcore.so
LINKNETCORESLIBNAME = netevcore

NETCORELIBPATH = $(NETSDKPATH)lib

NETMSGSLIBNAME = libnetmsg.a
NETMSGMLIBNAME = libnetmsg.so
LINKNETMSGSLIBNAME = netmsg

LIBNETMSGLIBPATH = $(NETAPPPATH)lib/
LIBNETMSGINCPATH = $(NETAPPPATH)netmsg/







ISILFILESDKSLIBNAME         = libfilesdk.a
ISILFILESDKMLIBNAME         = libfilesdk.so
LINKISILFILESDKSLIBNAME     = filesdk

FILESDKINCPATH = $(FILESDKPATH)include
FILESDKLIBPATH = $(FILESDKPATH)lib

PBSLIBNAME =  libpb.a
PBMLIBNAME = libpb.so
LINKPBSLIBNAME = pb

LIBPBBPATH = $(NETAPPPATH)lib/
LIBPBINCPATH = $(NETAPPPATH)pbapp/






#SHELL  = /bin/shell


CC=$(PREFIX)gcc  -lpthread
AR=$(PREFIX)ar
RANLIB=$(PREFIX)ranlib
LINK=$(PREFIX)ld
STRIP = $(PREFIX)strip


CFLAGS=   -static -Wall   -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-unused-variable

ifneq ($(PLATFORM) ,ppc)
CFLAGS += -D__LITTLE_ENDIAN_BITFIELD
else
ifneq ($(ISLOCALGUI) , 0)
CFLAGS += LOCAL_GUI_ON_BOARD
endif
endif





LINKFLAGS = $(INCPATH)  #-static






	


