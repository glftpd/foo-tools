
include ../Makefile.config

SRCDIR = ..

OBJS = ../collection/sortedlist.o ../collection/hashtable.o ../util/linefilereader.o \
	glnukelog.o nukes.o

all: foo-nukes

foo-nukes: $(OBJS)
	gcc -O2 -o $@ $(OBJS)

clean:
	rm -f *~ *.o foo-nukes

release:
	mkdir $(RELEASEDIR)/nukes
	cp *.c *.h Makefile README $(RELEASEDIR)/nukes
