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
/*
 * nfo cleaner (c) Tanesha Team
 *
 * Cleans much faster/better than old cleaner.  Also supports
 * prepend/append of add to .nfo.   /sorend.
 *
 * CHANGELOG:
 * + version 2.o, modularized for frame checker.
 *
 * $Id: mod_nfo.c,v 1.2 2003/01/22 14:31:29 sorend Exp $
 */
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>

#include "mod_nfo.h"

// include frame env.
#include <checker/frame.h>

#include <collection/strlist.h>
#include <collection/hashtable.h>
#include <lib/macro.h>
#include <util/linefilereader.h>
#include <util/strmatch.h>
#include <util/release.h>

// ugly hack ;[
strlist_t *_nfo_cleanlist_normal = 0;

int nfo_str_trim(char *in, char *out, int len) {
	char *start, *tmp, *end;

	tmp = strdup(in);

	start = tmp;

	while (*start && ((*start == ' ') || (*start == '\t')))
		start++;

	end = start;

	while (*end)
		end++;

	if (end > start) {
		end--;

		while ((end > start) && ((*end == ' ') || (*end == '\t')))
			end--;
	}

	if (*end != 0)
		*(end + 1) = 0;

	strncpy(out, start, len);

	free(tmp);

	return 1;
}

strlist_t * nfo_clean_load(strlist_t *l, char *fn) {
	linefilereader_t lr;
	char buf[300], trimmed[300];

	if (lfr_open(&lr, fn) < 0) {
		lfr_close(&lr);
		return 0;
	}

	while (lfr_getline(&lr, buf, 300) > -1) {
		_nfo_cleanlist_normal = str_add(_nfo_cleanlist_normal, buf);
		nfo_str_trim(buf, trimmed, 300);

		if (strlen(trimmed) > 0)
			l = str_add(l, trimmed);
	}

	lfr_close(&lr);

	return l;
}

int nfo_reg_match(char *s, char *p) {
	regex_t re;
	int rc;

	rc = regcomp(&re, p, REG_NOSUB | REG_ICASE | REG_EXTENDED);

	// this is not a regexp.
	if (rc != 0)
		return 0;

	rc = regexec(&re, s, 0, 0, 0);

	regfree(&re);

	if (rc == 0)
		return 1;

	return 0;
}


int nfo_find_match(char *s, strlist_t *l) {
	strlist_iterator_t *i;
	char *t;

	i = str_iterator(l);

	// excact match.
	while (t = str_iterator_next(i))
		if (strstr(s, t))
			break;

	free(i);

	if (t)
		return 1;

	// wildcard match.
	i = str_iterator(_nfo_cleanlist_normal);
	while (t = str_iterator_next(i))
		if (strmatch_filename(t, s, 0))
			break;

	free(i);

	if (t)
		return 1;

	return 0;
}

char *nfo_str_concat(strlist_t *l, char *mid) {
	strlist_iterator_t *i;
	int midlen, newlen = 0;
	char *buf, *t;

	midlen = strlen(mid);

	// find length of new string.
	i = str_iterator(l);

	while (t = str_iterator_next(i))
		newlen += strlen(t) + midlen;

	free(i);

	buf = (char*)malloc(newlen + 1);
	*buf = 0;

	i = str_iterator(l);

	while (t = str_iterator_next(i)) {
		strcat(buf, t);
		strcat(buf, mid);
	}

	free(i);

	return buf;
}


