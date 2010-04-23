STRTPIO_DIR = ../../src/svc/stRtpIO
EXTRACTOR_DIR = ../../../../../codecs/svc/extractor
LIVE_DIR = $(STRTPIO_DIR)/live/src
INCLUDES = -I$(STRTPIO_DIR) -I../../include -I.

##### Change the following for your environment: 
COMPILE_OPTS =		$(INCLUDES) -I. -O2 -D__LINUX__
C =			c
C_COMPILER =		gcc
C_FLAGS =		$(COMPILE_OPTS)
CPP =			cpp
CPLUSPLUS_COMPILER =	g++
CPLUSPLUS_FLAGS =	$(COMPILE_OPTS) -Wall -DBSD=1
OBJ =			o
LINK =			g++ -o 
LINK_OPTS =		-L. 
CONSOLE_LINK_OPTS =	$(LINK_OPTS)
LIBRARY_LINK =		ld -o
LIBRARY_LINK_OPTS =	$(LINK_OPTS) -r -Bstatic
LIB_SUFFIX =			a
LIBS_FOR_CONSOLE_APPLICATION =
LIBS_FOR_GUI_APPLICATION =
EXE =
##### End of variables to change

SVC_ROUTER_APP = svcRouter$(EXE) 

ALL = $(SVC_ROUTER_APP) 
all: $(ALL)

.$(C).$(OBJ):
	$(C_COMPILER) -c $(C_FLAGS) $<       

.$(CPP).$(OBJ):
	$(CPLUSPLUS_COMPILER) -c $(CPLUSPLUS_FLAGS) $<

SVC_ROUTER_OBJS = st_svc_router_test_1t.$(OBJ) cfg.$(OBJ)

STRTPIO_LIB = $(STRTPIO_DIR)/libstRtpIO.$(LIB_SUFFIX)
EXTRACTOR_LIB = $(EXTRACTOR_DIR)/libextractor.$(LIB_SUFFIX)      
LIVE_LIB = $(LIVE_DIR)/liveMedia/libliveMedia.$(LIB_SUFFIX) \
           $(LIVE_DIR)/UsageEnvironment/libUsageEnvironment.$(LIB_SUFFIX) \
           $(LIVE_DIR)/BasicUsageEnvironment/libBasicUsageEnvironment.$(LIB_SUFFIX) \
           $(LIVE_DIR)/groupsock/libgroupsock.$(LIB_SUFFIX) 

SVC_ROUTER_LIBS = $(STRTPIO_LIB) $(EXTRACTOR_LIB) $(LIVE_LIB)

svcRouter$(EXE):	$(SVC_ROUTER_OBJS) $(SVCROUTER_LIBS)
		pushd $(STRTPIO_DIR); make
		pushd $(EXTRACTOR_DIR); make
		pushd $(LIVE_DIR)/liveMedia; make
		pushd $(LIVE_DIR)/UsageEnvironment; make
		pushd $(LIVE_DIR)/BasicUsageEnvironment; make
		pushd $(LIVE_DIR)/groupsock; make; 
	    $(LINK)$@ -lpthread $(SVC_ROUTER_OBJS) $(SVC_ROUTER_LIBS)

clean:
	-rm -rf *.$(OBJ) $(ALL) core *.core *~ include/*~

##### Any additional, platform-specific rules come here:
