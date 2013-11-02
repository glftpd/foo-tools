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

#ifndef _hashtable_h
#define _hashtable_h

#define HT_DEF_DELIM '_'
#define HT_COMMENT_PREFIXES "#;"

struct hashtable_item {
	char *key;
	char *value;
	int is_obj;

	struct hashtable_item *next;
};

typedef struct hashtable_item hashtable_item_t;

struct hashtable {
	hashtable_item_t *list, *cur;
};

typedef struct hashtable hashtable_t;

char *ht_get(hashtable_t *ht, char *key);

int ht_put(hashtable_t *ht, char *key, char *value);
int ht_put_obj(hashtable_t *ht, char *key, void *obj);
int ht_put_obj_free(hashtable_t *ht, char *key, void *obj, int dofree);

// returns number of elements removed.
int ht_remove(hashtable_t *ht, char *key);

int ht_init(hashtable_t *ht);

int ht_finalize(hashtable_t *ht);

int ht_size(hashtable_t *ht);

hashtable_t *ht_get_tree(hashtable_t *ht, char *leaf, char delim);

/*
 * Methods to load hashtable_t from files.
 */
int ht_load(hashtable_t *ht, char *fn);
int ht_load_prop(hashtable_t *ht, char *fn, char delim);

// methods for traversing the hashtable_t.
hashtable_item_t *ht_next(hashtable_t *ht);
int ht_hasnext(hashtable_t *ht);
int ht_reset(hashtable_t *ht);

#endif
