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
//
// $Id: usage.c,v 1.2 2003/01/22 14:31:30 sorend Exp $
//
#include <string.h>

#include <lib/who.h>
#include <lib/common.h>
#include <collection/strlist.h>

#include "usage.h"


#define FU 0
#define FD 1
#define SU 2
#define SD 3
#define TU 4
#define TD 5

int got_hidden(strlist_t *l, char *p) {

	strlist_iterator_t *i;
	char *tmp;
	int found = 0;

	for (i = str_iterator(l); str_iterator_hasnext(i); ) {
		tmp = str_iterator_next(i);

		if (!strncasecmp(p, tmp, strlen(tmp)))
			found++;
	}
	free(i);

	return found;
}

void do_usage(char *user, online_t *who) {
	int max, rc, i, idle = 0, ul = 0, dl = 0, trans;
	struct ONLINE *tmp, n;
	char users[10][50], owntransfers[1024], buf[500], filebuf[1024], *tbuf, perbuf[30];
	double speeds[10], speed;
	int bw;
	strlist_t *hidden;

	hidden = str_load(0, HIDDENDIRS);
	owntransfers[0] = 0;
	max = who_online_max(who);

	for (i = 0; i < 6; i++) {
		users[i][0] = 0;
		speeds[i] = 0;
	}

	for (i = 0; i < max; i++) {
		tmp = who_getnode(who, i);

		if (!tmp)
			continue;

		// clone
		memcpy(&n, tmp, sizeof(n));

		// no user online
		if (n.procid <= 0)
			continue;

		// user is in hidden dir.
		if (got_hidden(hidden, n.currentdir))
			continue;

		// check if its a transfer.
		trans = who_transfer_direction(&n);

		if (trans == TRANS_NONE) {
			idle++;

		} else {

			speed = who_transfer_speed(&n);

			if (trans == TRANS_UP) {

				if (speeds[FU] < speed) {
					strcpy(users[FU], n.username);
					speeds[FU] = speed;
				}
				if ((speeds[SU] == 0) || (speeds[SU] > speed)) {
					strcpy(users[SU], n.username);
					speeds[SU] = speed;
				}

				speeds[TU] += speed;
				ul++;
			} else {

				if (speed > speeds[FD]) {
					strcpy(users[FD], n.username);
					speeds[FD] = speed;
				}
				if ((speeds[SD] == 0) || (speed < speeds[SD])) {
					strcpy(users[SD], n.username);
					speeds[SD] = speed;
				}

				speeds[TD] += speed;
				dl++;
			}

			// check if this transfer is for the user we're looking at.
			if (user && !strcmp(n.username, user)) {
				who_transfer_file(&n, filebuf);

				tbuf = strrchr(filebuf, '/');

				if (!tbuf)
					tbuf = filebuf;
				else
					tbuf++;

				sprintf(buf, " %s/%s/%.1fk", (trans==TRANS_UP?"ul":"dl"),
						tbuf, speed);

				strcat(owntransfers, buf);
			}
		}

	}

	if (ul) {
		common_make_percent((int)speeds[TU], UPBW, 10, '-', 'x', perbuf);
		printf("incoming [%s] %d at %.1fKb/s (%s/%.0fk to %s/%.0fk)\n",
			   perbuf, ul, speeds[TU],
			   users[FU], speeds[FU],
			   users[SU], speeds[SU]);
	}

	if (dl) {
		common_make_percent((int)speeds[TD], DOWNBW, 10, '-', 'x', perbuf);
		printf("outgoing [%s] %d at %.1fKb/s (%s/%.0fk to %s/%.0fk)\n",
			   perbuf, dl, speeds[TD],
			   users[FD], speeds[FD],
			   users[SD], speeds[SD]);
	}

	common_make_percent(idle+ul+dl, MAXNORMAL, 10, '-', 'x', perbuf);
	printf("total .. [%s] %d/%d logins, %d busy at %.1fKb/s, %d idle\n",
		   perbuf, idle+ul+dl, MAXNORMAL, ul+dl, speeds[TU]+speeds[TD], idle);

	if (user && (owntransfers[0] != 0))
		printf("%-8.8s%s\n", user, owntransfers);
}




int main(int argc, char *argv[]) {

	online_t who;
	int rc;


	rc = who_init(&who, IPCKEY);

	if (!rc) {
		printf(EMPTY);
		exit(0);
	}

	do_usage(argc > 1 ? argv[1] : 0, &who);

	who_deinit(&who);

	return 0;
}
