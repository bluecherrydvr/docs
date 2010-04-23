OUTPUT_DIR?=../../../../bin/linux
include ../../../../make.cfg
TARGET = $(OUTPUT_BIN)/lintest
ROM=sctdemo_s6.rom
OBJECTS = $(OUTPUT_OBJ)/linsct_test.o

EXTRA_CPPFLAGS += -I../include -I ../../../../../common_include/sct
EXTRA_CPPFLAGS += $(DBGFLAGS)
LIBPATH = -L$(OUTPUT_LIB)
LIBS = -lsct -lpthread -lrt -laio

CPPFLAGS = -MMD \
           -fPIC \
           $(EXTRA_CPPFLAGS)

CPPFLAGS += -pipe \
           -g \
           -Wall

LINKOPTIONS := $(LIBPATH) $(LIBS)

all:: $(OUTPUT_BIN) $(TARGET) $(OUTPUT_BIN)/$(ROM)

$(TARGET): $(OUTPUT_OBJ) $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LINKOPTIONS)

$(OUTPUT_OBJ)/linsct_test.o : linsct_test.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

$(OUTPUT_BIN)/$(ROM) : $(ROM)
	cp -f $(ROM) $(OUTPUT_BIN)

clean::
	$(RM) $(OBJECTS)
	$(RM) $(subst .o,.d,$(OBJECTS))
	$(RM) $(TARGET)
	$(RM) $(OUTPUT_BIN)/$(ROM)

test: $(TARGET)
	$(OUTPUT_BIN)/lintest

include ../../../../rule.cfg
