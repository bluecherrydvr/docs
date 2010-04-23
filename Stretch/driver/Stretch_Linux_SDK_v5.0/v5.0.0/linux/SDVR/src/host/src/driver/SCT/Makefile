OUTPUT_DIR?=../../../bin/linux
include ../../../make.cfg
TARGET = $(OUTPUT_LIB)/libsct.a
OBJECTS_1 = sct_board.o \
          sct_channel.o \
          sct.o

OBJECTS = $(foreach obj, $(OBJECTS_1), $(addprefix $(OUTPUT_OBJ)/, $(obj)))

ifdef DEBUG
DBGFLAGS = -g
else
DBGFLAGS = -O3
endif

EXTRA_CPPFLAGS += -I../linux/include -I ../../../../common_include/sct
EXTRA_CPPFLAGS += $(DBGFLAGS)
LINKOPTIONS = 

CPPFLAGS = -MMD \
           -fPIC \
           $(EXTRA_CPPFLAGS)

CPPFLAGS += -pipe \
           -Wall

all:: $(OUTPUT_INC) $(OUTPUT_LIB) $(TARGET) $(OUTPUT_INC)/sct.h

$(OUTPUT_INC)/sct.h : ../../../../common_include/sct/sct.h
	cp -fp $< $(OUTPUT_INC)

$(TARGET): $(OUTPUT_OBJ) $(OBJECTS)
	$(AR) r $@ $(OBJECTS)

$(OUTPUT_OBJ)/sct_board.o : sct_board.cpp sct_internal.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

$(OUTPUT_OBJ)/sct_channel.o : sct_channel.cpp sct_internal.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

$(OUTPUT_OBJ)/sct.o : sct.cpp sct_internal.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

clean::
	$(RM) $(OBJECTS)
	$(RM) $(subst .o,.d,$(OBJECTS))
	$(RM) $(TARGET)
	$(RM) $(OUTPUT_INC)/sct.h

include ../../../rule.cfg
