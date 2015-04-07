

CFLAGS += -I$(NETAPPPATH)include  \
		   -I$(ISILSDKPATH)include  \
		   -I$(EVENTCOREINCPATH) \
		   -I$(MEDIASDKINCPATH) \
		   -I$(ISILSDKINCPATH) \
		   -I$(NETCOREINCPATH) \
		   -I$(FILESDKINCPATH) \
		   -I$(LIBEVINCPATH) \
		   -I$(LIBEVENTINCPATH) 
           
		   	
		   
       
	       


 
	   
#CFLAGS += -static	

LIBNAME = $(NETMSGSLIBNAME)
	   

MAKE_DIR=$(PWD)




SRCS :=$(wildcard *.c)
SRC_FILES:=$(notdir $(SRCS))
OBJ_FILES :=$(patsubst %c,%o,$(SRC_FILES))





 
all: $(OBJ_FILES)
	$(AR) cr $(LIBNAME) $(OBJ_FILES)
	@mv $(LIBNAME) $(LIBNETMSGLIBPATH)
#-rm -f *.o
	
.PHONY:clean
clean:
	-rm -f *.o
	
	
	
