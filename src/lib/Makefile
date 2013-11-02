#
# $Id: Makefile.in,v 1.1 2003/05/24 05:39:36 sorend Exp $
# $Source: /home/cvs/footools/footools/src/lib/Makefile.in,v $
#
# Uncomment this to make win32/cygwin versions.
# DEFS = -DWIN32
#
# Use this if you have a system without fnmatch() function.
# DEFS = -DNO_FNMATCH

include ../Makefile.config

SRCDIR = ..

OBJS = dirlist.o cookie.o common.o genlistc.o sfv.o macro.o gllogs.o pwdfile.o stringtokenizer.o security.o sockop.o who.o xferlog.o mpegtool.o


all:	$(OBJS)

clean:
	rm -f *.o *~

release:
	@mkdir $(RELEASEDIR)/lib
	@cp Makefile README* *.c *.h $(RELEASEDIR)/lib

