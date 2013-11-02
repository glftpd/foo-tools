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
 * Generic sortable/searchable list for items/structs.
 *
 **
 * $Id: genlistc.c,v 1.2 2003/01/22 14:31:29 sorend Exp $
 * $Source: /home/cvs/footools/footools/src/lib/genlistc.c,v $
 * Author: Soren
 */
#include "genlistc.h"

void glist_init(struct glist_t *l, int type) {
	l->head=NULL; l->tail=NULL;
	l->insert_type=type;
}

void glist_deinit(struct glist_t *l) {
	while (l->head) {
		l->cur=l->head;
		l->head=l->head->next;
		if (l->cur->obj)
			free(l->cur->obj);
		free(l->cur);
	}
}

void glist_reset(struct glist_t *l) {
	l->cur=l->head;
}

void glist_resettail(struct glist_t *l) {
	l->cur=l->head;
	while (l->cur && l->cur->next)
		l->cur=l->cur->next;
}

void glist_next(struct glist_t *l) {
	if (l->cur)
		l->cur=l->cur->next;
}

void glist_prev(struct glist_t *l) {
	if (l->cur)
		l->cur=l->cur->prev;
}

int glist_isgood(struct glist_t *l) {
	if (l->cur)
		return 1;
	else
		return 0;
}

void *glist_get(struct glist_t *l) {
	if (l->cur && l->cur->obj)
		return l->cur->obj;
	else
		return NULL;
}

void *glist_find(struct glist_t *l, double key) {
	struct list_node_t *t=l->head;

	while (t) {
		if (t->key==key) break;
		t=t->next;
	}
	if (t)
		return t->obj;
	else
		return NULL;
}

int glist_sort(struct glist_t *l) {
	struct list_node_t *t,*largest,*tprev,*tnext;

	l->cur=l->head;

	while (l->cur) {
		largest=l->cur;
		t=l->cur->next;

		// find node with largest key.
		while (t) {
			if (t->key>largest->key)
				largest=t;
			t=t->next;
		}

		// if largest element isnt where its supposed to be, do magic.
		if (largest!=l->cur) {
			// remove largest entry from list.
			if (largest->prev)
				largest->prev->next=largest->next;
			if (largest->next)
				largest->next->prev=largest->prev;

			if (l->cur->prev)
				l->cur->prev->next=largest;
			else
				largest->prev=NULL;

			largest->prev=l->cur->prev;
			l->cur->prev=largest;
			largest->next=l->cur;
		}

		// continue sorting rest of list.
		l->cur=l->cur->next;
	}
}

int glist_count(struct glist_t *l) {
	struct list_node_t *t=l->head;
	int i=0;

	while (t) {
		i++;
		t=t->next;
	}

	return i;
}

void glist_add(struct glist_t *l, void *obj, double key) {
	struct list_node_t *tmp, *tt;

	// create the container-node.
	struct list_node_t *t=(struct list_node_t*)malloc(sizeof(struct list_node_t));
	t->key=key;
	t->obj=obj;
	t->next=NULL;
	t->prev=NULL;

	if (l->insert_type==INSERT_SORTED) {
		if (l->head) {
			tmp=l->head;
			while (tmp->next && (tmp->key > key))
				tmp=tmp->next;

			// means tmp->next == 0, so we're at last element.
			if (tmp->key > key) {
				tmp->next=t;
				t->prev=tmp;
			} else {
				tt=tmp;
				tt=tmp->prev;
				tmp->prev=t;
				t->next=tmp;
				if (tt)
					tt->next=t;
				t->prev=tt;
				if (tmp==l->head)
					l->head=t;
			}
		} else
			l->head=l->tail=t;

	// insert unsorted reverse.
	} else if (l->insert_type==INSERT_REVERSE) {
		if (l->head)
			l->head->prev=t;
		t->next=l->head;
		l->head=t;

	// else, just put it in the list.
	} else {
		if (l->tail) {
			t->prev=l->tail;
			l->tail->next=t;
			l->tail=t;
		} else
			l->head=l->tail=t;
	}
}
