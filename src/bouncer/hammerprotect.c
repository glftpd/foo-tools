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
 * $Id: hammerprotect.c,v 1.2 2003/01/22 14:31:28 sorend Exp $
 */

// std imports
#include <time.h>
#include <stdlib.h>

#include <collection/strlist.h>
#include <collection/hashtable.h>

// project imports
#include "hammerprotect.h"


// count list size.
int _hp_cl_size(_connection_list_t *cl) {
	if (cl != 0)
		return 1 + _hp_cl_size(cl->next);
	else
		return 0;
}

void _hp_cl_close(_connection_list_t *t) {

	if (t) {
		_hp_cl_close(t->next);
		free(t);
	}
}

// create new list without expired items.
_connection_list_t * _hp_cl_remove(_connection_list_t *cl, time_t max_age) {

	_connection_list_t *tmp, *ncl = 0, *tcl;

	for (tmp = cl; tmp; tmp = tmp->next) {

		// printf("DEBUG: tmp time = %d, max_age = %d\n", tmp->time, max_age);

		if (tmp->time >= max_age) {

			tcl = malloc(sizeof(_connection_list_t));
			tcl->time = tmp->time;

			tcl->next = ncl;
			ncl = tcl;
		}
	}

	// free memory used by the old list.
	_hp_cl_close(cl);

	return ncl;
}


// remove expired items from connection_list
void _hp_clean_expired(hashtable_t *ht, time_t max_age) {

	hashtable_item_t *i;
	_connection_info_t *ci;
	_connection_list_t *cl;
	strlist_t *rl = 0;
	strlist_iterator_t *rli;
	char *tip;


	for (ht_reset(ht); ht_hasnext(ht); ) {
		i = ht_next(ht);

		ci = (_connection_info_t*) i->value;

		// remove expired connection times
		ci->connects = _hp_cl_remove(ci->connects, max_age);

		// if there are none left, remove the ip from the table.
		if (_hp_cl_size(ci->connects) < 1)
			rl = str_add(rl, ci->ip);
	}

	// remove ips with no connection-times from hashtable.
	for (rli = str_iterator(rl); str_iterator_hasnext(rli); ) {
		tip = str_iterator_next(rli);

		// printf("DEBUG: expiring %s\n", tip);

		ht_remove(ht, tip);
	}

	str_close(rl);
	free(rli);
}


// implemented from hammerprotect.h
int hp_get_connections_since(hashtable_t *connections, char *ip, int seconds) {

	_connection_info_t *ci;
	_connection_list_t *cl;
	time_t now;
	int found = 0;

	ci = (_connection_info_t*) ht_get(connections, ip);

	if (ci == 0)
		return 0;

	now = time(0);
	now -= seconds;

	for (cl = ci->connects; cl; cl = cl->next) {
		if (cl->time > now)
			found++;
	}

	return found;
}

// implemented from hammerprotect.h
int hp_add_connection(hashtable_t *connections, char *ip, int timeout_seconds) {

	_connection_info_t *ci;
	_connection_list_t *cl;

	ci = (_connection_info_t*) ht_get(connections, ip);

	// create new node.
	if (ci == 0) {
		ci = malloc(sizeof(_connection_info_t));
		strcpy(ci->ip, ip);
		ci->connects = 0;
		ht_put_obj(connections, ip, ci);
	}

	// create new connect attempt.
	cl = malloc(sizeof(_connection_list_t));
	cl->time = time(0);

	// add to list.
	cl->next = ci->connects;
	ci->connects = cl;

	// call expire function to clean list.
	_hp_clean_expired(connections, cl->time - timeout_seconds);
}

// shows state.
void hp_show_state(hashtable_t *ht, int timeout_seconds, int (*prnf)(char *fmt, ...)) {

	time_t now;
	hashtable_item_t *i;
	_connection_info_t *ci;
	_connection_list_t *cl;

	now = time(0);
	now -= timeout_seconds;

	for (ht_reset(ht); ht_hasnext(ht); ) {
		i = ht_next(ht);

		ci = (_connection_info_t*) i->value;

		prnf("info: %s\n", ci->ip);

		for (cl = ci->connects; cl; cl = cl->next)
			prnf("        %d, ttl = %d\n", cl->time, cl->time - now);

		prnf("\n");

	}


}

/**
 * sample main method to test functionality.
 *

int main(int argc, char *argv[]) {

	hashtable_t h;

	ht_init(&h);

	hp_add_connection(&h, "127.0.0.1", 5);

	hp_show_state(&h, 5);

	sleep(3);

	hp_add_connection(&h, "127.0.0.1", 5);

	hp_show_state(&h, 5);

	sleep(3);

	hp_add_connection(&h, "127.0.0.1", 5);

	hp_show_state(&h, 5);

	sleep(7);

	hp_add_connection(&h, "127.0.0.2", 5);

	hp_show_state(&h, 5);

	return 0;
}

 */
