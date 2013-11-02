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
 * $Id: hammerprotect.h,v 1.2 2003/01/22 14:31:29 sorend Exp $
 */

#ifndef _hammerprotect_h
#define _hammerprotect_h

#include <collection/hashtable.h>

// internal structure for connection lists.
struct _connection_list {
	time_t time;

	struct _connection_list *next;
};

typedef struct _connection_list _connection_list_t;

// internal structure for ip <-> connects mapping.
struct _connection_info {

	char ip[30];

	_connection_list_t *connects;
};

typedef struct _connection_info _connection_info_t;


/*
 * Returns number of connections from ip 'ip' in 'since' seconds.
 */
int hp_get_connections_since(hashtable_t *connections, char *ip, int seconds);

/*
 * Adds a connection from 'ip', and remove expired connections older than
 * 'timeout_seconds'.
 */
int hp_add_connection(hashtable_t *connections, char *ip, int timeout_seconds);


#endif
