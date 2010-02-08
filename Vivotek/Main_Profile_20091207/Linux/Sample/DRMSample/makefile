##

TARGET 		= SampleCode.out

vpath %.h ../../Bin_MainProfile/inc
vpath %.a ../../lib

INC_FLAGS	= -I../../Bin_MainProfile/inc
LIB_FLAGS	= -L../../Bin_MainProfile/lib -lDRMControl -lpthread -lm

SRCS = SampleCode.c
OBJS = SampleCode.o
CC = gcc

libsrc: $(OBJS)
	$(CC) $(OBJS) -O0 -g $(CFLAGS) $(INC_FLAGS) -o $(TARGET) -Wl,--start-group $(LIB_FLAGS) -Wl,--end-group

$(OBJS): $(SRCS)
	$(CC) -O0 -g  -D_REENTRANT -D_LINUX $(CFLAGS) $(INC_FLAGS) -c $(SRCS) -o $(OBJS)
