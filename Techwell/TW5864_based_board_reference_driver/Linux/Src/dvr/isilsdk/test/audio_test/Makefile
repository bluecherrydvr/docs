CFLAGS +=  -I$(ISILSDKPATH)include  \
			-I$(EVENTCOREINCPATH) \
			-I$(MEDIASDKINCPATH) \
			-I$(ISILSDKINCPATH) \
			-I$(LIBEVENTINCPATH) 
           
	       


AUDIO_GET_FRAME_NAME = audio_get_frame
AUDIO_FILE_STORE = audio_file_store

	   
	   
#CFLAGS += -static	

	   

MAKE_DIR=$(PWD)


LIBPATH = -L$(ISILSDKLIBPATH) \
		  -L$(LIBEVENTLIBPATH) 
	  


DVRLIB = -l$(LINKISILMEDIASLIBNAME)\
         -lpthread\
         -levent
		 
         


SRCS :=$(wildcard *.c)
SRC_FILES:=$(notdir $(SRCS))
OBJ_FILES :=$(patsubst %c,%o,$(SRC_FILES))





 
all: $(OBJ_FILES)
	@echo -e "lib $(DVRDIR)"
	$(CC) $(CFLAGS) -o $(AUDIO_GET_FRAME_NAME) $(AUDIO_GET_FRAME_NAME).o $(LIBPATH) $(DVRLIB)
	
	$(CC) $(CFLAGS) -o $(AUDIO_FILE_STORE) $(AUDIO_FILE_STORE).o $(LIBPATH) $(DVRLIB)
	
    

	
.PHONY:clean
clean:
	-rm -f *.o $(AUDIO_GET_FRAME_NAME) $(AUDIO_FILE_STORE)
	
	
	
