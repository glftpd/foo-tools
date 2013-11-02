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


#include "xferlog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lib/stringtokenizer.h>

#define BUFSIZE 1024

int _xferlog_parse(char *buf, xferlog_t *log) {
	stringtokenizer st;
	char *tmp;

	if (strlen(buf) < 25)
		return 0;

	log->xfer_date = date_parse_unix(buf);

	if (!log->xfer_date)
		return 0;

	st_initialize(&st, buf + 25, " ");

	if (st_count(&st) < 12) {
		free(log->xfer_date);
		return 0;
	}

	log->xfer_duration = atoi(st_next(&st));
	log->xfer_host = strdup(st_next(&st));
	log->xfer_size = atol(st_next(&st));
	log->xfer_file = strdup(st_next(&st));
	st_next(&st);
	st_next(&st);
	tmp = st_next(&st);
	log->xfer_direction = *tmp;
	st_next(&st);
	log->xfer_user = strdup(st_next(&st));
	log->xfer_group = strdup(st_next(&st));
	st_next(&st);
	log->xfer_ident = strdup(st_next(&st));

	return 1;
}

void xferlog_free(xferlog_t *log) {
	free(log->xfer_host);
	free(log->xfer_file);
	free(log->xfer_user);
	free(log->xfer_group);
	free(log->xfer_ident);
}

xferlog_t * xferlog_clone(xferlog_t *log) {
	xferlog_t *n = malloc(sizeof(xferlog_t));

	n->xfer_date = malloc(sizeof(date_t));
	memcpy(n->xfer_date, log->xfer_date, sizeof(date_t));

	n->xfer_duration = log->xfer_duration;
	n->xfer_host = strdup(log->xfer_host);
	n->xfer_file = strdup(log->xfer_file);
	n->xfer_user = strdup(log->xfer_user);
	n->xfer_group = strdup(log->xfer_group);
	n->xfer_ident = strdup(log->xfer_ident);

	n->xfer_size = log->xfer_size;
	n->xfer_direction = log->xfer_direction;

	return n;
}

long xferlog_read(char *file, int (*handler)(xferlog_t *item)) {
	FILE *f;
	char buf[BUFSIZE];
	int rc;
	long entries = 0;
	xferlog_t tmplog;

	f = fopen(file, "r");

	if (!f)
		return 0;

	while (fgets(buf, BUFSIZE, f)) {

		if (!_xferlog_parse(buf, &tmplog))
			continue;

		rc = handler(&tmplog);

		xferlog_free(&tmplog);

		if (rc == 0)
			break;

		entries++;
	}
	fclose(f);

	return entries;
}
