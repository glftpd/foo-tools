

include ../Makefile.config

SRCDIR = ..

LIBS = ../collection/strlist.o ../lib/gllogs.o ../util/linefilereader.o ../collection/hashtable.o ../lib/stringtokenizer.o ../pre/gl_userfile.o ../util/filelock.o records.o ../util/date.o

foo-reset: reset.o $(LIBS)
	$(CC) -o $@ $< $(LIBS)

all: foo-reset

clean:
	rm -f *.o *~ foo-reset

release:
	mkdir $(RELEASEDIR)/reset
	cp *.c *.h Makefile README $(RELEASEDIR)/reset

