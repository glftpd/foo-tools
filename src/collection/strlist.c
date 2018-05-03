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
/**
 * Library implementing a searchable string list.
 *
 * Refactored a bunch here  /sorend.
 *
 **
 * $Id: strlist.c,v 1.2 2003/01/22 14:31:29 sorend Exp $
 * $Source: /home/cvs/footools/footools/src/collection/strlist.c,v $
 * Author: Soren
 */

#include "strlist.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fnmatch.h>
#include <util/linefilereader.h>

long str_count(strlist_t *l) {

	if (l)
		return 1 + str_count(l->next);
	else
		return 0;
}

strlist_t *str_search(strlist_t *l, char *s, int flags) {
	strlist_t *t;

	for (t = l; t; t = t->next) {
		if (!t->data)
			continue;

		if (!flags && !strcmp(s, t->data))
			break;
		else if ((flags & STR_NOCASE) && !strcasecmp(s, t->data))
			break;
#ifndef NO_FNMATCH
		else if ((flags & STR_FNMATCH) && !fnmatch(s, t->data, 0))
			break;
#endif
	}

	return t;
}

strlist_t *str_add(strlist_t *l, char *s) {
	strlist_t *t=(strlist_t *)malloc(sizeof(strlist_t));

	t->data = strdup(s);

	t->next = l;

	return t;
}

strlist_t *str_add_last(strlist_t *l, char *s) {
	strlist_t *b, *t = (strlist_t*)malloc(sizeof(strlist_t));

	t->data = strdup(s);
	t->next = 0;

	if (!l)
		return t;

	for (b = l; b->next; b = b->next);
	b->next = t;

	return l;
}

strlist_t *str_load(strlist_t *l, char *fn) {
	linefilereader_t lr;
	char buf[STR_MAXLINELEN + 1];

	if (lfr_open(&lr, fn) < 0) {
		lfr_close(&lr);

		return l;
	}

	while (lfr_getline(&lr, buf, STR_MAXLINELEN) > -1)
		l = str_add_last(l, buf);
	
	lfr_close(&lr);

	return l;
}

void str_close(strlist_t *l) {
	strlist_t *t;

	while (l) {
		t = l;

		l = l->next;

		if (t->data)
			free(t->data);

		free(t);
	}
}

strlist_t *str_reverse(strlist_t *l) {
	strlist_t *t, *r = 0;

	for (t = l; t; t = t->next)
		r = str_add(r, t->data);

	return t;
}

strlist_t *str_op_and(strlist_t *p, strlist_t *q) {
	strlist_t *r = 0, *t;

	// add items in both p and q.
	for (t = p; t; t = t->next)
		if (str_search(q, t->data, 0))
			r = str_add(r, t->data);

	return r;
}

strlist_t *str_op_or(strlist_t *p, strlist_t *q) {
	strlist_t *r = 0, *t;

	// add contents of p
	for (t = p; t; t = t->next)
		r = str_add(r, t->data);

	// add contents of q
	for (t = q; t; t = t->next)
		if (!str_search(r, t->data, 0))
			r = str_add(r, t->data);

	return r;
}

strlist_iterator_t * str_iterator(strlist_t *l) {
	strlist_iterator_t *i = (strlist_iterator_t*)malloc(sizeof(strlist_iterator_t));

	i->cur = l;
}

int str_iterator_hasnext(strlist_iterator_t *i) {
	if (i->cur)
		return 1;

	return 0;
}

char *str_iterator_next(strlist_iterator_t *i) {
	char *ret = 0;

	if (i->cur) {
		ret = i->cur->data;
		i->cur = i->cur->next;
	}

	return ret;
}

char *str_join(strlist_t *l, char *delim) {
	strlist_t *tmp;
	char *ret;
	int newlen = 0, dl;

	dl = strlen(delim);

	for (tmp = l; tmp; tmp = tmp->next) {
		newlen += strlen(tmp->data);

		if (tmp->next)
			newlen += dl;
	}

	if (newlen == 0)
		return strdup("");


	ret = malloc(newlen + 1);
	*ret = 0;
	for (tmp = l; tmp; tmp = tmp->next) {
		strcat(ret, tmp->data);

		if (tmp->next)
			strcat(ret, delim);
	}

	return ret;
}
