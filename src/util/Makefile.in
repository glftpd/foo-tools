#
# $Id: Makefile.in,v 1.1 2003/05/24 05:39:36 sorend Exp $
# $Source: /home/cvs/footools/footools/src/util/Makefile.in,v $
#
# Uncomment this to make win32/cygwin versions.
# DEFS = -DWIN32
#
# Use this if you have a system without fnmatch() function.
# DEFS = -DNO_FNMATCH

include ../Makefile.config

SRCDIR = ..

OBJS = linefilereader.o fd.o date.o strmatch.o linereaderbuffer.o release.o filelock.o

all:	$(OBJS)

clean:
	rm -f *.o *~ lfrtest

release:
	@mkdir $(RELEASEDIR)/util
	@cp *.[ch] Makefile $(RELEASEDIR)/util

