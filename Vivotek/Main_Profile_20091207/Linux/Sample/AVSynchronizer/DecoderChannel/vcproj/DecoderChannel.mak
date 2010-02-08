# Makefile - DecoderChannel

SolutionDir = ../vcproj/
SolutionName = DecoderChannel
PlatformName = Linux
ConfigurationName = Release
CFG=Release
CC=gcc
CFLAGS=-m32 -fpic
CXX=g++
CXXFLAGS=$(CFLAGS)
ifeq "$(CFG)" "Debug"
CFLAGS+=  -W  -O0 -fexceptions -g  -fno-inline    -D_DEBUG -D_LINUX  -I ../inc -I  $(SolutionDir)../../../../Bin_MainProfile/inc
LD=$(CXX) $(CXXFLAGS)
LDFLAGS=
LDFLAGS+= -L$(SolutionDir)../../../../Bin_MainProfile/lib
LIBS+=-lAVSynchronizer -lDeinterlaceIpp -lH264DecLib -lImgProcessIppDll -lJpegDecLib -lMpeg4DecDll -lVIVOTEKIpp -lparsedatapacket -lstdc++ -lm
ifndef TARGET
TARGET=DecoderChannel.exe
endif
else
ifeq "$(CFG)" "Release"
CFLAGS+=  -W  -O2 -fexceptions -g  -fno-inline    -DNDEBUG -D_LINUX  -I ../inc -I  $(SolutionDir)../../../../Bin_MainProfile/inc
LD=$(CXX) $(CXXFLAGS)
LDFLAGS=
LDFLAGS+= -L$(SolutionDir)../../../../Bin_MainProfile/lib
LIBS+=-lAVSynchronizer -lDeinterlaceIpp -lH264DecLib -lImgProcessIppDll -lJpegDecLib -lMpeg4DecDll -lVIVOTEKIpp -lparsedatapacket -lstdc++ -lm
ifndef TARGET
TARGET=DecoderChannel.out
endif
endif
endif
ifndef TARGET
TARGET=DecoderChannel.out
endif
.PHONY: all
all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o ../src/$@ -c $<

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o ../src/$@ $<

%.o: %.cxx
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $<

SOURCE_FILES= \
	../src/CDataPacketParser.cpp \
	../src/DecoderChannel.cpp

HEADER_FILES= \
	../src/CDataPacketParser.h

RESOURCE_FILES= \

SRCS=$(SOURCE_FILES) $(HEADER_FILES) $(RESOURCE_FILES) 

OBJS=$(patsubst %.cxx,%.o,$(patsubst %.cpp,%.o,$(patsubst %.cc,%.o,$(patsubst %.c,%.o,$(filter %.c %.cc %.cpp %.cxx,$(SRCS))))))

$(TARGET): $(OBJS)
	if test -d $(SolutionDir)../../$(SolutionName)_Build/$(PlatformName)_$(ConfigurationName)/lib; then true; else mkdir $(SolutionDir)../../$(SolutionName)_Build/$(PlatformName)_$(ConfigurationName)/lib -p; fi
	if test -d $(SolutionDir)../../$(SolutionName)_Build/$(PlatformName)_$(ConfigurationName)/bin; then true; else mkdir $(SolutionDir)../../$(SolutionName)_Build/$(PlatformName)_$(ConfigurationName)/bin -p; fi
	$(LD) $(LDFLAGS) -o $(SolutionDir)../../$(SolutionName)_Build/$(PlatformName)_$(ConfigurationName)/bin/$@ $(OBJS) $(LIBS)
	if test -d $(SolutionDir)../../$(SolutionName)_Build/$(PlatformName)_$(ConfigurationName)/inc; then true; else mkdir $(SolutionDir)../../$(SolutionName)_Build/$(PlatformName)_$(ConfigurationName)/inc -p; fi
	cp testfile_v3.raw $(SolutionDir)../../$(SolutionName)_Build/$(PlatformName)_$(ConfigurationName)/bin/

.PHONY: clean
clean:
	-rm -f -v $(OBJS) $(TARGET) DecoderChannel.dep

.PHONY: depends
depends:
	-$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MM $(filter %.c %.cc %.cpp %.cxx,$(SRCS)) > DecoderChannel.dep

-include DecoderChannel.dep

