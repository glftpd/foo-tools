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
