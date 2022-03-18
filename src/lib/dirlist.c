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
 * lib for handling releases in dirs, implmenentation of
 * dirlist.h interface.  /sorend
 */

#include <sys/file.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <fnmatch.h>
#include "dirlist.h"

#define DIRLIST_VERSION 1

int dirlist_gettmpracefile(dirlist_t *d, char *rf, int n) {
	char *tmp;

	strncpy(rf, d->dir, n);

	rf[n - 1] = 0;

	tmp = rf;

	while (*tmp) {
		if (*tmp == '/')
			*tmp = '_';
		else if (*tmp == '.')
			*tmp = '_';

		tmp++;
	}

	return 1;
}

int dirlist_init(dirlist_t *d, char *dir) {

	d->list = 0;
	d->dir = strdup(dir);
	d->raceinfo = 0;

	return 1;
}

int dirlist_readdir(dirlist_t *d) {
    dirlist_item_t *tmp;
    DIR *dh;
    struct dirent *dent;
    char *buf;
    int dirlen;
    struct stat st;

    dirlen = strlen(d->dir);

    dh = opendir(d->dir);

    if (!dh)
		return 0;

    while (dent = readdir(dh)) {
		buf = (char*)malloc(strlen(dent->d_name) + dirlen + 2);

		sprintf(buf, "%s/%s", d->dir, dent->d_name);

		if (stat(buf, &st) == -1) {
			free(buf);

			continue;
		} else
			free(buf);
		
		tmp = (dirlist_item_t *)malloc(sizeof(dirlist_item_t ));
		
		tmp->downloads = st.st_gid % 100;
		st.st_gid = st.st_gid - (st.st_gid % 100);
		tmp->raceinfo = 0;
		tmp->file = strdup(dent->d_name);
		memcpy(&tmp->st, &st, sizeof(st));
		
		tmp->next = d->list;
		d->list = tmp;
    }
	
    closedir(dh);
	
    return 1;
}

/*
 * Finds complete files in the dir 'p'.
 */
strlist_t * dirlist_get_perms(dirlist_t *d, int perms) {
    strlist_t *l = 0;
    dirlist_item_t *tmp;

    for (tmp = d->list; tmp; tmp = tmp->next)
		if (tmp->st.st_mode & perms)
			l = str_add(l, tmp->file);

    return l;
}

/*
 * Finds rarsize of files in the dir 'p'.
 */
long dirlist_get_filesize(dirlist_t *d) {
    dirlist_item_t *tmp;
    long largest = 0;

    for (tmp = d->list; tmp; tmp = tmp->next)
	if (tmp->st.st_size > largest)
	    largest = tmp->st.st_size;

    return largest;
}

/*
 * Finds sfv in dirlist.
 */
int dirlist_get_sfv(dirlist_t *d, char *sfv) {
    dirlist_item_t *tmp;

    for (tmp = d->list; tmp; tmp = tmp->next)
	if (fnmatch(DL_MASK_SFV, tmp->file, 0) == 0)
	    break;

    if (!tmp)
		return 0;

    strcpy(sfv, tmp->file);

    return 1;
}

/*
 * Finds files in a dir.
 */
strlist_t * dirlist_get_pattern(dirlist_t *d, char *p) {
    dirlist_item_t *tmp;
    strlist_t *l = 0;

    for (tmp = d->list; tmp; tmp = tmp->next)
		if (fnmatch(p, tmp->file, 0) == 0)
			l = str_add(l, tmp->file);

    return l;
}

dirlist_item_t * dirlist_get_file(dirlist_t *d, char *f) {
    dirlist_item_t *tmp;

    for (tmp = d->list; tmp; tmp = tmp->next)
		if (!strcmp(f, tmp->file))
			break;

    return tmp;
}

FILE * dirlist_raceinfo_openw(dirlist_t *d, char *f) {

	d->racefile_lock = fopen(f, "a+");

	if (!d->racefile_lock)
		return 0;

	if (flock(fileno(d->racefile_lock), LOCK_EX) == -1) {
		fclose(d->racefile_lock);

		d->racefile_lock = 0;
	}

	return d->racefile_lock;
}

void dirlist_raceinfo_close(dirlist_t *d) {
	flock(fileno(d->racefile_lock), LOCK_UN);

	fclose(d->racefile_lock);
}


