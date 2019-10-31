CC=gcc
CFLAGS=-g -std=gnu99 -O3 -Wall #-DPTH

all : libspin.so libcounter.so liblist.so libhash.so
spin.o : spin.c
	$(CC) $(CFLAGS) -c -fpic spin.c

libspin.so: spin.o
	$(CC) $(CFLAGS) -shared -o $@ $<

counter.o : counter.c libspin.so
	$(CC) $(CFLAGS) -c -fpic counter.c

libcounter.so : counter.o
	$(CC) $(CFLAGS) -shared -o $@ $<

list.o: list.c libspin.so
	$(CC) $(CFLAGS) -c -fpic list.c

liblist.so: list.o
	$(CC) $(CFLAGS) -shared -o $@ $<

hash.o: hash.c libspin.so
	$(CC) $(CFLAGS) -c -fpic hash.c

libhash.so: hash.o
	$(CC) $(CFLAGS) -shared -o $@ $<

clean:
	rm -f *.o
	rm -f *.so
