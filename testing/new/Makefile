
include ../Makefile.config

SRCDIR = ..

OBJS = dirlog.o dirlog_wrap.o ../collection/sortedlist.o ../collection/hashtable.o \
	../util/linefilereader.o ../lib/dirlist.o ../collection/strlist.o ../lib/sfv.o \
	../util/date.o ../lib/stringtokenizer.o ../lib/pwdfile.o


all: foo-new

foo-new: new.o $(OBJS)
	gcc -O2 -o $@ $< $(OBJS)

clean:
	rm -f *~ *.o foo-new

