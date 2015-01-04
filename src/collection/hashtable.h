
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

// returns number of elements removed.
int ht_remove(hashtable_t *ht, char *key);

int ht_init(hashtable_t *ht);

int ht_finalize(hashtable_t *ht);

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
