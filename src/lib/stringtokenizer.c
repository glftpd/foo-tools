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

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "stringtokenizer.h"


void st_initialize(stringtokenizer *st, char *str, char *split) {
	char *tmp, *start;
	st_item *i;

	st->list = st->cur = NULL;

	tmp = start = str;

	while (tmp && *tmp) {

		if (!strncmp(tmp, split, strlen(split))) {
			i = (st_item*)malloc(sizeof(st_item));
			i->token = (char*)malloc(tmp - start + 1);

			strncpy(i->token, start, tmp - start);
			i->token[tmp - start] = 0;

			i->next = st->cur;
			st->cur = i;

			start = tmp = tmp + strlen(split);
		} else
			tmp++;
	}

	if (tmp) {
		i = (st_item*)malloc(sizeof(st_item));
		i->token = (char*)malloc(strlen(start) + 1);
		strcpy(i->token, start);

		i->next = st->cur;
		st->cur = i;
	}

	// make order correct in list of tokens.
	while (st->cur) {
		i = st->cur->next;

		st->cur->next = st->list;
		st->list = st->cur;

		st->cur = i;
	}

	st->cur = st->list;
}

void st_finalize(stringtokenizer *st) {
	
	while (st->list) {
		st->cur = st->list;

		st->list = st->list->next;

		free(st->cur);
	}

	st->list = st->cur = NULL;
}

int _st_count(st_item *i) {
	if (i)
		return 1 + _st_count(i->next);
	
	return 0;
}

int st_count(stringtokenizer *st) {
	return _st_count(st->cur);
}



int st_hasnext(stringtokenizer *st) {
	if (st->cur)
		return 1;

	return 0;
}

char *st_next(stringtokenizer *st) {
	char *t;

	if (!st->cur)
		return NULL;

	t = st->cur->token;

	st->cur = st->cur->next;

	return t;
}

void st_reset(stringtokenizer *st) {
	st->cur = st->list;
}
