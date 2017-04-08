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
 * Defines some routines to sort about everything with.
 *
 * <sorend@tanesha.net>
 * $Id: sortedlist.h,v 1.2 2003/01/22 14:31:29 sorend Exp $
 */
#ifndef _sortedlist_h
#define _sortedlist_h

#include <stdlib.h>

// holds list items.
struct sortedlist_item {
	void *obj;

	struct sortedlist_item *next, *prev;
};

typedef struct sortedlist_item sortedlist_item_t;

// sorted list structure.
struct sortedlist {
	sortedlist_item_t *current;
	sortedlist_item_t *list;
};

typedef struct sortedlist sortedlist_t;

/*
 * sorts a list by using sortfunc'tion.
 */
int sortedlist_sort(sortedlist_t *l, int (*sortfunc)(void *p, void *q));

/*
 * adds an item to the list to be sorted.
 */
int sortedlist_add(sortedlist_t *l, void *p);

/*
 * initializes sortedlist structure.
 */
int sortedlist_init(sortedlist_t *l);

/*
 * closes sortedlist structure. if freeobjects is set then free() will
 * be called on each item in the list (use with care!).
 */
int sortedlist_close(sortedlist_t *l, int freeobjects);

/*
 * initializes sorted list for traversal.
 */
int sortedlist_reset(sortedlist_t *l);

/*
 * returns 1 if there are more items in the list.
 */
int sortedlist_hasnext(sortedlist_t *l);

/*
 * returns next item in the sorted list, or 0 if none.
 */
void * sortedlist_next(sortedlist_t *l);

#endif
