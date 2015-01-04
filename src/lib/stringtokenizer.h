#ifndef _stringtokenizer_h
#define _stringtokenizer_h

/*
 * Small lib to do safe tokenizing of strings (c) tanesha team.
 */

struct stringtokenizer_item_t {
	char *token;

	struct stringtokenizer_item_t *next;
};

typedef struct stringtokenizer_item_t st_item;

struct stringtokenizer_t {
	st_item *list;

	st_item *cur;
};

typedef struct stringtokenizer_t stringtokenizer;

/*
 * initializer method. str is input string, and split is string
 * to split on.
 */
void st_initialize(stringtokenizer *st, char *str, char *split);

/*
 * destructor for stringtokenizer structure.
 */
void st_finalize(stringtokenizer *st);

/*
 * returns number of elements left in the tokenizer.
 */
int st_count(stringtokenizer *st);

/*
 * returns 1 if more elements are left, and 0 if not.
 */
int st_hasnext(stringtokenizer *st);

/*
 * returns next item, or NULL if no more exists.
 */
char *st_next(stringtokenizer *st);

/*
 * resets the tokenizer, so you can use the next/hasnext methods
 * once again.
 */
void st_reset(stringtokenizer *st);

#endif
