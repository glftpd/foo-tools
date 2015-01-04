#
# $Id: Makefile,v 1.8 2002/03/07 20:43:58 flower Exp $
# $Source: /var/cvs/foo/src/util/Makefile,v $
#
# Uncomment this to make win32/cygwin versions.
# DEFS = -DWIN32
#
# Use this if you have a system without fnmatch() function.
# DEFS = -DNO_FNMATCH

include ../Makefile.config

SRCDIR = ..

OBJS = linefilereader.o fd.o date.o strmatch.o linereaderbuffer.o

all:	$(OBJS)

clean:
	rm -f *.o *~ lfrtest

release:
	@mkdir $(RELEASEDIR)/util
	@cp *.c *.h Makefile $(RELEASEDIR)/util

