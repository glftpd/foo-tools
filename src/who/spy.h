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



#ifndef _spy_h
#define _spy_h

#include "../lib/who.h"

struct spy_list {
	int pos;
	struct ONLINE *user;

	struct spy_list *next, *prev;
};

typedef struct spy_list spy_list_t;

struct spy_status {
	int ul_num, dl_num;

	double ul_speed, dl_speed;
	int online, max;

	int max_ul_num, max_dl_num;
	double max_ul_speed, max_dl_speed;

	int mode;
	char glroot[300];
};

typedef struct spy_status spy_status_t;

#define MODE_USERLIST 1
#define MODE_DIRLIST 2

#endif
