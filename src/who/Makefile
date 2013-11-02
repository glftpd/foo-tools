#
# $Id: Makefile,v 1.5 2002/03/14 07:38:23 flower Exp $
# $Source: /var/cvs/foo/src/who/Makefile,v $
#

include ../Makefile.config

SRCDIR = ..
LIBS = -lcurses

OBJS = ../lib/who.o spy_view.o spy.o ../lib/dirlist.o ../collection/strlist.o ../util/linefilereader.o

all:	spy


spy: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(DEFS)

clean:
	rm -f *.o *~ spy

release:
	mkdir $(RELEASEDIR)/who
	cp spy*.c spy*.h Makefile README $(RELEASEDIR)/who