int dirlist_raceinfo_update_header(dirlist_t *d, unsigned long t) {
	race_header_t hdr;
	int ret = 0;

	// scan to beginning of file.
	fseek(d->racefile_lock, 0, SEEK_SET);

	if (fread(&hdr, 1, sizeof(hdr), d->racefile_lock) != sizeof(hdr)) {
		hdr.start = 0;
		hdr.end = 0;
		hdr.version = DIRLIST_VERSION;
	}

	if (hdr.start == 0)
		hdr.start = t;
	if (hdr.end == 0)
		hdr.end = t;

	if (t > hdr.end)
		hdr.end = t;

	fseek(d->racefile_lock, 0, SEEK_SET);

	if (fwrite(&hdr, 1, sizeof(hdr), d->racefile_lock) != sizeof(hdr))
		return 0;
	else
		return 1;
}


int dirlist_raceinfo_add(dirlist_t *d, char *f, int uid, long s, unsigned long t, int op) {
	race_file_t rf;

	strncpy(rf.file, f, MAX_FILELEN);
	rf.speed = s;
	rf.time = t;
	rf.op = op;
	rf.uid = uid;

	// update header + position file after header.
	dirlist_raceinfo_update_header(d, t);

	fseek(d->racefile_lock, 0, SEEK_END);

	if (fwrite(&rf, 1, sizeof(rf), d->racefile_lock) == sizeof(rf))
		return 1;
	else
		return 0;
}

/*
 * uid = -1, op = RACE_INIT -> find who started to upload file first.
 * uid = 10, op = RACE_INIT -> find when uid=10 started to upload file.
 *
 * uid = 10, op = -1 -> find first action user had on that file.
 *
 */
race_file_t * dirlist_raceinfo_find(dirlist_item_t *i, int uid, int op) {
	race_file_list_t *l;

	for (l = i->raceinfo; l; l = l->next) {
		if ((uid != -1) && (l->info.op == op))
			break;

		if ((op != -1) && (l->info.uid == uid))
			break;

		if ((l->info.op == op) && (l->info.uid == uid))
			break;
	}

	if (l)
		return &l->info;

	return 0;
}

int dirlist_raceinfo_load(dirlist_t *d, char *ef) {
	char *tmp;
	FILE *f;
	race_file_t buf;
	race_file_list_t *rlist;
	dirlist_item_t *i;

	tmp = (char*)malloc(strlen(ef) + strlen(d->dir) + 2);
	sprintf(tmp, "%s/%s", d->dir, ef);

	f = fopen(tmp, "r");
	if (!f)
		return 0;

	// read header.
	d->raceinfo = (race_header_t *)malloc(sizeof(race_header_t ));
	if (fread(d->raceinfo, sizeof(race_header_t ), 1, f) != sizeof(race_header_t )) {
		fclose(f);

		return 0;
	}

	// read file entries.
	while (fread(&buf, sizeof(buf), 1, f) == sizeof(buf)) {
		// find the file where we want to attach this file to.
		for (i = d->list; i; i = i->next)
			if (!strcmp(i->file, buf.file) && (i->st.st_uid == buf.uid))
				break;

		// put the item onto the list.
		if (i) {
			rlist = (race_file_list_t *)malloc(sizeof(race_file_list_t ));

			memcpy(&rlist->info, &buf, sizeof(race_file_t ));

			rlist->next = i->raceinfo;
			i->raceinfo = rlist;
		}
	}

	fclose(f);

	return 1;
}


void dirlist_closedir(dirlist_t *d) {
    dirlist_item_t *tmp;
	race_file_list_t *rl;

    while (d->list) {
		tmp = d->list;

		d->list = d->list->next;

		if (tmp->file)
			free(tmp->file);

		while (tmp->raceinfo) {
			rl = tmp->raceinfo;

			tmp->raceinfo = tmp->raceinfo->next;

			free(rl);
		}
    }

    if (d->dir)
		free(d->dir);

	if (d->raceinfo)
		free(d->raceinfo);
}

void dirlist_reset(dirlist_t *d) {
	d->cur = d->list;
}

int dirlist_hasnext(dirlist_t *d) {
	if (d->cur)
		return 1;

	return 0;
}

dirlist_item_t * dirlist_next(dirlist_t *d) {
	dirlist_item_t *t;

	t = d->cur;

	if (t)
		d->cur = t->next;

	return t;
}
