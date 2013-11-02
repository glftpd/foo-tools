#
# $Id: Makefile,v 1.25 2002/03/07 20:43:57 flower Exp $
# $Source: /var/cvs/foo/src/bouncer/Makefile,v $
#

# make debug version, uncomment this to make running versino.
# DEFS = -DDEBUG=1
#
# Use this one to run the bouncer on a win32 box with cygwin.
#DEFS = -DDEBUG=1 -DWIN32
#DEFS = -DDNVIEW

include ../Makefile.config

LIBS = -lpthread -lncurses -lcrypto -lssl
#LIBS = -pthread -lncurses

GENOBJS = ../lib/sockop.o ../collection/hashtable.o ../collection/strlist.o ../lib/common.o ../lib/stringtokenizer.o ../thread/threadpool.o ../util/linefilereader.o
ENTRYOBJS = entry_ssl.o entry_datanode.o entry.o datanode_communicate.o $(GENOBJS)
#SIMPLEENTRYOBJS = simple_entry.o admin_httpd.o $(GENOBJS) ../util/linereaderbuffer.o
SIMPLEENTRYOBJS = simple_entry.o $(GENOBJS) ../util/linereaderbuffer.o
DATAOBJS = datanode_handler.o datanode_view.o ../lib/sfv.o $(GENOBJS)

# DEFS = -DADMINPORT
SRCDIR = ..

all:	simple_entry

entry: $(ENTRYOBJS)
	$(CC) -o $@ $(ENTRYOBJS) $(LIBS) $(DEFS)

simple_entry: $(SIMPLEENTRYOBJS)
	$(CC) -o $@ $(SIMPLEENTRYOBJS) -lpthread $(DEFS)


datanode: $(DATAOBJS)
	$(CC) -o $@ $(DATAOBJS) $(LIBS)

tripserver: tripserver.o $(GENOBJS)
	$(CC) -o $@ $(GENOBJS) $(LIB)

clean:
	rm -f *.o *~ entry datanode *.pid

release:
	mkdir $(RELEASEDIR)/bouncer
	cp my*.txt Makefile simple_entry.[ch] simple_sample.cfg $(RELEASEDIR)/bouncer


