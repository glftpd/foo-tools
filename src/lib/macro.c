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
 * Library to replace items in text files. Note that this one should be replaced
 * by bio's replacer lib, which is fancier.
 *
 **
 * $Id: macro.c,v 1.2 2003/01/22 14:31:29 sorend Exp $
 * $Source: /home/cvs/footools/footools/src/lib/macro.c,v $
 * Author: Soren
 */
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include "macro.h"

struct macro_donelist *ml_adddone(struct macro_donelist *l, char *s) {
	struct macro_donelist *t = (struct macro_donelist*)
		malloc(sizeof(struct macro_donelist));

	t->str = (char*)malloc(strlen(s) + 1);
	strcpy(t->str, s);

	t->next = l;

	return t;
}

long ml_donelen(struct macro_donelist *l) {
	if (l && l->str)
		return strlen(l->str) + ml_donelen(l->next);
	else
		return 0;
}

void ml_reversedone(struct macro_donelist *l, char *b) {
	if (l) {
		ml_reversedone(l->next, b);

		strcat(b, l->str);
	}
}

void ml_showlist(struct macro_list *l) {
	struct macro_list *t;

	for (t = l; t; t = t->next)
		printf("mac: %s\n", t->mac_key);
}

struct macro_list *ml_addfloat(struct macro_list *l, char *r, double f) {
	struct macro_list *tl = (struct macro_list*)malloc(sizeof(struct macro_list));

	double *tf = (double*)malloc(sizeof(double));
	*tf = f;

	tl->mac_rep = (char*)tf;
	tl->mac_type = ML_FLOAT;
	tl->mac_key = (char*)malloc(strlen(r)+1);
	strcpy(tl->mac_key, r);
	tl->next = l;

	return tl;
}

struct macro_list *ml_addstring(struct macro_list *l, char *r, char *s) {
	struct macro_list *tl = (struct macro_list*)malloc(sizeof(struct macro_list));

	tl->mac_rep = (char*)malloc(strlen(s)+1);
	strcpy(tl->mac_rep, s);
	tl->mac_type = ML_STRING;
	tl->mac_key = (char*)malloc(strlen(r)+1);
	strcpy(tl->mac_key, r);
	tl->next = l;

	return tl;
}

struct macro_list *ml_addint(struct macro_list *l, char *r, int i) {
	struct macro_list *tl = (struct macro_list*)malloc(sizeof(struct macro_list));
	int *ti = (int*)malloc(sizeof(int));
	*ti = i;
	
	tl->mac_rep = (char*)ti;
	tl->mac_type = ML_INT;
	tl->mac_key = (char*)malloc(strlen(r)+1);
	strcpy(tl->mac_key, r);
	tl->next = l;

	return tl;
}

struct macro_list *ml_addchar(struct macro_list *l, char *r, char c) {
	struct macro_list *tl = (struct macro_list*)malloc(sizeof(struct macro_list));
	char *cc = (char*)malloc(sizeof(char));
	*cc = c;

	tl->mac_rep = (char*)cc;
	tl->mac_type = ML_CHAR;
	tl->mac_key = (char*)malloc(strlen(r)+1);
	strcpy(tl->mac_key, r);
	tl->next = l;

	return tl;
}

struct macro_list *ml_gotmacro(struct macro_list *l, char *m) {
	if (!m)
		return NULL;
	else if (l && !strcmp(m, l->mac_key))
		return l;
	else if (l)
		return ml_gotmacro(l->next, m);
	else
		return NULL;
}

char *ml_replacebuf(struct macro_list *l, char *buf) {
	struct macro_list *tm, *tl = l;
	struct macro_donelist *dl = NULL, *tdl;
	char *newbuf, *tmpbuf, *t, *tt, *o, *mac = 0, *repf = 0, *last = 0;
	double *f;
	int *i;
	char *c;

	// this is not bad i know, but we cannot know what the
	// replaced macro expands to.
	char repbuf[1024];
	repbuf[0] = 0;

	o = buf;

	for (o = buf; *o; o++) {
		if (!last)
			last = o;

		if (!strncmp(o, "%[", 2)) {
			if (mac) {
				free(mac);
				mac = 0;
			}
			if (repf) {
				free(repf);
				repf = 0;
			}

			t = o;
			while (*t && (*t != ']')) t++;
			if (*t != ']')
				continue;


			repf = (char*)malloc(t - o - 1);
			*repf = 0;
			strncpy(repf, o + 2, t - o - 2);
			repf[t - o - 2] = 0;

			tt = t + 1;
			while (*t && (*t != '%')) t++;
			if (*t != '%')
				continue;

			mac = (char*)malloc(t - tt + 1);
			*mac = 0;
			strncpy(mac, tt, t - tt);
			mac[t - tt] = 0;

			// check if we have this macro.
			tm = ml_gotmacro(l, mac);

			if (!tm)
				continue;

			switch (tm->mac_type) {

			case ML_STRING:
				sprintf(repbuf, repf, tm->mac_rep);
				break;
			case ML_FLOAT:
				f = (double*)tm->mac_rep;
				sprintf(repbuf, repf, *f);
				break;
			case ML_INT:
				i = (int*)tm->mac_rep;
				sprintf(repbuf, repf, *i);
				break;
			case ML_CHAR:
				c = (char*)tm->mac_rep;
				sprintf(repbuf, repf, *c);
				break;
			default:
				strcpy(repbuf, "");
			}

			tmpbuf = (char*)malloc(o - last + 1);
			strncpy(tmpbuf, last, o - last);
			tmpbuf[o - last] = 0;
			dl = ml_adddone(dl, tmpbuf);
			
			dl = ml_adddone(dl, repbuf);

			last = t + 1;
			o = t;
		}
	}

	if (last && *last)
		dl = ml_adddone(dl, last);

	newbuf = (char*)malloc(ml_donelen(dl) + 1);
	*newbuf = 0;

	ml_reversedone(dl, newbuf);

	// we're nice, so we free the list ;p
       	while (dl) {
		tdl = dl;
		dl = dl->next;
		free(tdl->str);
		free(tdl);
	}

	return newbuf;
}

void ml_free(struct macro_list *l) {
	if (l) {
		ml_free(l->next);

		if ((l->mac_type != ML_CHAR) && l->mac_rep)
			free(l->mac_rep);
		if (l->mac_key)
			free(l->mac_key);
	}
}

