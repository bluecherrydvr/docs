#Makefile for lib

CC      =  gcc   
CFLAGS  =  -Wall   -O3  -fPIC   
    
all: libscode
    
libscode: scode.o capture.o
	$(CC) -shared -Wl,-soname,libscode.so -o libscode.so $^
	strip libscode.so

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

install: 
	cp libscode.so /usr/lib/

clean:   
	rm   -fr   *.o   
	rm   -fr   *.so*   
