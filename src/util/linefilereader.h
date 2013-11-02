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
 * Reads a file on line-oriented basis.
 */

struct linefilereader {
	char *file_content;
	long len, pos;
};

typedef struct linefilereader linefilereader_t;

/*
 * Initializes a line-filereader from the given filename (fn).
 *
 * returns 0 on success and -1 on error.
 */
int lfr_open(linefilereader_t *lfr, char *fn);

/*
 * Returns next line in buf upto 'len' of size.
 *
 * Returns length of next line, or 0 on eof/error.
 */
int lfr_getline(linefilereader_t *lfr, char *buf, int len);

/*
 * Closes a lfr context.
 *
 * Always returns 0 (success).
 */
int lfr_close(linefilereader_t *lfr);

