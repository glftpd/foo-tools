/*
 * Defines some routines to sort about everything with.
 *
 * <flower@tanesha.net>
 * $Id: sortedlist.h,v 1.1 2001/11/25 15:34:50 sd Exp $
 */
#ifndef _sortedlist_h
#define _sortedlist_h

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
