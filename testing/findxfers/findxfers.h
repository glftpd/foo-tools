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


#ifndef _findxfers_h
#define _findxfers_h

#include <util/date.h>

#define MODE_BINARY 1
#define MODE_ASCII 2

#define DIRECTION_UPLOAD 1
#define DIRECTION_DOWNLOAD 2

struct xferlog {
	date_t *time;

	int secs;
	char *host;
	long bytes;
	char *file;
	int mode;
	int direction;
	char *user;
	char *group;

	struct xferlog *next;
};


typedef struct xferlog xferlog_t;


struct userxfers {
	char *user;

	long kbytes_up;
	long files_up;
	long seconds_up;

	long kbytes_down;
	long files_down;
	long seconds_down;

	date_t *first;
	date_t *last;

	struct userxfers *next;
};

typedef struct userxfers userxfers_t;

#endif
