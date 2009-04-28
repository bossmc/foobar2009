CC = gcc
CPP = g++
CFLAGS = -g -pg -Wall -pedantic -DDEBUG__
CPPFLAGS = $(CFLAGS) -O0
OBJECTS = foobar2009.o pulsefuncs.o callbacks.o plug-loader.o MainWindow.o PlaylistBrowser.o ColumnBrowser.o SimpleControls.o
LIBRARIES = -lpulse -lfltk

all : foobar2009 utils
	cd Plugins; make;

clean : 
	-rm *.o foobar2009 swapend wavify
	cd Plugins; make clean;

foobar2009 : $(OBJECTS)
	$(CC) -pg -o foobar2009 $(OBJECTS) $(LIBRARIES)

.c.o : $*.h
	$(CC) $(CFLAGS) -c $<

.cpp.o :
	$(CPP) $(CPPFLAGS) -c $<

utils : swapend wavify

swapend : swapend.c
	$(CC) $(FLAGS) -o swapend swapend.c

wavify : wavify.c
	$(CC) $(FLAGS) -o wavify wavify.c
