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

#include <string.h>
#include <util/strmatch.h>

#include "release.h"


// subdir patterns
char SUBDIR_PATTERNS[][30] = {
	"CD?",
	"DISC?",
	"DISC0?",
	"SAMPLE",
	"COVER*",
	"SUBS",
	0
};


int _release_is_release_subdir(char *dir) {
	int i;

	for (i = 0; SUBDIR_PATTERNS[i][0] != 0; i++) {
		if (!strmatch_filename(SUBDIR_PATTERNS[i], dir, STRMATCH_IGNORECASE))
			return 1;
	}

	return 0;
}

int release_get_with_subdir(char *dir, char *releasename, int nlen) {

	char *buf;
	char *t, *t2;

	buf = strdup(dir);

	t=strrchr(buf, '/');

	if (!t) {
		strncpy(releasename, buf, nlen);
		return 0;
	} else
		t++;

	if (_release_is_release_subdir(t)) {

		*(t-1) = 0;
		t2 = strrchr(buf, '/')+1;
		*(t-1) = '/';
		t = t2;
	}

	strncpy(releasename, t, nlen);

	return 1;
}
