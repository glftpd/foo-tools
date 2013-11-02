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
 * Tanesha Team! $Id: glnukelog.c,v 1.2 2003/01/22 14:31:29 sorend Exp $
 */
#include "glnukelog.h"

void _nukelog_printf(nukelog_t *t) {
	nukeelog_t *nl;

	printf("%s [x%d %s %s]", t->dirname, t->mult, t->nuker, t->reason);

	sortedlist_reset(&t->nukees);
	while (nl = sortedlist_next(&t->nukees)) {
		printf(" (%s %.0f)", nl->nukee, nl->bytes);
	}
	printf("\n");
}


int _nukelog_same_release(glnukelog_t *a, glnukelog_t *b) {

	if (a->status != b->status)
		return 0;

	if (strcmp(a->dirname, b->dirname))
		return 0;

	if (a->mult != b->mult)
		return 0;

	if (strcmp(a->nuker, b->nuker))
		return 0;

	if (strcmp(a->reason, b->reason))
		return 0;

	return 1;
}

nukelog_t * _nukelog_add_release(sortedlist_t *t, glnukelog_t *g) {
	nukelog_t *tmp = malloc(sizeof(nukelog_t));

	sortedlist_init(&tmp->nukees);

	tmp->nuketime = g->nuketime;
	tmp->mult = g->mult;
	tmp->dirname = strdup(g->dirname);
	tmp->status = g->status;
	tmp->nuker = (g->status == 0)?strdup(g->nuker):strdup(g->unnuker);
	tmp->reason = malloc(65);
	strncpy(tmp->reason, g->reason, 60);
	tmp->bytes = 0;

	sortedlist_add(t, tmp);

	return tmp;
}

void _nukelog_add_nukee(nukelog_t *t, glnukelog_t *g) {
	nukeelog_t *ne = malloc(sizeof(nukeelog_t));

	ne->nukee = strdup(g->nukee);
	ne->bytes = g->bytes;
	t->bytes += g->bytes;

	sortedlist_add(&t->nukees, ne);
}


int nukelog_load(sortedlist_t *collection, char *filename) {
	FILE *f;
	glnukelog_t tmp, old;
	nukelog_t *last = 0;
	int first = 1;

	bzero(&old, sizeof(glnukelog_t));

	f = fopen(filename, "rb");

	if (!f)
		return 0;

	while (fread(&tmp, sizeof(tmp), 1, f)) {

		if (first || !_nukelog_same_release(&tmp, &old)) {
			memcpy(&old, &tmp, sizeof(old));
			last = _nukelog_add_release(collection, &tmp);
			_nukelog_add_nukee(last, &tmp);
			first = 0;
		} else {
			_nukelog_add_nukee(last, &tmp);
		}

		// _nukelog_printf(last);
	}
	fclose(f);

	return 1;
}

int _nukelog_sort_by_age(void *a, void *b) {
	nukelog_t *p, *q;

	p = (nukelog_t*)a;
	q = (nukelog_t*)b;

	if (p->nuketime > q->nuketime)
		return 1;
	else if (p->nuketime == q->nuketime)
		return 0;

	return -1;
}

