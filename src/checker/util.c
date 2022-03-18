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


#include "util.h"
#include "frame.h"
#include <lib/macro.h>
#include <collection/hashtable.h>
#include <lib/sfv.h>
#include <lib/dirlist.h>
#include <lib/stringtokenizer.h>
#include <util/strmatch.h>
#include <string.h>
#include <fnmatch.h>


dirlist_t * util_get_dirlist(hashtable_t *conf, char *dir) {
	dirlist_t *dl;

	dl = (dirlist_t *)ht_get(conf, PROPERTY_DIRLIST);

	if (!dl) {
		dl = (dirlist_t *)malloc(sizeof(dirlist_t ));
		dirlist_init(dl, dir);

		ht_put_obj(conf, PROPERTY_DIRLIST, dl);	
	}

	return dl;
}

sfv_list_t * util_get_sfvlist(hashtable_t *conf, char *dir) {
	sfv_list_t *sfv;

	sfv = (sfv_list_t*)ht_get(conf, PROPERTY_SFVLIST);

	if (!sfv) {
		sfv = sfv_list_load_path(dir);
		ht_put_obj(conf, PROPERTY_SFVLIST, sfv);
	}

	return sfv;
}


char * util_replacer_env(hashtable_t *env, char *buf) {
	char *out;
	hashtable_item_t *tmp;
	struct macro_list *ml = 0;

	ht_reset(env);

	while (tmp = ht_next(env)) {
		if (tmp->is_obj)
			continue;

		ml = ml_addstring(ml, tmp->key, tmp->value);
	}

	out = ml_replacebuf(ml, buf);

	ml_free(ml);

	return out;
}

char * util_replacer(hashtable_t *conf, char *buf) {
	hashtable_t *env;
	char *out;

	env = ht_get_tree(conf, PROPERTY_DEFAULT_REPLACER_ENV, HT_DEF_DELIM);

	out = util_replacer_env(env, buf);

	ht_finalize(env);

	return out;
}

void util_replacer_set(hashtable_t *conf, char *key, char *val) {
	char *tkey;

	tkey = (char*)malloc(strlen(key) + strlen(PROPERTY_DEFAULT_REPLACER_ENV) + 2);

	sprintf(tkey, "%s%c%s", PROPERTY_DEFAULT_REPLACER_ENV, HT_DEF_DELIM, key);
	ht_put(conf, tkey, val);
}

void error_printf(hashtable_t *conf, char *buf, char *err) {
	struct macro_list *ml = 0;
	char *errbuf, *out;

	ml = ml_addstring(ml, "MSG", err);

	errbuf = ml_replacebuf(ml, buf);
	
	ml_free(ml);

	out = util_replacer(conf, errbuf);

	printf(out);

	free(out);
	free(errbuf);
}

void msg_printf(hashtable_t *conf, char *buf, char *msg, char *status) {
	struct macro_list *ml = 0;
	char *errbuf, *out;

	ml = ml_addstring(ml, "MSG", msg);
	ml = ml_addstring(ml, "STATUS", status);

	errbuf = ml_replacebuf(ml, buf);

	ml_free(ml);

	out = util_replacer(conf, errbuf);

	printf(out);

	free(out);
	free(errbuf);
}

void util_printf(hashtable_t *conf, char *buf) {
	char *out;

	out = util_replacer(conf, buf);

	printf(out);

	free(out);
}

int util_path_stmatch(stringtokenizer *s, char *dir) {
	char *tmp;
	int ret = 0;

	st_reset(s);
	while (tmp = st_next(s)) {
		if (!fnmatch(tmp, dir, 0))
			ret = 1;
	}

	return ret;
}

int util_path_match(char *p, char *dir) {
	stringtokenizer st;
	char *tmp;
	int ret = 0;

	st_initialize(&st, p, "|");

	ret = util_path_stmatch(&st, dir);

	st_finalize(&st);

	return ret;
}
