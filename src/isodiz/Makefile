
include ../Makefile.config

SRCDIR = ..

LIBS = ../lib/sfv.o ../lib/common.o ../util/linefilereader.o

all: isodiz

isodiz: isodiz.o
	gcc -O2 -o $@ $< $(LIBS)

clean:
	rm -f *.o *~ isodiz

