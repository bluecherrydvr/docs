ifneq ($(GLABALCOMPILE),yes)
	override TOPDIR := ./../../
	sinclude	$(TOPDIR)rules.make	
endif


CFLAGS +=  -I$(LIBPBINCPATH)       \
		   -I$(LIBNETMSGINCPATH)  \
		   -I$(NETAPPPATH)include  \
		   -I$(ISILSDKPATH)include  \
		   -I$(EVENTCOREINCPATH) \
		   -I$(MEDIASDKINCPATH) \
		   -I$(ISILSDKINCPATH) \
		   -I$(NETCOREINCPATH) \
		   -I$(FILESDKINCPATH) \
		   -I$(LIBEVINCPATH) \
		   -I$(LIBEVENTINCPATH) 
		   
           
	       


EXECFILENAME = dvr	   
	   
#CFLAGS += -static	

	   

MAKE_DIR=$(PWD)

LIBNAME = $(PBSLIBNAME)


SRCS :=$(wildcard *.c)
SRC_FILES:=$(notdir $(SRCS))
OBJ_FILES :=$(patsubst %c,%o,$(SRC_FILES))





 
all: $(OBJ_FILES)
	$(AR) cr $(LIBNAME) $(OBJ_FILES)
	@mv $(LIBNAME) $(LIBPBBPATH)
	
    
#	$(AR) cr $(LIBNAME) $(OBJ_FILES)
#	@mv $(LIBNAME) $(TOPLPATH)
#-rm -f *.o
	
.PHONY:clean
clean:
	-rm -f *.o
	
	
	
