#
# $Id: Makefile.in,v 1.1 2003/05/24 05:39:36 sorend Exp $
# $Source: /home/cvs/footools/footools/src/who/Makefile.in,v $
#

include ../Makefile.config

SRCDIR = ..
LIBS = -lcurses

OBJS = ../lib/who.o spy_view.o spy.o ../lib/dirlist.o ../collection/strlist.o ../util/linefilereader.o

WEBSPYOBJS = ../lib/who.o webspy.o webspy_nodeview.o webspy_nodedetailsview.o webspy_dirview.o ../lib/dirlist.o ../collection/strlist.o ../util/linefilereader.o ../lib/macro.o ../collection/hashtable.o ../lib/pwdfile.o ../lib/stringtokenizer.o

WEBSPYLIBS = -lhttpd
WEBSPYFLAGS = -I/opt/nk/include -L/opt/nk/lib

# for webspy
#DEFS = -D__DEBUG__

all:	spy

webspy:
	for i in $(patsubst %.o,%.c,$(WEBSPYOBJS)); do \
		$(CC) -c $$i $(INCLIB) $(WEBSPYLIBS) $(WEBSPYFLAGS) $(DEFS) ;\
	done
	$(CC) -static -o $@ $(WEBSPYOBJS) $(WEBSPYLIBS) $(WEBSPYFLAGS) $(DEFS)

spy: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(DEFS)

usage: usage.o ../lib/who.o ../collection/strlist.o ../util/linefilereader.o ../lib/common.o
	$(CC) -o $@ usage.o ../lib/common.o ../lib/who.o ../collection/strlist.o ../util/linefilereader.o $(LIBS) $(DEFS)

clean:
	rm -f *.o *~ spy webspy

release:
	mkdir $(RELEASEDIR)/who
	cp spy*.c spy*.h Makefile README $(RELEASEDIR)/who
