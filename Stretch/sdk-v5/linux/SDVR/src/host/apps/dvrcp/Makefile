OUTPUT_DIR?=$(shell cd ../../bin/linux;pwd)
include ../../make.cfg
ARCH_M := $(shell uname -m)
ifeq ($(ARCH_M),x86_64)
QT_DIR :=/usr/lib64/qt4
else
QT_DIR :=/usr/lib/qt4
endif
QMAKE_GUESS = $(QT_DIR)/bin/qmake
QMAKE = $(shell if [ -f $(QMAKE_GUESS) ]; then echo $(QMAKE_GUESS); else which qmake;fi)
QMAKE_CONFIGOPTS += OUTPUT_BIN=$(OUTPUT_BIN)
QMAKE_CONFIGOPTS += OUTPUT_OBJ=$(OUTPUT_OBJ)/dvrcp
QMAKE_CONFIGOPTS += SDK_LIB=$(OUTPUT_LIB)
ifdef DEBUG
QMAKE_CONFIGOPTS += CONFIG+=debug
endif 
#WARNINGOPTS = "QMAKE_CFLAGS+=-Werror" "QMAKE_CXXFLAGS+=-Werror"
QMAKE_CONFIGOPTS += $(WARNINGOPTS)

all	: dvrcp

dvrcp	: dvrcp.pro $(OUTPUT_OBJ)/dvrcp $(OUTPUT_BIN) $(wildcard *.cpp) $(wildcard *.h)
	$(QMAKE) dvrcp.pro -o Makefile.qt $(QMAKE_CONFIGOPTS)
	make -f Makefile.qt
	cp -f config_enc.def $(OUTPUT_BIN)

clean :
	$(QMAKE) dvrcp.pro -o Makefile.qt $(QMAKE_CONFIGOPTS)
	make -f Makefile.qt compiler_clean
	make -f Makefile.qt clean
	rm -f $(OUTPUT_BIN)/config_enc.def
	rm -rf Makefile.qt

run :
		$(OUTPUT_BIN)/dvrcp

include ../../rule.cfg

$(OUTPUT_OBJ)/dvrcp	:
	$(MKDIR) $@


