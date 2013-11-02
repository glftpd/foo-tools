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


#ifndef _dirlog_h
#define _dirlog_h


// need these for ushort/time_t ;\
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>


#define DIRLOGFILE "/ftp-data/logs/dirlog"

// number of dirlog entries to read at once.
#define DIRLOG_CACHESIZE 1024

struct dirlog_item {
    ushort status;     // 0 = NEWDIR, 1 = NUKE, 2 = UNNUKE, 3 = DELETED
    time_t uptime;
    ushort uploader;    /* Libc6 systems use ushort in place of uid_t/gid_t */
    ushort group;
    ushort files;
    long bytes;
    char dirname[255];
    struct dirlog_item *nxt;
    struct dirlog_item *prv;
};


typedef struct dirlog_item dirlog_item_t;


struct dirlog {
	FILE *f;

	dirlog_item_t *cache;

	long first, last, size;
	time_t age;
};

typedef struct dirlog dirlog_t;


/*
 * Initializes dirlog structure, returns 1 on success and 0 on failure.
 */
int dirlog_init(dirlog_t *dl, char *file);

int dirlog_getentry(dirlog_t *dl, long entry, dirlog_item_t *item);

long dirlog_getsize(dirlog_t *dl);
time_t dirlog_getage(dirlog_t *dl);

int dirlog_finalize(dirlog_t *dl);




#endif
