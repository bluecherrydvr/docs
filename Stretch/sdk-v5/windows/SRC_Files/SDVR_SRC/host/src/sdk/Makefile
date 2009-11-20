OUTPUT_DIR?=../../bin/linux
include ../../make.cfg

LIBNAME = sdvr_sdk
UI_LIB = sdvr_ui_sdk
TARGET=$(OUTPUT_LIB)/lib$(LIBNAME).a
UI_TARGET=$(OUTPUT_LIB)/lib$(UI_LIB).a
OBJECTS_1=bytestrm.o\
		channels.o\
		dbg_cmd_name.o\
		debug.o\
		decoder.o\
		display.o\
		encoder.o\
		g711.o\
		mp4mux.o\
		mp4read.o\
		mp4write.o\
		osd.o\
		ptz.o\
		sdk.o\
		sdk_utils.o\
		sensor_relays.o\
		sha1.o\
		smo.o\
		venc264_authenticate.o\
		venc264_SCE.o\
		writer.o\
		utils.o

OBJECTS = $(foreach obj, $(OBJECTS_1), $(addprefix $(OUTPUT_OBJ)/, $(obj)))

CFLAGS = -I../../include -I../../../common_include -I../../../common_include/sct  -I../codecs/h264
ifdef DEBUG
	CFLAGS += -g
else
	CFLAGS += -O3
endif

all : $(TARGET) $(OUTPUT_INC)/$(LIBNAME).h $(UI_TARGET)

$(UI_TARGET) :	
	make -C $(UI_LIB)

clean :
	rm -f $(OBJECTS)
	rm -f $(TARGET)
	rm -f $(OUTPUT_INC)/$(LIBNAME).h
	make -C $(UI_LIB) clean

$(TARGET) : $(OBJECTS)
	$(AR) r $@ $(OBJECTS) 

$(OBJECTS) : Makefile

$(OUTPUT_OBJ)/bytestrm.o	:	../codecs/avmux/mp4/bytestrm.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(OUTPUT_OBJ)/g711.o	:	../codecs/g711/g711.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(OUTPUT_OBJ)/mp4mux.o	:	../codecs/avmux/mp4/mp4mux.c
	$(CC) $(CFLAGS) -o $@ -c $<
	
$(OUTPUT_OBJ)/mp4read.o	:	../codecs/avmux/mp4/mp4read.c
	$(CC) $(CFLAGS) -o $@ -c $<
	
$(OUTPUT_OBJ)/utils.o	:	../codecs/avmux/mp4/utils.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(OUTPUT_OBJ)/mp4write.o	:	../codecs/avmux/mp4/mp4write.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(OUTPUT_OBJ)/sha1.o : ../codecs/h264/sha1.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(OUTPUT_OBJ)/venc264_authenticate.o : ../codecs/h264/venc264_authenticate.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(OUTPUT_OBJ)/venc264_SCE.o : ../codecs/h264/venc264_SCE.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(OUTPUT_OBJ)/%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(OUTPUT_INC)/%.h	: ../../include/%.h
	cp -pf $< $@

include ../../rule.cfg
