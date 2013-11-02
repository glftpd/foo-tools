
include ../../src/Makefile.config

SRCDIR = ../../src

OBJS = $(LIBDIR)/collection/strlist.o $(LIBDIR)/collection/hashtable.o \
	$(LIBDIR)/lib/stringtokenizer.o $(LIBDIR)/util/linefilereader.o \
	$(LIBDIR)/collection/sortedlist.o


all: foo-mover


foo-mover: mover.o
	gcc -O2 -o $@ $< $(OBJS)

clean:
	rm -f *~ *.o foo-mover

