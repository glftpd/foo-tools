#
# $Id: Makefile.in,v 1.1 2003/05/24 05:39:36 sorend Exp $
# $Source: /home/cvs/footools/footools/src/thread/Makefile.in,v $
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

