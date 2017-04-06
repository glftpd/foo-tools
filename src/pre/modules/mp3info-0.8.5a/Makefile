#   Makefile for MP3Info and GMP3Info
#
#   Copyright (C) 2000-2006 Cedric Tefft <cedric@phreaker.net>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# ***************************************************************************
#
#  This program is based in part on:
#
#	* MP3Info 0.5 by Ricardo Cerqueira <rmc@rccn.net>
#	* MP3Stat 0.9 by Ed Sweetman <safemode@voicenet.com> and 
#			 Johannes Overmann <overmann@iname.com>
#

# bindir = where binaries get installed (default = /usr/local/bin)
# mandir = where the manual page gets installed (default = /usr/local/man/man1)

prefix=/usr/local
bindir=${prefix}/bin
mandir = $(prefix)/man/man1

# No changes necessary below this line

PROG =	mp3info
SRCS =	mp3info.c textfunc.c mp3curs.c mp3tech.c
OBJS =  mp3info.o textfunc.o mp3curs.o mp3tech.o
XOBJ =  mp3tech.o
RM = /bin/rm
INSTALL = /usr/bin/install -c
STRIP = strip

LIBS = -lncurses
CC = gcc
CFLAGS = -g -O2 -Wall -fPIC

all: mp3info doc

doc: mp3info.txt

mp3info: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

mp3info.txt: mp3info.1
	groff -t -e -mandoc -Tascii mp3info.1 | col -bx > mp3info.txt

clean: 
	$(RM) -f $(OBJS) $(XOBJ) mp3info core

dist: clean doc

distclean: clean
	$(RM) -f mp3info.txt

install-mp3info: mp3info
	$(STRIP) mp3info
	$(INSTALL) mp3info $(bindir)/mp3info
	$(INSTALL) mp3info.1 $(mandir)/mp3info.1

install: install-mp3info


uninstall:
	rm -f $(bindir)/mp3info
	rm -f $(mandir)/mp3info.1

