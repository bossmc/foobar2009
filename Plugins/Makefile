CC = gcc
CPP = g++
CFLAGS = -g -pg -Wall -pedantic -DDEBUG__
CFLAGSPLUG = -fPIC
PLUGINS = plug-libmad.so plug-libvorbisfile.so plug-wav.so


all : $(PLUGINS)

clean :
	-rm *.o *.so

bstdfile.o : bstdfile.c bstdfile.h
	$(CC) $(CFLAGS) -c bstdfile.c
	
MP3_header.o : MP3_header.c MP3_header.h
	$(CC) $(CFLAGS) -c MP3_header.c

plug-libmad.so : plug-libmad.c plug-libmad.h plugin_api.h bstdfile.o MP3_header.o
	g++ $(CFLAGS) $(CFLAGSPLUG) -c plug-libmad.c
	g++ -shared -o plug-libmad.so plug-libmad.o bstdfile.o MP3_header.o -lmad -lm -lid3

plug-libvorbisfile.so : plug-libvorbisfile.c plug-libvorbisfile.h plugin_api.h
	$(CC) $(CFLAGS) $(CFLAGSPLUG) -c plug-libvorbisfile.c
	$(CC) -shared -o plug-libvorbisfile.so plug-libvorbisfile.o -lvorbisfile
  
plug-wav.so : plug-wav.c plug-wav.h plugin_api.h
	$(CC) $(CFLAGS) $(CFLAGSPLUG) -c plug-wav.c
	$(CC) -shared -o plug-wav.so plug-wav.o