char *nfo_fillout_vars(hashtable_t *h, char *s, char *d) {
	struct macro_list *ml = NULL;
	char rel[300], the_date[30], the_time[30];
	char *buf;
	long now;
	struct tm *now_s;

	// build variables for macro rep list.
	release_get_with_subdir(d, rel, 300);

	now = time(0);
	now_s = localtime(&now);

	sprintf(the_date, "%02d-%02d-%02d",
			now_s->tm_mon + 1,
			now_s->tm_mday,
			now_s->tm_year+1900);

	sprintf(the_time, "%02d:%02d",
			now_s->tm_hour,
			now_s->tm_min);

	// build the macro rep list.
	ml = ml_addstring(ml, "USER", ht_get(h, "user"));
	ml = ml_addstring(ml, "DIR", rel);
	ml = ml_addstring(ml, "TIME", the_time);
	ml = ml_addstring(ml, "DATE", the_date);

	buf = ml_replacebuf(ml, s);

	return buf;
}

int nfo_init(hashtable_t *conf, char *file, char *dir) {

	return 1;
}

// method for 'dupe' part of checker framework.
int nfo_dupe(hashtable_t *conf, char *file, char *dir) {
	char *propfile, *tmp;
	FILE *f;

	tmp = ht_get(conf, PROPERTY_NFO_UNWANTEDDIR);

	// we dont have unwanted dir, so we dont care.
	if (!tmp)
		return 0;

	propfile = (char*)malloc(strlen(tmp) + strlen(file) + 2);
	sprintf(propfile, "%s/%s", tmp, file);

	f = fopen(propfile, "r");

	free(propfile);

	if (!f)
		return 1;

	fclose(f);

	ht_put(conf, PROPERTY_ERRORMSG, "Nfo is unwanted propaganda-shit");

	return 0;
}

void nfo_append_add(hashtable_t *conf, FILE *f, char *add, char *dir) {
	strlist_t *tmp;
	char *sum, *filled;

	if (!add)
		return;

	tmp = str_load(0, add);
	sum = nfo_str_concat(tmp, "\n");
	filled = nfo_fillout_vars(conf, sum, dir);

	// write to file.
	fwrite(filled, 1, strlen(filled), f);

	// clean up.
	free(filled);
	free(sum);
	str_close(tmp);
}


// method for 'check' part of checker framework.
int nfo_check(hashtable_t *conf, char *file, char *dir, long crc) {
	char *tmpstr, *connfo, *nfofilename, *newnfo, *tmp;
	strlist_t *cleanlist = 0, *goodnfo = 0, *tmpnfo = 0;
	FILE *f;
	strlist_iterator_t *iter;

	nfo_init(conf, file, dir);

	// load the nfo (nfo in reverse).
	nfofilename = ht_get(conf, PROPERTY_PATH);
	tmpnfo = str_load(tmpnfo, nfofilename);

	// load the clean list.
	tmpstr = ht_get(conf, PROPERTY_NFO_CLEANLISTFILE);
	if (tmpstr)
		cleanlist = nfo_clean_load(cleanlist, tmpstr);

	// remove bad lines from nfo (and get it in correct order).
	iter = str_iterator(tmpnfo);
	while (tmp = str_iterator_next(iter)) {
		if (nfo_find_match(tmp, cleanlist))
			continue;

		goodnfo = str_add_last(goodnfo, tmp);
	}

	// open new nfofile.
	f = fopen(nfofilename, "w");

	if (!f)
		return 0;

	// add prepend nfo.
	tmpstr = ht_get(conf, PROPERTY_NFO_PREPENDFILE);
	nfo_append_add(conf, f, tmpstr, dir);

	newnfo = nfo_str_concat(goodnfo, "\n");
	if (fwrite(newnfo, 1, strlen(newnfo), f) != strlen(newnfo)) {
#ifdef _FRAME_
		error_printf(conf, ht_get(conf, PROPERTY_TMPL_ERRORMSG), "Writing cleaned nfo");
#else
		printf(ht_get(conf, PROPERTY_TMPL_ERRORMSG), "Writing cleaned nfo");
#endif
	}
	free(newnfo);

	// add append nfo.
	tmpstr = ht_get(conf, PROPERTY_NFO_APPENDFILE);
	nfo_append_add(conf, f, tmpstr, dir);

	fclose(f);

	return 1;
}
