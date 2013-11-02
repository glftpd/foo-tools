#
# $Id: Makefile,v 1.3 2002/03/07 20:43:57 flower Exp $
# $Source: /var/cvs/foo/src/collection/Makefile,v $
#
# Uncomment this to make win32/cygwin versions.
# DEFS = -DWIN32
#
# Use this if you have a system without fnmatch() function.
# DEFS = -DNO_FNMATCH

include ../Makefile.config

SRCDIR = ..

DEFS = -DHT_SUPPORT_FILEPROP

OBJS = sortedlist.o strlist.o hashtable.o

all:	$(OBJS)

clean:
	rm -f *.o *~

release:
	@mkdir $(RELEASEDIR)/collection
	@cp *.c *.h Makefile $(RELEASEDIR)/collection

