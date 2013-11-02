
include ../Makefile.config

SRCDIR = ..

LIBS = ../lib/sfv.o ../lib/common.o ../util/linefilereader.o

all: createsfv blehsfv

createsfv: createsfv.o
	gcc -O2 -o $@ $< $(LIBS)

blehsfv: blehsfv.o
	gcc -O2 -o $@ $< $(LIBS)

clean:
	rm -f *.o *~ createsfv blehsfv

