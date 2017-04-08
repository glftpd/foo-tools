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
 * The standalone cleaner is implemented just as a wrapper to
 * call into the checker framework.
 *
 * $Id: cleaner.c,v 1.2 2003/01/22 14:31:29 sorend Exp $
 */

#include <checker/frame.h>
#include <collection/hashtable.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "mod_nfo.h"

void cleaner_init(hashtable_t *conf, char *file, char *dir) {
	char *buf;
        
	buf = (char*)malloc(strlen(file) + strlen(dir) + 2);
        sprintf(buf, "%s/%s", dir, file);
        
        ht_put(conf, PROPERTY_PATH, buf);
        
        ht_put(conf, PROPERTY_DIR, dir);
        ht_put(conf, PROPERTY_FILE, file);
        
        ht_put(conf, PROPERTY_USERNAME, getenv("USER"));
        ht_put(conf, PROPERTY_GROUP, getenv("GROUP"));
        
        free(buf);
}


int main(int argc, char *argv[]) {
	hashtable_t conf;

	if (argc < 4) {
		printf("syntax; %s <file> <dir> <crc>\n", argv[0]);

		return 1;
	}

	// setup the environment.
	ht_init(&conf);
	ht_load_prop(&conf, CONFIGFILE, ' ');

	cleaner_init(&conf, argv[1], argv[2]);

	// run the check method in nfo module.
	return nfo_check(&conf, argv[1], argv[2], 0);
}
