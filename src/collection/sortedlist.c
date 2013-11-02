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
 * Impl of sortedlist.h interface, <sorend@tanesha.net>
 *
 * $Id: sortedlist.c,v 1.2 2003/01/22 14:31:29 sorend Exp $
 */

#include "sortedlist.h"


int sortedlist_init(sortedlist_t *l) {
	l->list = 0;
	l->current = 0;
	return 1;
}

int sortedlist_reset(sortedlist_t *l) {
	l->current = l->list;
	return 1;
}

int sortedlist_hasnext(sortedlist_t *l) {
	if (l->current)
		return 1;

	return 0;
}

void * sortedlist_next(sortedlist_t *l) {
	void *p;

	if (!l->current)
		return 0;

	p = l->current->obj;

	l->current = l->current->next;

	return p;
}

int sortedlist_add(sortedlist_t *l, void *p) {
	sortedlist_item_t *tmp = (sortedlist_item_t*)malloc(sizeof(sortedlist_item_t));

	tmp->obj = p;
	tmp->prev = tmp->next = 0;

	if (l->list) {
		tmp->next = l->list;
		l->list->prev = tmp;
		l->list = tmp;
	} else
		l->list = tmp;
}

int sortedlist_sort(sortedlist_t *l, int (*sortfunc)(void *p, void *q)) {
	sortedlist_item_t *ret = 0, *tmp, *ins, *clone, *tt;

	for (tmp = l->list; tmp; tmp = tmp->next) {

		// create new position object.
		clone = (sortedlist_item_t*)malloc(sizeof(sortedlist_item_t));
		clone->obj = tmp->obj;
		clone->prev = clone->next = 0;

		if (ret) {
			ins = ret;

			while (ins->next && sortfunc(ins->obj, clone->obj))
				ins = ins->next;

			if (sortfunc(ins->obj, clone->obj)) {
				ins->next = clone;
				clone->prev = ins;
			} else {
				tt = ins;
				tt = ins->prev;
				ins->prev = clone;
				clone->next = ins;
				if (tt)
					tt->next = clone;
				clone->prev = tt;
				if (ins == ret)
					ret = clone;
			}
		} else
			ret = clone;
	}

	while (l->list) {
		tt = l->list;

		l->list = l->list->next;

		free(tt);
	}

	// make list the new list.
	l->list = ret;
}
