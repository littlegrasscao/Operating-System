CC=gcc
CFLAGS=-g -std=gnu99 -O3 -Wall

all : libmfs.so server

mfs.o : mfs.c
	$(CC) $(CFLAGS) -c -fpic mfs.c

libmfs.so : mfs.o
	$(CC) $(CFLAGS) -shared -o $@ $<

superBlock.o : superBlock.c
	$(CC) $(CFLAGS) -c superBlock.c
	
inodeBitmap.o : inodeBitmap.c
	$(CC) $(CFLAGS) -c inodeBitmap.c

dataBitmap.o : dataBitmap.c
	$(CC) $(CFLAGS) -c dataBitmap.c

datablock.o : datablock.c
	$(CC) $(CFLAGS) -c datablock.c

inode.o : inode.c
	$(CC) $(CFLAGS) -c inode.c
	
udp.o : udp.c
	$(CC) $(CFLAGS) -c udp.c
	
server : server.o superBlock.o udp.o inode.o datablock.o dataBitmap.o inodeBitmap.o 
	$(CC) $(CFLAGS) $< superBlock.o inodeBitmap.o dataBitmap.o datablock.o inode.o udp.o -o $@ -L. -lpthread -lm

clean :
	rm -rf *.o *.so
