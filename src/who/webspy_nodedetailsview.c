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


#include "webspy_nodedetailsview.h"
#include "webspy_dirview.h"
#include "webspy.h"

#include <string.h>
#include <lib/macro.h>
#include <lib/who.h>
#include <collection/strlist.h>
#include <collection/hashtable.h>
#include <lib/pwdfile.h>


char * webspy_nodedetailsview(httpd *server) {

	httpVar *pid;
	int ipos, max, i;
	online_t *who;
	struct ONLINE clone, *tmp;
	struct macro_list *ml = 0;
	char agebuf[100], *out;
	time_t now;
	pwdfile *pw;
	hashtable_item_t *dirid;

	now = time(0);

	who = (online_t*) ht_get(get_context(), ONLINE_ATTR);

	pid = httpdGetVariableByName(server, "pid");

	if (!pid || !pid->value)
		return strdup(NODEDETAILSVIEW_NOSELECTED);

	// we have a pid, add it to refresh vars.
	spy_add_refresh_var("pid");

	ipos = atoi(pid->value);

	max = who_online_max(who);
	for (i = 0; i < max; i++) {
		tmp = who_getnode(who, i);

		if (tmp && (tmp->procid == ipos))
			break;
	}

	if (!tmp)
		return strdup(NODEDETAILSVIEW_NOSELECTED);

	memcpy(&clone, tmp, sizeof(struct ONLINE));

	if (clone.procid != ipos)
		return strdup(NODEDETAILSVIEW_NOSELECTED);

	ml = ml_addstring(ml, NODEDETAILSVIEW_CWD, clone.currentdir);
	ml = ml_addstring(ml, NODEDETAILSVIEW_TAGLINE, clone.tagline);
	ml = ml_addstring(ml, NODEDETAILSVIEW_USERNAME, clone.username);
	ml = ml_addstring(ml, NODEDETAILSVIEW_HOST, clone.host);
	ml = ml_addint(ml, NODEDETAILSVIEW_PID, clone.procid);
	ml = ml_addint(ml, NODEDETAILSVIEW_GID, (int)clone.groupid);

	dirid = webspy_dirview_get_direntry(clone.currentdir);

	ml = ml_addstring(ml, NODEDETAILSVIEW_DIRID, dirid->key);

	spy_makeage(clone.login_time, now, agebuf);
	ml = ml_addstring(ml, NODEDETAILSVIEW_LOGIN, agebuf);

	out = ml_replacebuf(ml, ht_get(get_config(), PROPERTY_HTML_NODEDETAILSVIEW));

	ml_free(ml);

	return out;
}

