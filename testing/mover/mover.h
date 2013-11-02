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

#ifndef _mover_h
#define _mover_h

#include <collection/hashtable.h>
#include <collection/sortedlist.h>

struct archive_item {
	char *name;
	char *dir;
	double margin;
};

typedef struct archive_item archive_item_t;

struct mover_item {
	char *incoming_dir;
	char *archive_dir;

	int rule_order;

	char *dir_mask;

	int delete;

	archive_item_t *archivegroup;
};

typedef struct mover_item mover_item_t;


struct config {
	hashtable_t configfile;

	long bufsize;
	long usleept;
	
	double incomingfree;
	char *incomingdir;

	char *lockfile;

	long nuked_age;
	char *nuked_dirstyle;

	sortedlist_t rules;

	hashtable_t archives;
};

typedef struct config config_t;



#endif
