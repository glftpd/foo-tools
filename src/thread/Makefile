#
# $Id: Makefile,v 1.2 2002/03/07 20:43:58 flower Exp $
# $Source: /var/cvs/foo/src/thread/Makefile,v $
#
# Uncomment this to make win32/cygwin versions.
# DEFS = -DWIN32
#
# Use this if you have a system without fnmatch() function.
# DEFS = -DNO_FNMATCH

include ../Makefile.config

OBJS = threadpool.o
SRCDIR = ..

all:	$(OBJS)

clean:
	rm -f *.o *~

release:
	mkdir $(RELEASEDIR)/thread
	cp Makefile *.c *.h $(RELEASEDIR)/thread

