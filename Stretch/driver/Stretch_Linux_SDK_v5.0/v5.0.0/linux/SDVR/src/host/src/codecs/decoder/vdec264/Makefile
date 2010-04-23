all:.all

SLIB =./lib/vdec264.lib
.all: $(SLIB)

CC = gcc
SRC_DIR = src
I386_DIR = i386

SRCS_C=\
	$(SRC_DIR)/bitstream.c \
	$(SRC_DIR)/cabac.c \
	$(SRC_DIR)/dsputil.c \
	$(SRC_DIR)/ls.c \
	$(SRC_DIR)/vdec264.c \
	$(SRC_DIR)/error_resilience.c \
	$(SRC_DIR)/golomb.c \
	$(SRC_DIR)/h264.c \
	$(SRC_DIR)/h264_parser.c \
	$(SRC_DIR)/h264idct.c \
	$(SRC_DIR)/h264pred.c \
	$(SRC_DIR)/imgconvert.c \
	$(SRC_DIR)/log.c \
	$(SRC_DIR)/mem.c \
	$(SRC_DIR)/mpegvideo.c \
	$(SRC_DIR)/parser.c \
	$(SRC_DIR)/rational.c \
	$(SRC_DIR)/utils.c \
	$(I386_DIR)/dsputil_mmx.c \
	$(I386_DIR)/cpuid.c
	

CFLAGS += -I. -I./include \
         -D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112 -std=gnu99 -fasm
CFLAGS += -mno-cygwin -mthreads -pipe
CFLAGS += -mdll

#64BIT = yes
#DEBUG = yes
SSE = yes
SSE2 = yes

YASM=yasm.exe

ifeq ($(64BIT),yes)
    TARGET_OS = x86_64-pc-mingw32
else
    TARGET_OS = i686-pc-mingw32
endif

OUTPUTFLAG=-MMD -o\

ifeq ($(64BIT),yes)
    CFLAGS += -DWIN64 -D_WIN64
else
    CFLAGS += -DWIN32 -D_WIN32
endif

ifneq ($(64BIT),yes)
CFLAGS += -mmmx
endif

ifeq ($(SSE),yes)
    CFLAGS += -msse -mfpmath=sse
endif
ifeq ($(SSE2),yes)
    CFLAGS += -msse2
endif

ifeq ($(DEBUG),yes)
    CFLAGS += -DDEBUG -D_DEBUG -g 
    OPTFLAGS = -O2 -fomit-frame-pointer
    YASMFLAGS += -g cv8
else
    CFLAGS += -DNDEBUG -UDEBUG
    OPTFLAGS = -O3 -fomit-frame-pointer
    LDFLAGS = --strip-all
endif

ifeq ($(64BIT),yes)
    OPTFLAGS += -m64
else
    OPTFLAGS += -march=i686
endif

CPPFLAGS += -fno-rtti

GCCDEF = $(DEF)

ifeq ($(UNICODE),yes)
    CFLAGS += -DUNICODE -D_UNICODE
endif

OBJS += $(SRCS:.cpp=.o) $(SRCS_C:.c=.o) $(SRCS_YASM:.asm=.o) 

ifeq ($(FORCECPP),1)
    CFLAGS += -x c++
endif

ifeq ($(EXCEPTIONS),1)
    CPPFLAGS += -fexceptions
else
    CPPFLAGS += -fno-exceptions
endif

%.o: %.asm
ifeq ($(64BIT),yes)
	$(YASM) -f win64 -DWIN64 -DARCH_X86_64 -DPREFIX $(YASMFLAGS) -I$(<D)/ -o $@  $<
else
	$(YASM) -f win32 -DWIN32 -DARCH_X86_32 -DPREFIX $(YASMFLAGS) -I$(<D)/ -o $@  $<
endif

$(SLIB): $(OBJS) 
	ar rcs $@ $(OBJS) 

distclean: clean

#CFLAGS+= -save-temps
#CFLAGS+= -fverbose-asm

ifeq ($(64BIT),yes)
    CFLAGS += -DARCH_X86_64
else
    CFLAGS += -DARCH_X86_32
endif


OPTFLAGS+= -fno-math-errno
#OPTFLAGS+= -fno-signed-zeros

.c.o:
	$(CC) -c $(CFLAGS) $(OPTFLAGS) $(OUTPUTFLAG)$@ $<

.cpp.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(OPTFLAGS) $(OUTPUTFLAG)$@ $<


clean:
	rm -f $(SRC_DIR)/*.o $(SRC_DIR)/*.d $(I386_DIR)/*.o $(I386_DIR)/*.d $(SLIB)

#
# include dependency files if they exist
#
-include $(SRCS:.cpp=.d) $(SRCS_C:.c=.d)
