#
# $Id: Makefile,v 1.12 2002/03/07 20:43:57 flower Exp $
# $Source: /var/cvs/foo/src/lib/Makefile,v $
#
# Uncomment this to make win32/cygwin versions.
# DEFS = -DWIN32
#
# Use this if you have a system without fnmatch() function.
# DEFS = -DNO_FNMATCH

include ../Makefile.config

SRCDIR = ..

OBJS = dirlist.o cookie.o common.o genlistc.o sfv.o macro.o gllogs.o pwdfile.o stringtokenizer.o security.o sockop.o who.o xferlog.o


all:	$(OBJS)

clean:
	rm -f *.o *~

release:
	@mkdir $(RELEASEDIR)/lib
	@cp Makefile *.c *.h $(RELEASEDIR)/lib

