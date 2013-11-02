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

#include "hashtable.h"
#include <util/linefilereader.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


int ht_size(hashtable_t *ht) {
	hashtable_item_t *tmp;
	int i = 0;

	for (tmp = ht->list; tmp; tmp = tmp->next)
		i++;

	return i;
}


hashtable_item_t *_ht_find_item(hashtable_item_t *t, char *k) {
	hashtable_item_t *tmp;

	for (tmp = t; tmp; tmp = tmp->next)
		if (tmp->key && !strcmp(tmp->key, k))
			break;

	return tmp;
}

int ht_remove(hashtable_t *ht, char *key) {
	hashtable_item_t *tmp, *n = 0, *next;
	int found = 0;

	tmp = ht->list;

	while (tmp) {
		next = tmp->next;

		if (tmp->key && strcmp(tmp->key, key)) {
			tmp->next = n;
			n = tmp;
			found++;
		} else
			free(tmp);

		// set tmp ptr to point to next.
		tmp = next;
	}

	ht->list = n;

	return found;
}

char *ht_get(hashtable_t *ht, char *key) {
	hashtable_item_t *tmp;

	tmp = _ht_find_item(ht->list, key);

	if (tmp)
		return tmp->value;

	return 0;
}

int ht_put(hashtable_t *ht, char *key, char *value) {
	hashtable_item_t *tmp;
	char *tmpstr;

	if (!(key && value))
		return 0;

	tmp = _ht_find_item(ht->list, key);

	if (tmp) {
		if (tmp->value)
			free(tmp->value);

		tmp->value = (char*)malloc(strlen(value) + 1);
		strcpy(tmp->value, value);
		tmp->is_obj = 0;
	} else {
		tmp = (hashtable_item_t *)malloc(sizeof(hashtable_item_t));

		tmp->key = (char*)malloc(strlen(key) + 1);
		tmp->value = (char*)malloc(strlen(value) + 1);

		strcpy(tmp->key, key);
		strcpy(tmp->value, value);
		tmp->is_obj = 0;

		tmp->next = ht->list;
		ht->list = tmp;
	}

	return 1;
}

int ht_put_obj(hashtable_t *ht, char *key, void *obj) {

	return ht_put_obj_free(ht, key, obj, 1);
}

int ht_put_obj_free(hashtable_t *ht, char *key, void *obj, int dofree) {
	hashtable_item_t *tmp;
	char *tmpstr;

	if (!(key && obj))
		return 0;

	tmp = _ht_find_item(ht->list, key);

	if (tmp) {
		if (dofree && tmp->value)
			free(tmp->value);

		tmp->value = (char*)obj;
		tmp->is_obj = 1;
	} else {
		tmp = (hashtable_item_t *)malloc(sizeof(hashtable_item_t));

		tmp->key = (char*)malloc(strlen(key) + 1);
		strcpy(tmp->key, key);

		tmp->value = (char*)obj;
		tmp->is_obj = 1;

		tmp->next = ht->list;
		ht->list = tmp;
	}

	return 1;
}

int ht_init(hashtable_t *ht) {
	ht->list = 0;
}

int ht_finalize(hashtable_t *ht) {
	hashtable_item_t *del, *tmp = ht->list;

	while (tmp) {
		del = tmp;
		tmp = tmp->next;

		if (del->key)
			free(del->key);
		if (del->value)
			free(del->value);

		free(del);
	}
}

hashtable_t *ht_get_tree(hashtable_t *ht, char *leaf, char delim) {
	hashtable_item_t *tmp;
	hashtable_t *newht;
	char *buf;

	buf = (char*)malloc(strlen(leaf) + 2);
	sprintf(buf, "%s%c", leaf, (delim==0?HT_DEF_DELIM:delim));

	newht = (hashtable_t*)malloc(sizeof(hashtable_t));
	ht_init(newht);

	for (tmp = ht->list; tmp; tmp = tmp->next)
		if (tmp->key && !strncmp(tmp->key, buf, strlen(buf)))
			ht_put(newht, tmp->key + strlen(buf), tmp->value);

	free(buf);
								 
	return newht;
}

int ht_load(hashtable_t *ht, char *fn) {

	return ht_load_prop(ht, fn, '=');
}


char *_ht_trim(char *s) {
	char *begin, *tmp;

	tmp = begin = s;

	/* skip initial spaces.
	while ((*begin == ' ') || (*begin == '\t') || (*begin == '\n') || (*begin == '\r'))
		begin++;

	tmp = begin;
	*/

	while (*tmp)
		tmp++;

	if (tmp == begin)
		return begin;

	tmp--;

	while ((tmp > begin) && ((*tmp == ' ') || (*tmp == '\t') || (*tmp == '\n') || (*tmp == '\r')))
		tmp--;

	*(tmp+1) = 0;

	return begin;
}


int _ht_replace(char *b, char *n, char *r) {
	char *t, *save;
	int i=0;

	while (t=strstr(b, n)) {
		save = (char*)malloc(strlen(t)-strlen(n)+1);
		strcpy(save, t+strlen(n));
		*t=0;
		strcat(b, r);
		strcat(b, save);
		free(save);
		i++;
	}

	return i;
}

char *_ht_readfile(char *fn) {
	FILE *f;
	char *buf = 0;
	long len;

	f = fopen(fn, "r");

	if (!f)
		return 0;

	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fseek(f, 0, SEEK_SET);

	buf = (char*)malloc(len + 1);
	fread(buf, len, 1, f);
	fclose(f);

	buf[len] = 0;

	return buf;
}


int ht_load_prop(hashtable_t *ht, char *fn, char delim) {
	linefilereader_t lr;
	char buf[1024], *tmp, *key, *val;

	if (lfr_open(&lr, fn) < 0) {
		lfr_close(&lr);
		return 0;
	}

	while (lfr_getline(&lr, buf, 1024) > -1) {
		if (strchr(HT_COMMENT_PREFIXES, buf[0]))
			continue;

		tmp = strchr(buf, delim);

		if (!tmp)
			continue;

		*tmp = 0;

		key = _ht_trim(buf);
		val = _ht_trim(tmp + 1);

		_ht_replace(val, "\\n", "\n");
		_ht_replace(val, "\\r", "\r");

#ifdef HT_SUPPORT_FILEPROP
		if (!strncmp(val, "file:", 5)) {
			val = _ht_readfile(val + 5);
			ht_put(ht, key, val);
			free(val);
		} else
#endif
			ht_put(ht, key, val);
	}

	lfr_close(&lr);

	return 1;
}

hashtable_item_t *ht_next(hashtable_t *ht) {
	hashtable_item_t *tmp;

	tmp = ht->cur;

	if (ht->cur)
		ht->cur = ht->cur->next;

	return tmp;
}

int ht_reset(hashtable_t *ht) {
	ht->cur = ht->list;
}

int ht_hasnext(hashtable_t *ht) {
	if (ht->cur)
		return 1;

	return 0;
}
