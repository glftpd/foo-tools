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

#include "webspy_nodeview.h"
#include "webspy.h"

#include <string.h>
#include <lib/macro.h>
#include <lib/who.h>
#include <collection/strlist.h>




char *webspy_nodeview_item(struct ONLINE *o, int pos, time_t now) {
	struct macro_list *ml = 0;
	char tbuf[1024], agebuf[30], *last, tfile[1024], *tmp;
	int rc;
	double speed;

	ml = ml_addint(ml, NODEVIEWITEM_POS, pos);
	ml = ml_addint(ml, NODEVIEWITEM_PID, o->procid);
	ml = ml_addstring(ml, NODEVIEWITEM_USERNAME, o->username);
	
        spy_makeage(o->tstart.tv_sec, now, agebuf);
	ml = ml_addstring(ml, NODEVIEWITEM_IDLE, agebuf);
					  
	sprintf(tbuf, o->status);
	last = (char*)&tbuf;
	while (*last)
		if ((*last == '\r') || (*last == '\n'))
			*last = 0;
		else
			last++;

	rc = who_transfer_direction(o);

	if (rc != TRANS_NONE) {

		who_transfer_file(o, tfile);
		speed = who_transfer_speed(o);
		last = strrchr(tfile, '/');
		if (last)
			last++;
		else
			last = (char*)&tfile;

		sprintf(tbuf, "%s: %s %5dk %.1fks", (rc == TRANS_UP)?"UL":"DL", last, o->bytes_xfer/1024, speed);

	}

	ml = ml_addstring(ml, NODEVIEWITEM_ACTION, tbuf);

	tmp = ml_replacebuf(ml, ht_get(get_config(), PROPERTY_HTML_NODEVIEW_ITEM));

	ml_free(ml);

	return tmp;
}


char *webspy_nodeview(httpd *server) {

	char *tmp, *nodelist;
	int i = 0, max, offset = 1;
	struct ONLINE clone, *tmpn;
	online_t *who;
	strlist_t *nlist = 0;
	time_t now;
	struct macro_list *ml = 0;
	strlist_iterator_t *si;

	who = (online_t*) ht_get(get_context(), ONLINE_ATTR);
	max = who_online_max(who);
	now = time(0);

	for (i = 0; i < max; i++) {

		tmpn = who_getnode(who, i);

		if (!tmpn)
			continue;

		memcpy(&clone, tmpn, sizeof(struct ONLINE));

		if (clone.procid == 0)
			continue;
		
		tmp = webspy_nodeview_item(&clone, offset++, now);

		nlist = str_add_last(nlist, tmp);

		free(tmp);
	}

	// concat list
	max = 0;
	si = str_iterator(nlist);
	while (str_iterator_hasnext(si))
		max += strlen(str_iterator_next(si)) + 1;
	free(si);

	tmp = malloc(max + 1);
	*tmp = 0;
	si = str_iterator(nlist);
	while (str_iterator_hasnext(si))
		strcat(tmp, str_iterator_next(si));

	free(si);
	ml = ml_addstring(ml, NODEVIEW_NODEVIEWITEMS, tmp);

	free(tmp);
	tmp = ml_replacebuf(ml, ht_get(get_config(), PROPERTY_HTML_NODEVIEW));

	ml_free(ml);

	return tmp;
}

