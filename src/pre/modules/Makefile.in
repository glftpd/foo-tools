

include ../../Makefile.config

SRCDIR = ../..

LIBS = $(SRCDIR)/lib/macro.o $(SRCDIR)/lib/gllogs.o $(SRCDIR)/lib/pwdfile.o $(SRCDIR)/util/linefilereader.o $(SRCDIR)/collection/hashtable.o $(SRCDIR)/lib/stringtokenizer.o

all: mod_idmp3.so mod_sitenfoadd.so mod_nfohandler.so mod_chmod.so mod_symlink.so mod_audiosort.so

mod_idmp3.so: mod_idmp3.o
	(cd mp3info; make)
	$(CC) -g -shared -Wl,-soname,mod_idmp3.so -o $@ $< mp3info/mp3tech.o $(LIBS)

mod_sitenfoadd.so: mod_sitenfoadd.o
	$(CC) -g -shared -Wl,-soname,mod_sitenfoadd.so -o $@ $< $(LIBS)

mod_chmod.so: mod_chmod.o
	$(CC) -g -shared -Wl,-soname,mod_chmod.so -o $@ $< $(LIBS)

mod_nfohandler.so: mod_nfohandler.o
	$(CC) -g -shared -Wl,-soname,mod_nfohandler.so -o $@ $< $(LIBS)

mod_symlink.so: mod_symlink.o
	$(CC) -g -shared -Wl,-soname,mod_symlink.so -o $@ $< $(LIBS)

mod_audiosort.so: mod_audiosort.o
	$(CC) -g -shared -Wl,-soname,mod_audiosort.so -o $@ $< $(LIBS) $(SRCDIR)/collection/strlist.o

tester: tester.o
	$(CC) -o $@ $< ../../collection/hashtable.o ../../util/linefilereader.o -ldl

clean:
	(cd mp3info; make clean)
	rm -f *.o *~ *.so


