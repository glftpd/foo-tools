
include ../Makefile.config

SRCDIR = ..

OBJS = ../collection/strlist.o ../lib/macro.o ../collection/hashtable.o \
	../util/linefilereader.o mod_nfo.o ../util/strmatch.o ../util/release.o

all: nfocleaner

nfocleaner: cleaner.o mod_nfo.o
	gcc -O2 -o $@ $< $(OBJS)

clean:
	rm -f *~ *.o nfocleaner

release:
	mkdir $(RELEASEDIR)/nfo
	cp nfo-*.txt Makefile mod_nfo.[ch] cleaner.c README $(RELEASEDIR)/nfo


