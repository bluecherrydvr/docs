#Makefile for Solo6010 card demo

CC 	   = gcc -O3

LIBS   = `pkg-config --libs gtk+-2.0 glib-2.0 gthread-2.0` -Wl,-rpath,/usr/lib -lSDL -lpthread
FLAGS  = `pkg-config --cflags gtk+-2.0 glib-2.0 gthread-2.0`

TARGET: scodedemo

scodedemo: scodedemo.c
	rm -f scodedemo
	$(CC) $(FLAGS) $< -o $@ $(LIBS) -lscode

clean:
	rm  -f scodedemo 
	
