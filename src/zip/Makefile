
include ../Makefile.config

SRCDIR = ..

OBJS = ../collection/strlist.o ../lib/common.o ../lib/macro.o ../collection/hashtable.o \
	../util/linefilereader.o mod_zip.o ../lib/stringtokenizer.o zip.o ../unzip/unzip.o \
	../checker/util.o ../lib/sfv.o ../lib/dirlist.o

all: mod_zip.o zip.o zipcheck

zipcheck: zipcheck.o
	gcc -O2 -o $@ $< $(OBJS) -lz

clean:
	rm -f *~ *.o zipcheck


