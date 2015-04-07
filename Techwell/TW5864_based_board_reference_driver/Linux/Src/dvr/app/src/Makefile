

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
           
	       


EXECFILENAME = dvr-$(PLATFORM)	   
#EXECFILENAME = dvr-ppc
	   
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
	$(CC) $(CFLAGS) -o $(EXECFILENAME) $(OBJ_FILES) $(LIBPATH) $(DVRLIB) #-L$(DVMFFMPEGLIB) $(FFMPEGLIBS)
	
    
#	$(AR) cr $(LIBNAME) $(OBJ_FILES)
#	@mv $(LIBNAME) $(TOPLPATH)
#-rm -f *.o
	
.PHONY:clean
clean:
	-rm -f *.o
	
	
	
