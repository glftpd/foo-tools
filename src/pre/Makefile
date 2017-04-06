

include ../Makefile.config

SRCDIR = ..

LIBS = ../collection/strlist.o ../lib/gllogs.o ../lib/pwdfile.o ../util/linefilereader.o ../collection/hashtable.o ../lib/stringtokenizer.o ./gl_userfile.o ./mp3genre/mp3genre.o

foo-pre: foo-pre.o $(LIBS)
	$(CC) -o $@ $< mp3genre/mp3genre.o $(LIBS) -ldl

foo.Pre: foo.Pre.o
	$(CC) -o $@ $< $(LIBS)

all: foo-pre

clean:
	rm -f *.o *~ foo.Pre foo-pre

release:
	mkdir $(RELEASEDIR)/pre
	cp *.c *.h Makefile README *.txt pre.cfg $(RELEASEDIR)/pre

