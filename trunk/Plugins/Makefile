CC = gcc
CPP = g++
CFLAGS = -g -Wall -pedantic -DDEBUG__ -fPIC
PLUGINS = plug-libmad.so plug-libvorbisfile.so plug-wav.so plug-libflac.so

all : $(PLUGINS)

clean :
	-rm *.o *.so

bstdfile.o : bstdfile.c bstdfile.h
	$(CC) $(CFLAGS) -c bstdfile.c
	
MP3_header.o : MP3_header.c MP3_header.h
	$(CC) $(CFLAGS) -c MP3_header.c

plug-libmad.so : plug-libmad.c plug-libmad.h plugin_api.h bstdfile.o MP3_header.o
	g++ $(CFLAGS) -c plug-libmad.c
	g++ -shared -o plug-libmad.so plug-libmad.o bstdfile.o MP3_header.o -lmad -lm -lid3

plug-libvorbisfile.so : plug-libvorbisfile.c plug-libvorbisfile.h plugin_api.h
	$(CC) $(CFLAGS) -c plug-libvorbisfile.c
	$(CC) -shared -o plug-libvorbisfile.so plug-libvorbisfile.o -lvorbisfile
  
plug-wav.so : plug-wav.c plug-wav.h plugin_api.h
	$(CC) $(CFLAGS) -c plug-wav.c
	$(CC) -shared -o plug-wav.so plug-wav.o

plug-libflac.so : plug-libflac.c plugin_api.h
	$(CC) $(CFLAGS) -c plug-libflac.c
	$(CC) -shared -o plug-libflac.so plug-libflac.o -lFLAC
