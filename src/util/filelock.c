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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#include "filelock.h"


FILE *filelock_accuire(char *lockfile) {

	FILE *f;

	f = fopen(lockfile, "a+");

	if (!f)
		return 0;

	// make sure everyone can open it.
	fchmod(fileno(f), 0666);

	if (flock(fileno(f), LOCK_EX) == -1) {
		fclose(f);
		return 0;
	}
		
	return f;
}


void filelock_release(FILE *lockfile) {

	flock(fileno(lockfile), LOCK_UN);
	fclose(lockfile);
}




