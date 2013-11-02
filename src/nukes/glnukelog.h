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
/*
 * Tanesha Team! $Id: glnukelog.h,v 1.2 2003/01/22 14:31:29 sorend Exp $
 */

#ifndef _glnukelog_h
#define _glnukelog_h

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <collection/sortedlist.h>



struct glnukelog {
	ushort status;
	time_t nuketime;
	char   nuker[12];
	char   unnuker[12];
	char   nukee[12];
	ushort mult;
	float  bytes;
	char   reason[60];
	char   dirname[255];
	struct nukelog *nxt;
	struct nukelog *prv;
};

typedef struct glnukelog glnukelog_t;

/**
 * Universan nukee log structure.
 */
struct nukeelog {
	char *nukee;
	float bytes;
};

typedef struct nukeelog nukeelog_t;

/**
 * Universal nuke structure.
 */
struct nukelog {
	int status;
	char *dirname;
	float bytes;
	int mult;
	char *nuker;

	char *reason;
	time_t nuketime;

	sortedlist_t nukees;
};

typedef struct nukelog nukelog_t;

/*
 * Will load nukelog into a hashtable, hashtable has 
 * dirname as key, and an iterator should give the
 * nuked releases in chronological order.
 */
int nukelog_load(sortedlist_t *collection, char *filename);

#endif
