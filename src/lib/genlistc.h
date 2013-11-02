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
 * $Id: genlistc.h,v 1.2 2003/01/22 14:31:29 sorend Exp $
 * $Source: /home/cvs/footools/footools/src/lib/genlistc.h,v $
 * Author: Soren
 */
#include <stdio.h>
#include <stdlib.h>

#define INSERT_NOORDER	0
#define	INSERT_SORTED	1
#define	INSERT_REVERSE	2

struct list_node_t {
	void *obj;
	double key;
	struct list_node_t *next;
	struct list_node_t *prev;
};

struct glist_t {
	struct list_node_t *cur, *head, *tail;
	int insert_type;
};

void glist_init(struct glist_t *l, int type);
void glist_deinit(struct glist_t *l);
void glist_reset(struct glist_t *l);
void glist_next(struct glist_t *l);
void glist_prev(struct glist_t *l);
int glist_isgood(struct glist_t *l);
void *glist_get(struct glist_t *l);
void *glist_find(struct glist_t *l, double key);
int glist_sort(struct glist_t *l);
void glist_add(struct glist_t *l, void *obj, double key);
int glist_count(struct glist_t *l);
void glist_resettail(struct glist_t *l);









