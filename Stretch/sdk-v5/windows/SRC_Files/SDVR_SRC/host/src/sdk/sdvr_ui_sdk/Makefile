OUTPUT_DIR?=../../../bin/linux
include ../../../make.cfg

LIBNAME = sdvr_ui_sdk
TARGET=$(OUTPUT_LIB)/lib$(LIBNAME).a
OBJECTS=yuvdraw.o\
		yuvdraw_x11.o

OBJECTS:=$(foreach obj, $(OBJECTS), $(addprefix $(OUTPUT_OBJ)/, $(obj)))

CFLAGS = -I../../../include -I../../../../common_include -I.. -I../../../../common_include/sct -Werror
ifdef DEBUG
	CFLAGS += -g
else
	CFLAGS += -O3
endif

ifeq ($(SKIP_UI),y)
all :
	@echo "skip build $(LIBNAME) for $(TARGET_SYS)"
else	
all : $(TARGET) $(OUTPUT_INC)/$(LIBNAME).h
endif	

clean :
	rm -f $(OBJECTS)
	rm -f $(TARGET)
	rm -f $(OUTPUT_INC)/$(LIBNAME).h

$(TARGET) : $(OBJECTS)
	$(AR) r $@ $(OBJECTS) 

$(OBJECTS) : Makefile

$(OUTPUT_OBJ)/%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(OUTPUT_INC)/%.h	: ../../../include/%.h
	cp -pf $< $@

include ../../../rule.cfg
