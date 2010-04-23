# Needed for Build Path definitions
OUTPUT_DIR ?= $(abspath ../../bin/linux)
include ../../make.cfg

# Variables to build it
CXXFLAGS = -pipe -O2 -Wall -W -D_REENTRANT
LDFLAGS = $(INC_PATH) $(INC_LIBS)
INC_LIBS = -L$(OUTPUT_LIB) -lsdvr_sdk -lsct -lpthread -laio -lrt
INC_PATH = -I$(OUTPUT_INC)

OBJECT = sdvr_diag

all : $(OUTPUT_BIN) $(OUTPUT_BIN)/$(OBJECT)

$(OUTPUT_BIN)/$(OBJECT): sdvr_diag.cpp
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

clean:
	rm -rf $(OUTPUT_BIN)/$(OBJECT)
