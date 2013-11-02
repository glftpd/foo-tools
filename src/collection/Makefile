#
# $Id: Makefile.in,v 1.1 2003/05/24 05:39:36 sorend Exp $
# $Source: /home/cvs/footools/footools/src/collection/Makefile.in,v $
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

