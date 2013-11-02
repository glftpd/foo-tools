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


#include <stdio.h>
#include <sys/stat.h>

#include "dirlog.h"



int dirlog_init(dirlog_t *dl, char *file) {
	struct stat st;

	dl->f = fopen(file, "r");

	if (!dl->f)
		return 0;

	if (fstat(fileno(dl->f), &st) != 0) {
		fclose(dl->f);
		return 0;
	}

	dl->age = st.st_mtime;

	if (fseek(dl->f, 0, SEEK_END) != 0) {
		fclose(dl->f);
		return 0;
	}

	dl->size = ftell(dl->f) / sizeof(dirlog_item_t);

	dl->first = dl->last = -1;
	dl->cache = 0;

	return 1;
}

int dirlog_finalize(dirlog_t *dl) {
	if (dl->f != 0)
		fclose(dl->f);

	if (dl->cache)
		free(dl->cache);

	dl->size = dl->first = dl->last = -1;

	return 1;
}

int _dirlog_update_cache(dirlog_t *dl, long entry) {

	long red, med = DIRLOG_CACHESIZE / 2;
	long i;

	if (dl->cache)
		free(dl->cache);

	dl->cache = 0;

	// malloc cache
	dl->cache = (dirlog_item_t*)malloc(sizeof(dirlog_item_t) * DIRLOG_CACHESIZE);

	if (dl->size < DIRLOG_CACHESIZE) {
		// dirlog is small, we can have all in the cache.
		dl->first = 0;
		dl->last = dl->size - 1;
	}
	else if (dl->size < (entry + med)) {
		// we want to get an entry which is at the end of the file.
		dl->first = dl->size - DIRLOG_CACHESIZE;
		dl->last = dl->size - 1;
	}
	else if ((entry - med) < 0) {
		dl->first = 0;
		dl->last = DIRLOG_CACHESIZE - 1;
	}
	else {
		dl->first = entry - med;
		dl->last = entry + med - 1;
	}

	red = dl->last - dl->first + 1;

	if (fseek(dl->f, dl->first * sizeof(dirlog_item_t), SEEK_SET) != 0)
		printf("ERROR SEEKING IN DIRLOG!\n");

	if (fread(&dl->cache[0], sizeof(dirlog_item_t), red, dl->f) != red)
		printf("ERROR READING FROM DIRLOG\n");

	return 1;
}


int dirlog_getentry(dirlog_t *dl, long entry, dirlog_item_t *item) {
	dirlog_item_t *tmp;
	long offset;

	if (entry < 0 || entry > (dl->size - 1))
		return 0;

	if (dl->first == -1 || dl->last == -1)
		_dirlog_update_cache(dl, entry);

	if (entry < dl->first || entry > dl->last)
		_dirlog_update_cache(dl, entry);

	offset = (entry - dl->first);

	// clone the item we want to get.
	tmp = &dl->cache[offset];

	memcpy(item, tmp, sizeof(dirlog_item_t));

	return 1;
}

long dirlog_getsize(dirlog_t *dl) {
	return dl->size;
}

time_t dirlog_getage(dirlog_t *dl) {
	return dl->age;
}
