CFLAGS +=  -I$(ISILSDKPATH)include  \
			-I$(EVENTCOREINCPATH) \
			-I$(MEDIASDKINCPATH) \
			-I$(ISILSDKINCPATH) \
			-I$(LIBEVENTINCPATH) 
           
	       


H264_SUB_GET_FRAME_NAME = h264_sub_get_frame
H264_SUB_FILE_STORE_NANE = h264_sub_file_store


	   
	   
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
	$(CC) $(CFLAGS) -o $(H264_SUB_GET_FRAME_NAME) $(H264_SUB_GET_FRAME_NAME).o $(LIBPATH) $(DVRLIB)
	$(CC) $(CFLAGS) -o $(H264_SUB_FILE_STORE_NANE) $(H264_SUB_FILE_STORE_NANE).o $(LIBPATH) $(DVRLIB)
    
#	$(AR) cr $(LIBNAME) $(OBJ_FILES)
#	@mv $(LIBNAME) $(TOPLPATH)
#-rm -f *.o
	
.PHONY:clean
clean:
	-rm -f *.o $(H264_SUB_GET_FRAME_NAME) $(H264_SUB_FILE_STORE_NANE)
	
	
	
