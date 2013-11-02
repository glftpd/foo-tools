
include ../../src/Makefile.config

SRCDIR = ../../src

LIBS = $(LIBDIR)/lib/gllogs.o $(LIBDIR)/collection/strlist.o \
	$(LIBDIR)/lib/who.o $(LIBDIR)/util/linefilereader.o

#
# Set this to your ipckey !!
#
DEFS = -DIPCKEY=0x0000DEAD

foo-antidualleech: antidualleech.o
	gcc -O2 -o $@ $< $(LIBS) 

all: foo-antidualleech

install: all
	install -m4711 foo-antidualleech $(FTPROOT)/bin/cscript/

clean:
	rm -f *.o *~ foo-antidualleech
