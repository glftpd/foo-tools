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
 * Module to add site.nfo to each dir of a release.
 * Author, Soren.
 * $Id: mod_sitenfoadd.c,v 1.2 2003/06/20 10:41:43 sorend Exp $
 */

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>


// project includes
#include "../foo-pre.h"
#include "mod_sitenfoadd.h"

// footools includes
#include <collection/hashtable.h>
#include <lib/macro.h>

// for expand func
#define MAGIC_NUM '@'
#define MAGIC_CHAR '?'
#define MAGIC_OTHER '%'
#define MAGIC_ALL '$'
char *CHARS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
char *NUMS = "0123456789";
char *OTHER = ",._-^+='\"";
char *ALL = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,._-^+='\"";


hashtable_t *_mod_sitenfoadd_cfg = 0;

void set_config(hashtable_t *cfg) {
        _mod_sitenfoadd_cfg = cfg;
}
hashtable_t *get_config() {
        return _mod_sitenfoadd_cfg;
}

struct tm *mod_sitenfoadd_now = 0;
int mod_sitenfoadd_dir_func(char *path, char *argv[]);

module_list_t mod_sitenfoadd_info = {
	// module name
	"site nfo adder",

	// module dir func
	mod_sitenfoadd_dir_func,

	// module file func
	0,

	// module rel func
	0,

	// module struct entry
	0
};

// function to return module info of this module.
module_list_t *module_loader() {
	return &mod_sitenfoadd_info;
}


// expansion rules.
int mod_sitenfoadd_expand(char *buf) {

	char *t;
	int ran = 0;
	int i = 0;
	struct timeval tv;

	gettimeofday(&tv, 0);
	srand(tv.tv_usec);

	for (t = buf; *t; t++) {

		ran = rand();

		switch (*t) {

		case MAGIC_NUM : *t = NUMS[ran % strlen(NUMS)];
			i++;
			break;

		case MAGIC_CHAR : *t = CHARS[ran % strlen(CHARS)];
			i++;
			break;

		case MAGIC_OTHER : *t = OTHER[ran % strlen(OTHER)];
			i++;
			break;

		case MAGIC_ALL : *t = ALL[ran % strlen(ALL)];
			i++;
			break;

		}
	}

	return i;
}



void mod_sitenfoadd_add_nfo(char *path, char *rel) {

	struct macro_list *ml = 0;
	char *tmp, buf[1024], *fn, fbuf[1024];
	FILE *f;
	time_t now;

	tmp = ht_get(get_config(), PROPERTY_MOD_SITENFOADD_NFO);
	fn = ht_get(get_config(), PROPERTY_MOD_SITENFOADD_FILENAME);

	if (!(tmp && fn))
		return;

	// release
	ml = ml_addstring(ml, "RELEASE", rel);

	if (mod_sitenfoadd_now == 0) {

		now = time(0);
		mod_sitenfoadd_now = localtime(&now);
	}

	// date
	strftime(buf, 1024, "%Y-%m-%d", mod_sitenfoadd_now);
	ml = ml_addstring(ml, "DATE", buf);

	// time
	strftime(buf, 1024, "%H:%M:%S", mod_sitenfoadd_now);
	ml = ml_addstring(ml, "TIME", buf);

	tmp = ml_replacebuf(ml, tmp);

	// expand
	strcpy(buf, fn);
	mod_sitenfoadd_expand(buf);

	sprintf(fbuf, "%s/%s", path, buf);

	f = fopen(fbuf, "w");
	if (f) {
		fwrite(tmp, strlen(tmp), 1, f);
		fclose(f);
		printf(" .. Created nfo %s\n", buf);
	} else
		printf(" .. Couldnt create %s\n", buf);

	free(tmp);

	// printf("%s\n", mod);
}


// function for module.
int mod_sitenfoadd_dir_func(char *path, char *argv[]) {

	mod_sitenfoadd_add_nfo(path, argv[1]);

	return 1;
}


