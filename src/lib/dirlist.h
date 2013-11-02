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
 * lib for handling releases in dirs.
 */

#ifndef _dirlist_h
#define _dirlist_h

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <collection/strlist.h>

#define MAX_FILELEN 256

#define RACE_INIT (1 << 1)
#define RACE_COMPLETE (1 << 2)
#define RACE_FAIL (1 << 3)

#define RACEINFO_DONT -1

struct race_file {
	int op;

	char file[MAX_FILELEN];
	int uid;

	long speed;

	unsigned long time;
};

typedef struct race_file race_file_t;

struct race_header {
	int version;

	unsigned long start;
	unsigned long end;
};

typedef struct race_header race_header_t;

// wrapper structure for the raceinfo items.
struct race_file_list {
	race_file_t info;

	struct race_file_list *next;
};

typedef struct race_file_list race_file_list_t;

struct dirlist_item {
    struct stat st;
    char *file;
	int downloads;

	race_file_list_t *raceinfo;

    struct dirlist_item *next;
};

typedef struct dirlist_item dirlist_item_t;

struct dirlist {
	dirlist_item_t *cur;
    dirlist_item_t *list;
	race_header_t *raceinfo;

	FILE *racefile_lock;

    char *dir;
};

typedef struct dirlist dirlist_t;

#define DL_PERM_INCOMING S_IXUSR
#define DL_PERM_COMPLETE (S_IFREG | !S_IXUSR)

// patterns.
#define DL_MASK_SFV "*.sfv"
#define DL_MASK_ZIP "*.zip"
#define DL_MASK_RAR "*.[r0123456789][a0123456789][r0123456789]"


int dirlist_init(dirlist_t *d, char *dir);

/*
 * Reads the dir.
 */
int dirlist_readdir(dirlist_t *d);

/*
 * Finds files in dirlist that has permissions matching 'perms'.
 */
strlist_t * dirlist_get_perms(dirlist_t *d, int perms);

/*
 * Finds rarsize of files in the dir 'p'.
 */
long dirlist_get_filesize(dirlist_t *d);

/*
 * Finds sfv in dirlist.
 */
int dirlist_get_sfv(dirlist_t *d, char *sfv);

/*
 * Finds zips in dirlist.
 */
strlist_t * dirlist_get_pattern(dirlist_t *d, char *p);

/*
 * gets a file in dir.
 */
dirlist_item_t * dirlist_get_file(dirlist_t *d, char *f);

/*
 * Loads race-info for a file.
 */
int dirlist_raceinfo_load(dirlist_t *d, char *ef);

/*
 * Adds another raceinfo entry.
 */
int dirlist_raceinfo_add(dirlist_t *d, char *f, int uid, long s, unsigned long t, int op);


race_file_t * dirlist_raceinfo_find(dirlist_item_t *i, int uid, int op);

/*
 * Closes a dirlist.
 */
void dirlist_closedir(dirlist_t *d);

/*
 * Gives a name for the temp file name.
 */
int dirlist_gettmpracefile(dirlist_t *d, char *rf, int n);

void dirlist_reset(dirlist_t *d);

int dirlist_hasnext(dirlist_t *d);

dirlist_item_t * dirlist_next(dirlist_t *d);

#endif
