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
 * Module for extracting info from .nfo files and adding to glftpd.log
 * Author, Soren.
 * $Id: mod_nfohandler.c,v 1.3 2003/06/20 11:32:28 sorend Exp $
 */

#include <sys/types.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include "../foo-pre.h"

#include <collection/hashtable.h>
#include <lib/gllogs.h>
#include <lib/macro.h>
#include "mod_nfohandler.h"

hashtable_t *_mod_nfohandler_cfg = 0;

void set_config(hashtable_t *cfg) {
	_mod_nfohandler_cfg = cfg;
}
hashtable_t *get_config() {
	return _mod_nfohandler_cfg;
}

int mod_nfohandler_file_func(char *filepath, char *argv[]);

module_list_t mod_nfohandler_info = {
	// module name
	"nfo info extractor",

	// module dir func
	0,

	// module file func
	mod_nfohandler_file_func,

	// module rel func
	0,

	// module struct entry
	0
};

// function to return module info of this module.
module_list_t *module_loader() {
	return &mod_nfohandler_info;
}

char *mod_nfohandler_do_regex(char *nfo, char *regex) {
	regex_t preg;
	int rc;
	regmatch_t match[3];
	char *good;

	//printf("regex: %s\nnfo: %s\n", regex, nfo);

	rc = regcomp(&preg, regex, REG_EXTENDED | REG_ICASE);
	
	if (rc != 0) {
		printf(" .. Bad nfohandler regex %s - tell sysop!\n", regex);

		return 0;
	}

	rc = regexec(&preg, nfo, 3, (regmatch_t*)&match, 0);

	// no match.
	if (rc != 0)
		return 0;

	// got a match, extract it.
	rc = match[1].rm_eo - match[1].rm_so;
	good = malloc(rc + 1);

	memcpy(good, nfo + match[1].rm_so, rc);
	*(good + rc) = 0;

	regfree(&preg);

	return good;
}


int mod_nfohandler_file_func(char *filepath, char *argv[]) {

	hashtable_t *regexs;
	hashtable_item_t *hi;
	struct macro_list *ml = 0;
	char *tmp, *out, *nfo;
	FILE *fh;
	size_t fsize;

	tmp = strrchr(filepath, '.');

	if (tmp)
		tmp++;
	else
		tmp = filepath;

	if (strcasecmp(tmp, "nfo")) {
		// printf(" .. %s -> not nfo, continue\n", tmp);
		return 1;
	}

	out = ht_get(get_config(), PROPERTY_MOD_NFOHANDLER_OUTPUT);

	// no output, no point in continuing with this module.
	if (!out)
		return 0;

	fh = fopen(filepath, "r");

	//printf(" .. parsing nfo: %s\n", filepath);

	// could not open file
	if (!fh)
		return 1;

	// find size
	fseek(fh, 0, SEEK_END);
	fsize = ftell(fh);
	fseek(fh, 0, SEEK_SET);

	// read into nfo
	nfo = malloc(fsize);
	fread(nfo, fsize, 1, fh);

	fclose(fh);

	regexs = ht_get_tree(get_config(), "mod_nfohandler", '.');

	ht_reset(regexs);

	while (ht_hasnext(regexs)) {

		hi = ht_next(regexs);

		// skip .output key.
		if (!strncmp(hi->key, "output", 6))
			continue;

		tmp = mod_nfohandler_do_regex(nfo, hi->value);

		if (tmp) {
			ml = ml_addstring(ml, hi->key, tmp);
			free(tmp);
		} else
			ml = ml_addstring(ml, hi->key, "");

	}

	// free the loaded nfo
	free(nfo);

	// add NFO key.
	tmp = strrchr(filepath, '/');
	if (tmp)
		tmp++;
	else
		tmp = filepath;
	ml = ml_addstring(ml, "NFO", tmp);

	// expand the buffer
	tmp = ml_replacebuf(ml, out);

	// print it.
	//printf("tmp = %s\n", tmp);

	// log it
	gl_gllog_add(tmp);



	return 1;
}


