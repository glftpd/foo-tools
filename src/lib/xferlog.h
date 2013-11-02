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

#ifndef _xferlog_h
#define _xferlog_h

#include <util/date.h>

#define DEFAULT_XFERLOG_FILE "/ftp-data/logs/xferlog"

struct xferlog {
	date_t *xfer_date;
	int xfer_duration;
	char *xfer_host;
	long xfer_size;
	char *xfer_file;
	// char xfer_mode
	// char ??
	char xfer_direction;
	// char ??
	char *xfer_user;
	char *xfer_group;
	// int ??
	char *xfer_ident;
};

typedef struct xferlog xferlog_t;

long xferlog_read(char *file, int (*handler)(xferlog_t *item));

xferlog_t * xferlog_clone(xferlog_t *log);
void xferlog_free(xferlog_t *log);


#endif
