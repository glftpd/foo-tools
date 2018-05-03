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
#include <stdio.h>
#include <stdlib.h>

#include "spy_view.h"
#include "spy.h"
#include "../lib/who.h"

extern spy_status_t status;

void spy_error(char *m) {
	printf("Syntax: foo-spy [path-to-glftpd-conf]\n");
	printf("Error: %s\n", m);
	exit(1);
}

spy_list_t * spy_list_add(spy_list_t *l, struct ONLINE *o, int pos) {
	spy_list_t *tmp, *f;

	tmp = (spy_list_t*)malloc(sizeof(spy_list_t));

	tmp->user = o;
	tmp->pos = pos;
	tmp->prev = 0;
	tmp->next = 0;

	f = l;

	if (!f)
		return tmp;
	else
		while (f->next)
			f = f->next;

	f->next = tmp;
	tmp->prev = f;

	return l;
}

unsigned long spy_gl_find_ipckey(char *fn) {
	FILE *f;
	char buf[1024], *tmp;
	// default glftpd ipckey (i think).
	unsigned long ipckey = 0x0000dead;

	f = fopen(fn, "r");

	if (!f)
		spy_error("Cannot open conf file.");

	while (fgets(buf, 1024, f)) {
		if (!strncasecmp(buf, "ipc_key", 7)) {
	
			tmp = strstr(buf, "0x");

			if (!tmp)
				spy_error("Couldnt parse the ipc_key line in gl conf");

			sscanf(tmp + 2, "%lx", &ipckey);

		}
		if (!strncasecmp(buf, "rootpath", 8)) {
			tmp = strchr(buf, '/');

			if (!tmp)
				spy_error("Couldnt parse rootpath in gl conf");

			strcpy(status.glroot, tmp);
			tmp = status.glroot;

			while (*tmp) {
				if ((*tmp == ' ') || (*tmp == '\r') || (*tmp == '\n'))
					*tmp = 0;
				else
					tmp++;
			}
		}
	}

	fclose(f);

	return ipckey;
}


int main(int argc, char *argv[]) {
	online_t who, *bah;
	int rc, max;
	spy_list_t *userlist = 0;
	unsigned long ipckey;
	char glconf[300];

	void spy_view_handler(spy_list_t *who);
	void spy_view_deinit();

	if (argc > 1)
		strcpy(glconf, argv[1]);
	else
		strcpy(glconf, "/etc/glftpd.conf");

	ipckey = spy_gl_find_ipckey(glconf);

	rc = who_init(&who, ipckey);
	bah = &who;

	if (!rc)
		spy_error("Cannot attach the who shm (wrong ipc-key?)");

	// setup the list of online users.
	max = who_online_max(&who);
	for (rc = 0; rc < max; rc++)
		userlist = spy_list_add(userlist, &bah->online[rc], rc + 1);

	// initialize view
	spy_view_init(max);

	spy_view_handler(userlist);

	spy_view_deinit();
	who_deinit(&who);
}


