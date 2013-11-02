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


#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <util/date.h>
#include <util/linefilereader.h>
#include <lib/stringtokenizer.h>
#include <collection/sortedlist.h>

#include "findxfers.h"

xferlog_t * findxfers_parseline(char *line) {
	xferlog_t *tmp = malloc(sizeof(xferlog_t));
	date_t *ttime;
	char *buf;
	stringtokenizer st;

	if (strlen(line) < 25) {
		free(tmp);
		return 0;
	}

	ttime = date_parse_unix(line);

	if (!ttime) {
		free(tmp);
		return 0;
	}
	
	st_initialize(&st, line + 25, " ");

	if (st_count(&st) != 12) {
		printf("Count %d, ", st_count(&st));
		st_finalize(&st);
		free(ttime);
		free(tmp);
		return 0;
	}

	tmp->time = ttime;
	tmp->secs = atoi(st_next(&st));
	tmp->host = strdup(st_next(&st));
	tmp->bytes = atol(st_next(&st));
	tmp->file = strdup(st_next(&st));

	buf = st_next(&st);
	switch (*buf) {
	case 'b': tmp->mode = MODE_BINARY;
		break;
	case 'a': tmp->mode = MODE_ASCII;
		break;
	default:
		tmp->mode = -1;
	}

	st_next(&st);

	buf = st_next(&st);

	switch (*buf) {
	case 'o': tmp->direction = DIRECTION_DOWNLOAD;
		break;
	case 'i': tmp->direction = DIRECTION_UPLOAD;
		break;
	default:
		tmp->direction = -1;
	}
	
	st_next(&st);

	tmp->user = strdup(st_next(&st));
	tmp->group = strdup(st_next(&st));

	tmp->next = 0;

	return tmp;
}

userxfers_t *userxfers_find_by_user(userxfers_t *l, char *u) {
	userxfers_t *tmp;

	for (tmp = l; tmp; tmp = tmp->next)
		if (!strcmp(tmp->user, u))
			break;

	return tmp;
}


userxfers_t *userxfers_add_xferlog(userxfers_t *l, xferlog_t *x) {
	userxfers_t *tmp, *ret = l;

	tmp = userxfers_find_by_user(l, x->user);

	if (!tmp) {
		tmp = malloc(sizeof(userxfers_t));
		tmp->user = strdup(x->user);
		tmp->files_up = tmp->files_down = 0;
		tmp->seconds_up = tmp->seconds_down = 0;
		tmp->kbytes_up = tmp->kbytes_down = 0;
		tmp->first = tmp->last = 0;

		tmp->next = l;
		ret = tmp;
	}

	if (!tmp->first)
		tmp->first = x->time;

	tmp->last = x->time;

	if (x->direction == DIRECTION_UPLOAD) {
		tmp->files_up++;
		tmp->kbytes_up += (long)x->bytes/1024;
		tmp->seconds_up += x->secs;
	} else if (x->direction == DIRECTION_DOWNLOAD) {
		tmp->files_down++;
		tmp->kbytes_down += (long)x->bytes/1024;
		tmp->seconds_down += x->secs;
	}

	return ret;
}


xferlog_t * findxfers_read_log(char *logfile) {
	linefilereader_t lfr;
	xferlog_t *head = 0, *tail = 0, *tmp;
	char buf[1024];
	int entries = 0;

	if (lfr_open(&lfr, logfile) < 0)
		return 0;

	while (lfr_getline(&lfr, buf, 1024) > -1) {
		tmp = findxfers_parseline(buf);

		if (!tmp)
			continue;

		entries++;

		if (!tail)
			tail = tmp;
		else {
			tail->next = tmp;
			tail = tmp;
		}		

		if (!head)
			head = tail;
	}

	lfr_close(&lfr);

	printf("-- %s contained %d parseable entries\n", logfile, entries);

	return head;
}

userxfers_t * findxfers_sum(xferlog_t *l) {
	xferlog_t *tmp;
	userxfers_t *sum = 0;

	for (tmp = l; tmp; tmp = tmp->next)
		sum = userxfers_add_xferlog(sum, tmp);

	return sum;
}

int userxfers_sortby_kbytesup(void *p, void *q) {
	userxfers_t *a, *b;

	a = (userxfers_t*)p;
	b = (userxfers_t*)q;

	if (a->kbytes_up > b->kbytes_up)
		return 1;

	return 0;
}

void xfers_output(userxfers_t *l) {
	sortedlist_t sorted;
	userxfers_t *tmp;

	// make sorted list ready.
	sortedlist_init(&sorted);
	for (tmp = l; tmp; tmp = tmp->next)
		sortedlist_add(&sorted, tmp);

	sortedlist_sort(&sorted, userxfers_sortby_kbytesup);

	sortedlist_reset(&sorted);

	//1234567890123456789012345678901234567890123456789012345678901234567890

	printf("e Username    fup   kbup       sup       fdn   kbdn       sdn\n");
	printf("- ----------- ----- ---------- --------- ----- ---------- ---------\n");

	while (sortedlist_hasnext(&sorted)) {
		tmp = (userxfers_t*)sortedlist_next(&sorted);

		printf("%c %-11.11s %5d %10.1f %9.1f %5d %10.1f %9.1f\n",
			   ' ',
			   tmp->user,
			   tmp->files_up,
			   (float)tmp->kbytes_up/1024,
			   (tmp->seconds_up > 0) ? (float)tmp->kbytes_up/tmp->seconds_up : 0,
			   tmp->files_down,
			   (float)tmp->kbytes_down/1024,
			   (tmp->seconds_down > 0) ? (float)tmp->kbytes_down/tmp->seconds_down : 0);
	}

	// sortedlist_close(&sorted, 0);
}


int main(int argc, char *argv[]) {
	xferlog_t *list;
	userxfers_t *sum = 0;
	long now;

	if (argc < 3) {
		printf("syntax: %s <logfile> <daysback>\n", argv[0]);
		return 0;
	}

	now = time(0);
	now -= (3600 * 24 * atoi(argv[2]));

	list = findxfers_read_log(argv[1]);

	sum = findxfers_sum(list);

	xfers_output(sum);

	return 0;
}
