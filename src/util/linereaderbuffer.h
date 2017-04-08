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

struct linereaderbuffer {
	char *data;
	int offset;
	int len;
};

typedef struct linereaderbuffer linereaderbuffer_t;

#define LRB_NO_DATA -1
#define LRB_EOF -2

int lrb_initialize(linereaderbuffer_t *lrb);

/*
 * Return 0 on success.
 */
int lrb_add_data(linereaderbuffer_t *lrb, char *data, int len);

int lrb_add_eof(linereaderbuffer_t *lrb);

/*
 * Return -1 on error, otherwise length of new line.
 */
int lrb_getline(linereaderbuffer_t *lrb, char *buf, int len);

int lrb_finalize(linereaderbuffer_t *lrb);

