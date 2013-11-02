/*
 * foo-tools, a collection of utilities for glftpd users.
 * Copyright (C) 2003  Tanesha FTPD Project, www.tanesha.net
 *
 * This file is part of foo-tools.
 *
 * foo-tools is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * foo-tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with foo-tools; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _dirlog_wrap_h
#define _dirlog_wrap_h


#include <time.h>
#include <collection/sortedlist.h>
#include <collection/hashtable.h>
#include "dirlog.h"

// levels for maindirs
#define SUBDIR_LEVEL 2

// skip this from dir.
#define STARTDIR "/fk"

struct di {
	long pos;
	dirlog_item_t di;
};

typedef struct di di_t;

struct release {
	time_t time;
	char *rel;

	di_t maindir;


	hashtable_t subdirs;
};

typedef struct release release_t;


int dirlog_wrap_getnew(dirlog_t *dl, sortedlist_t *rels, int show, char *startdir);


#endif
