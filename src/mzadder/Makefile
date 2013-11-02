
include ../Makefile.config

SRCDIR = ..

LIBS = ../lib/common.o ../util/linefilereader.o ../collection/hashtable.o \
	../collection/strlist.o ../util/strmatch.o ../lib/macro.o ../lib/stringtokenizer.o

DEFS = -I/home/sorenad/Desktop/daydream-2.14.9/main -I/home/sorenad/Desktop/daydream-2.14.9/lib

DDLIB = -L/home/bbs/lib

all: mzadder

mzadder: mzadder.o
	gcc -O2 -o $@ $< $(LIBS) $(DDLIB) -lzip -ldd

clean:
	rm -f *.o *~ mzadder

