# Makefile - ePTZ

SolutionDir = ../vcproj/
SolutionName = ePTZ
PlatformName = Linux
ConfigurationName = Release
CFG=Release
CC=gcc
CFLAGS=-m32 -fpic
CXX=g++
CXXFLAGS=$(CFLAGS)
ifeq "$(CFG)" "Debug"
CFLAGS+=  -W  -O0 -fexceptions -g  -fno-inline  -D_CRT_SECURE_NO_WARNINGS  -D_DEBUG -D_LIB -D_LINUX  -I ../inc -I ../../../../Bin_MainProfile/inc -I "$(SolutionDir)../../$(SolutionName)_Build/$(PlatformName)_$(ConfigurationName)/inc" -I /usr/local/include
LD=$(CXX) $(CXXFLAGS)
LDFLAGS=
LDFLAGS+= -L../../../../Bin_MainProfile/lib -L"$(SolutionDir)../../$(SolutionName)_Build/$(PlatformName)_$(ConfigurationName)/lib" 
LIBS+=-lServerManager  -l$(NOINHERIT) -lstdc++ -lm
ifndef TARGET
TARGET=ePTZ.exe
endif
else
ifeq "$(CFG)" "Release"
CFLAGS+=  -Wall -W -Wno-error  -O3 -fexceptions -g  -fno-inline  -D_CRT_SECURE_NO_WARNINGS  -D_NDEBUG -D_LIB -D_LINUX  -I ../inc -I ../../../../Bin_MainProfile/inc -I "$(SolutionDir)../../$(SolutionName)_Build/$(PlatformName)_$(ConfigurationName)/inc" -I /usr/local/include
LD=$(CXX) $(CXXFLAGS)
LDFLAGS=
LDFLAGS+= -L../../../../Bin_MainProfile/lib -L"$(SolutionDir)../../$(SolutionName)_Build/$(PlatformName)_$(ConfigurationName)/lib" 
LIBS+=-lServerManager  -lstdc++ -lm
ifndef TARGET
TARGET=ePTZ.exe
endif
endif
endif
ifndef TARGET
TARGET=ePTZ.exe
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
	../src/main.cpp

HEADER_FILES= \
	../inc/ePTZ.h

RESOURCE_FILES= \

SRCS=$(SOURCE_FILES) $(HEADER_FILES) $(RESOURCE_FILES) 

OBJS=$(patsubst %.cxx,%.o,$(patsubst %.cpp,%.o,$(patsubst %.cc,%.o,$(patsubst %.c,%.o,$(filter %.c %.cc %.cpp %.cxx,$(SRCS))))))

$(TARGET): $(OBJS)
	if test -d $(SolutionDir)../../$(SolutionName)_Build/$(PlatformName)_$(ConfigurationName)/lib; then true; else mkdir $(SolutionDir)../../$(SolutionName)_Build/$(PlatformName)_$(ConfigurationName)/lib -p; fi
	if test -d $(SolutionDir)../../$(SolutionName)_Build/$(PlatformName)_$(ConfigurationName)/bin; then true; else mkdir $(SolutionDir)../../$(SolutionName)_Build/$(PlatformName)_$(ConfigurationName)/bin -p; fi
	$(LD) $(LDFLAGS) -o $(SolutionDir)../../$(SolutionName)_Build/$(PlatformName)_$(ConfigurationName)/bin/$@ $(OBJS) $(LIBS)
	

.PHONY: clean
clean:
	-rm -f -v $(OBJS) $(TARGET) ePTZ.dep

.PHONY: depends
depends:
	-$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MM $(filter %.c %.cc %.cpp %.cxx,$(SRCS)) > ePTZ.dep

-include ePTZ.dep

