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
 * $Id: antidualleech.c,v 1.2 2003/01/22 14:31:30 sorend Exp $
 */

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>

#include <stdio.h>
#include <lib/who.h>
#include <lib/gllogs.h>
#include <collection/strlist.h>

// no kicking in this dir.
#define PRIVATE "/site/private"

// min pid to ick.
#define MINPID 30

extern int errno;

struct allow_user_t {
	char user[30];
} allow_users[] = {

	// users who are allowed to dual leech.
	"BigLeecher",
	"FatLeech",

	// end of structure.
	0
};


struct allow_ext_t {
	char ext[10];
} allow_exts[] = {

	// extentions that are allowed to be dual-leeched.
	"*.nfo",
	"*.mpg",

	// end of structure.
	0
};

void quit(online_t *w, int c) {

	// detach from the shared memory.
	who_deinit(w);

	exit(c);
}

int is_allowed_ext(char *buf) {
	int i;

	for (i = 0; allow_exts[i].ext[0] != 0; i++)
		if (!fnmatch(allow_exts[i].ext, buf, 0))
			return 1;

	return 0;
}

int is_allow_user(char *user) {
	int i;

	for (i = 0; allow_users[i].user[0] != 0; i++)
		if (!strcmp(allow_users[i].user, user))
			return 1;

	return 0;
}

int get_dl_file(char *cmdline, char *file) {
	char buf[1024], *tmp;

	strncpy(buf, cmdline, 1024);

	for (tmp = buf; *tmp; tmp++)
		if (*tmp == '\r' || *tmp == '\n')
			*tmp = 0;

	if (strncasecmp(buf, "RETR ", 5))
		return 0;

	strcpy(file, buf + 5);

	return 1;
}


int main(int argc, char *argv[]) {
	int nodes = 0, direction = 0, found = 0, i, j, ppid;
	struct ONLINE *o, tempo;
	char *user, buf[300], *tmp, fbuf[300], dlfile[1024];
	online_t who;
	int rc, pid, privlen;

	privlen = strlen(PRIVATE);
	user = getenv("USER");
	ppid = getppid();


	// check if its an allowed user.
	if (is_allow_user(user))
		return 0;

	// check if we are in private.
	getcwd(buf, 300);

	// debug, printf("150Cwd = %s, privlen = %d\r\n", buf, privlen);

	// skip private dirs.
	if (!strncmp(buf, PRIVATE, privlen))
		return 0;

	// skip allowed extentions.
	rc = get_dl_file(argv[1], dlfile);
	if (rc == 0) {
		printf("150Anti-dualleech failed to initialize ;(\r\n");
		return 1;
	}

	// debug, printf("150Dlfile = %s\r\n", dlfile);

	if (is_allowed_ext(dlfile))
		return 0;

	rc = who_init(&who, IPCKEY);

	if (rc == 0) {
		printf("150Anti-dualleech failed to initialize ;(\r\n");
		return 1;
	}

	nodes = who_online_max(&who);

	// nodeloop.
	for (i = 0; i < nodes; i++) {

		o = who_getnode(&who, i);

		// skip if nobody online.
		if (!o)
 			continue;

		memcpy(&tempo, o, sizeof(struct ONLINE));

		// skip current node.
		if (tempo.procid == ppid)
			continue;

		if (tempo.procid < MINPID)
			continue;

		// skip if node is in private dir.
		if (!strncmp(tempo.currentdir, PRIVATE, strlen(PRIVATE)))
			continue;

		direction = who_transfer_direction(&tempo);

		if ((direction == TRANS_DOWN) && (!strcmp(tempo.username, user)) ) {

			rc = who_transfer_file(&tempo, fbuf);
			if (!rc)
				continue;

			// check if its an allowed file.
			if (is_allowed_ext(fbuf))
				continue;

			tmp = strrchr(fbuf, '/');

			if (!tmp)
				tmp = fbuf;
			else
				tmp = tmp+1;

			fprintf(stdout, "550Dual leech detected. Kicking other session: %d/%s.\r\n", tempo.procid, (tmp?tmp:"UNKNOWN"));

			fflush(stdout);

			// new log entry: KICK: "DUAL" "sorend" <otherpid> "<otherfile>" <thispid> "<thisfile>".
			sprintf(buf, "\"DUAL\" \"%s\" %d \"%s\" %d \"%s\"",
					tempo.username,
					tempo.procid, fbuf,
					ppid, "UNKNOWN");

			// put to log.
			gl_gllog_announce("KICK", buf);

			rc = kill(tempo.procid, SIGTERM);

			if (rc == -1)
				printf("550Error killing other session: %d\r\n", errno);

			found = 1;
		}
	}

	quit(&who, found);
}

