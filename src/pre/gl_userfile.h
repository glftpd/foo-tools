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
 * Some functions to manipulate stats in glftpd's userfiles.
 *
 * important note, does not use any kind of locking, since glftpd
 * have not published their locking mechanism !
 *
 * <tanesha@tanesha.net>
 */

#ifndef _gl_userfile_h
#define _gl_userfile_h

// project includes
#include <collection/strlist.h>


struct gl_section_stat {
	int section;

	long files;
	long long kbytes;
	long seconds;

	struct gl_section_stat *next;
};

typedef struct gl_section_stat gl_section_stat_t;

struct gl_stat {
	char *user;
	char *cmd;
	gl_section_stat_t *stats;
};

typedef struct gl_stat gl_stat_t;


/*
 * Gets ratio from a userfile.
 */
int gl_userfile_get_ratio(char *userfile, int section);

/*
 * Updates stats + credits
 */
int gl_userfile_add_stats(char *userfile, int files, long kbytes, int seconds, long credits, int stat_section, int cred_section);

/*
 * explicit set stats (used by reset).
 *
 * types - a list of types to reset stats for. eg. DAYUP DAYDN
 */
int gl_userfile_set_stats(char *userfile, int files, long kbytes, int seconds, strlist_t *types, int (*callback)(gl_stat_t *stat));


#endif
