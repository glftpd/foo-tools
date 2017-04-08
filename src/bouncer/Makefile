#
# $Id: Makefile.in,v 1.3 2003/09/27 07:39:49 sorend Exp $
# $Source: /home/cvs/footools/footools/src/bouncer/Makefile.in,v $
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

FOOBNCOBJS = foobnc.o $(GENOBJS) ../util/linereaderbuffer.o hammerprotect.o

#DEFS = -DADMINPORT
SRCDIR = .. -I/usr/local/lib

#all:	foobnc
all:


foobnc: $(FOOBNCOBJS)
	$(CC) -o $@ $(FOOBNCOBJS) -L/usr/local/lib -lpthread -lhttpd $(DEFS)

clean:
	rm -f *.o *~ simple_entry entry datanode *.pid


