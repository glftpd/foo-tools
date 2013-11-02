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
 * $Id: dirlog_wrap.c,v 1.2 2003/01/22 14:31:30 sorend Exp $
 */
#include <string.h>

#include "dirlog.h"
#include "dirlog_wrap.h"

#include <collection/sortedlist.h>
#include <collection/hashtable.h>
#include <lib/dirlist.h>


di_t * _dirlog_clone(dirlog_item_t *t, long pos) {
	di_t *tmp = malloc(sizeof(di_t));

	memcpy(&tmp->di, t, sizeof(dirlog_item_t));
	tmp->pos = pos;

	return tmp;
}


int _dirlog_wrap_getmaindir(char *dir, char *main) {
	char *tmp;
	int s = 0, oke = 0;

	strcpy(main, dir);

	// advance and skip main '/'
	tmp = main + strlen(STARTDIR) + 1;

	// skip some dirs.
	while (*tmp) {
		if (*tmp == '/') {
			s++;

			if (s >= SUBDIR_LEVEL) {
				*tmp = 0;
				oke++;
			}
		}

		if (*tmp)
			tmp++;
	}

	return oke || (s == SUBDIR_LEVEL - 1);
}



int dirlog_wrap_getnew(dirlog_t *dl, sortedlist_t *rels, int show, char *startdir) {

	long s, i;
	dirlog_item_t dlent;
	char buf[300], gooddir[300];
	int found = 0, dirlogpos = 1;

	hashtable_t mdirs;
	release_t *tmp;

	ht_init(&mdirs);

	s = dirlog_getsize(dl);

	for (i = s; i > 0; i--) {

		if (!dirlog_getentry(dl, (i - 1), &dlent)) {
			printf("ERROR GETTING ENTRY %d\n", (i - 1));

			break;
		}

		// skip dir if not unked or new.
		if (dlent.status != 0 && dlent.status != 2)
			continue;

		// fix for too long dirnames.
		strcpy(gooddir, dlent.dirname);
		gooddir[254] = 0;

		if (strncmp(gooddir, startdir, strlen(startdir)))
			continue;

		if (!_dirlog_wrap_getmaindir(gooddir, buf))
			continue;

		dlent.nxt = malloc(sizeof(dirlist_t));
		dirlist_init((dirlist_t*)dlent.nxt, dlent.dirname);

		if (!dirlist_readdir((dirlist_t*)dlent.nxt)) {
			dirlist_closedir((dirlist_t*)dlent.nxt);
			free(dlent.nxt);
			dlent.nxt = 0;

			continue;
		}

		tmp = (release_t*)ht_get(&mdirs, buf);

		if (!tmp) {
			tmp = (release_t*)malloc(sizeof(release_t));
			tmp->time = dlent.uptime;
			tmp->rel = strdup(buf);

			ht_init(&tmp->subdirs);
			tmp->maindir.di.bytes = -1;

			ht_put_obj(&mdirs, buf, tmp);
		}


		if (!strcmp(gooddir, buf)) {

			if (tmp->maindir.di.bytes == -1) {
				memcpy(&tmp->maindir.di, &dlent, sizeof(dirlog_item_t));
				tmp->maindir.pos = dirlogpos++;
			}
		} else {

			ht_put_obj(&tmp->subdirs, gooddir + strlen(buf) + 1, _dirlog_clone(&dlent, dirlogpos++));
		}

		found = ht_size(&mdirs);

		if (found > show)
			break;
	}

	// populate list.
	for (ht_reset(&mdirs); ht_hasnext(&mdirs); ) {
		tmp = (release_t*)ht_next(&mdirs)->value;

		// skip the last added dir which is maybe incomplete.
		if (tmp->maindir.di.bytes != -1)
			sortedlist_add(rels, tmp);
	}

	return 1;
}
