#---------------------------------------------------------------
# May 1, 2002 Joe Briggs
#--------------------------------------------------------------

CFLAGS = -Wall
TARGET = pelco

all: $(TARGET)

install: all
	install -m 0755 $(TARGET) /usr/local/bin

clean:
	rm -f *.o $(TARGET)
