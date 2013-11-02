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
 * Library implementing a searchable string list.
 *
 * Changed line to be *data instead of line[300]
 *
 **
 * $Id: strlist.h,v 1.2 2003/01/22 14:31:29 sorend Exp $
 * $Source: /home/cvs/footools/footools/src/collection/strlist.h,v $
 * Author: Soren
 */

#ifndef _strlist_h
#define _strlist_h

// max line length in files, used by str_load.
#define STR_MAXLINELEN 1024

// flags for str_search
#define STR_NOCASE (1 << 0)
#define STR_FNMATCH (1 << 2)

struct strlist {
	char *data;
	struct strlist *next;
};

typedef struct strlist strlist_t;

struct strlist_iterator {
	strlist_t *cur;
};

typedef struct strlist_iterator strlist_iterator_t;

/*
 * Count number of items in the list.
 */
long str_count(strlist_t *l);

/*
 * Search for an item in the list.
 */
strlist_t *str_search(strlist_t *l, char *s, int flags);

/*
 * Add an item to the list.
 */
strlist_t *str_add(strlist_t *l, char *s);
strlist_t *str_add_last(strlist_t *l, char *s);

/*
 * Loads the list from a file.
 */
strlist_t *str_load(strlist_t *l, char *fn);

/*
 * Closes a list.
 */
void str_close(strlist_t *l);

/*
 * joins a stringlist into a char with delim inbetween each element.
 * the returned char* must be freed by user !  if no elements in list
 * "" is returned.
 */
char *str_join(strlist_t *l, char *delim);

/*
 * Returns the reverse ordered list.
 */
strlist_t *str_reverse(strlist_t *l);

/*
 * Iterator methods.
 */
strlist_iterator_t * str_iterator(strlist_t *l);
int str_iterator_hasnext(strlist_iterator_t *i);
char *str_iterator_next(strlist_iterator_t *i);

/*
 * Strlist_T operations
 */
strlist_t *str_op_and(strlist_t *p, strlist_t *q);
strlist_t *str_op_or(strlist_t *p, strlist_t *q);

/*
struct strlist_t *str_op_xor(struct strlist_t *p, struct strlist_t *q);
struct strlist_t *str_op_neg(struct strlist_t *p, struct strlist_t *q);
 */

#endif
