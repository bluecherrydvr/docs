CFLAGS +=  -I$(ISILSDKPATH)include  \
			-I$(EVENTCOREINCPATH) \
			-I$(MEDIASDKINCPATH) \
			-I$(ISILSDKINCPATH) \
			-I$(LIBEVENTINCPATH) 
           
	       


MJPEG_GET_FRAME_NAME = mjpeg_get_frame
MJPEG_FILE_STORE_NAME = mjpeg_file_store

	   
	   
#CFLAGS += -static	

	   

MAKE_DIR=$(PWD)


LIBPATH = -L$(ISILSDKLIBPATH) \
		  -L$(LIBEVENTLIBPATH) 
	  


DVRLIB = -l$(LINKISILMEDIASLIBNAME) \
         -lpthread \
         -levent 
         


SRCS :=$(wildcard *.c)
SRC_FILES:=$(notdir $(SRCS))
OBJ_FILES :=$(patsubst %c,%o,$(SRC_FILES))





 
all: $(OBJ_FILES)
	@echo -e "lib $(DVRDIR)"
	$(CC) $(CFLAGS) -o $(MJPEG_GET_FRAME_NAME) $(MJPEG_GET_FRAME_NAME).o $(LIBPATH) $(DVRLIB)
	$(CC) $(CFLAGS) -o $(MJPEG_FILE_STORE_NAME) $(MJPEG_FILE_STORE_NAME).o $(LIBPATH) $(DVRLIB)
	
    
#	$(AR) cr $(LIBNAME) $(OBJ_FILES)
#	@mv $(LIBNAME) $(TOPLPATH)
#-rm -f *.o
	
.PHONY:clean
clean:
	-rm -f *.o $(MJPEG_GET_FRAME_NAME) $(MJPEG_FILE_STORE_NAME)
	
	
	
