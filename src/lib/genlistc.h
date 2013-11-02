/**
 * Generic sortable/searchable list for items/structs.
 *
 **
 * $Id: genlistc.h,v 1.1.1.1 2001/04/30 10:49:37 sd Exp $
 * $Source: /var/cvs/foo/src/lib/genlistc.h,v $
 * Author: Soren
 */
#include <stdio.h>

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









