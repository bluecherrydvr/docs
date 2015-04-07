CFLAGS +=  -I$(LIBPBINCPATH)  \
		   -I$(NETAPPPATH)include  \
		   -I$(LIBNETMSGINCPATH)  \
		   -I$(ISILSDKPATH)include  \
		   -I$(EVENTCOREINCPATH) \
		   -I$(MEDIASDKINCPATH) \
		   -I$(ISILSDKINCPATH) \
		   -I$(NETCOREINCPATH) \
		   -I$(FILESDKPATH)include \
		   -I$(LIBEVINCPATH) \
		   -I$(LIBEVENTINCPATH) 
           
	       


EXECFILENAME = test_enc_1
	  
TEST_CHIP_CHAN_MAP = test_chip_chan_map
 
	   
#CFLAGS += -static	

	   

MAKE_DIR=$(PWD)


LIBPATH = -L$(LIBPBBPATH) \
		  -L$(ISILNETSDKLIBPATH) \
	      -L$(LIBNETMSGLIBPATH) \
	      -L$(ISILSDKLIBPATH) \
	      -L$(FILESDKLIBPATH) \
	      -L$(LIBEVLIBPATH) \
	      -L$(LIBEVENTLIBPATH)
	  


DVRLIB = -l$(LINKPBSLIBNAME) \
		 -l$(LINKNETCORESLIBNAME) \
         -l$(LINKISILFILESDKSLIBNAME) \
		 -l$(LINKNETMSGSLIBNAME) \
         -l$(LINKISILMEDIASLIBNAME) \
         -lpthread \
         -levent \
         -lev \
         -lm


SRCS :=$(wildcard *.c)
SRC_FILES:=$(notdir $(SRCS))
OBJ_FILES :=$(patsubst %c,%o,$(SRC_FILES))





 
all: $(OBJ_FILES)
	@echo -e "lib $(DVRDIR)"
	$(CC) $(CFLAGS) -o $(EXECFILENAME) $(EXECFILENAME).o $(LIBPATH) $(DVRLIB) #-L$(DVMFFMPEGLIB) $(FFMPEGLIBS)
	$(CC) $(CFLAGS) -o $(TEST_CHIP_CHAN_MAP) $(TEST_CHIP_CHAN_MAP).o $(LIBPATH) $(DVRLIB)
	
    
#	$(AR) cr $(LIBNAME) $(OBJ_FILES)
#	@mv $(LIBNAME) $(TOPLPATH)
#-rm -f *.o
	
.PHONY:clean
clean:
	-rm -f *.o $(EXECFILENAME) $(TEST_CHIP_CHAN_MAP)
	
	
	
