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
 * $Id: reset.c,v 1.4 2003/02/12 13:03:11 sorend Exp $
 */

#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>

#include <pre/gl_userfile.h>
#include <lib/gllogs.h>
#include <util/filelock.h>
#include <collection/hashtable.h>
#include <collection/sortedlist.h>

#include "reset.h"
#include "records.h"

#define GLLOG_ERROR "RESETERROR"
#define GLLOG_SUCCESS "RESET"

// note, /tmp should always be writeable for this to work !
#define RESET_LOCKFILE "/tmp/foo-reset.lock"

// parameter object for commandline properties.
struct reset_cmdline {
	int flag_weekly;
	int flag_daily;
	int flag_monthly;
	char *ftpdata_folder;
};

typedef struct reset_cmdline reset_cmdline_t;

// global variables
char *err_str = 0;

hashtable_t collected;
sortedlist_t reset_up;
sortedlist_t reset_down;

void reset_gllog(reset_cmdline_t *cmdline, char *str) {
	char buf[1024];

	sprintf(buf, "%s/logs/glftpd.log", cmdline->ftpdata_folder);

	gl_gllog_add_alt(str, buf);
}


void reset_error_log(reset_cmdline_t *cmdline, char *str) {

	char buf[1024];

	sprintf(buf, "%s: \"%s\"", GLLOG_ERROR, str);

	reset_gllog(cmdline, buf);
}

void reset_stats_spec_log(reset_cmdline_t *cmdline, char *name, char *up, char *dn) {

	char buf[1024];
	stat_sum_t *su, *sd;

	su = (stat_sum_t*) ht_get(&collected, up);
	sd = (stat_sum_t*) ht_get(&collected, dn);

	printf("su(%s) = %d, sd(%s) = %d\n", up, su, dn, sd);

	sprintf(buf, "%s: \"%s\" %.0f %d %d %d %.0f %d %d %d",
			GLLOG_SUCCESS, name,
			su->kbytes, su->files, su->seconds, su->users,
			sd->kbytes, sd->files, sd->seconds, sd->users);

	reset_gllog(cmdline, buf);

}


void reset_stats_log(reset_cmdline_t *cmdline) {


	if (cmdline->flag_weekly)
		reset_stats_spec_log(cmdline, "WEEK", "WKUP", "WKDN");

	if (cmdline->flag_daily)
		reset_stats_spec_log(cmdline, "DAY", "DAYUP", "DAYDN");

	if (cmdline->flag_monthly)
		reset_stats_spec_log(cmdline, "MONTH", "MONTHUP", "MONTHDN");

}


int reset_get_cmdline(reset_cmdline_t *cmdline, int argc, char *argv[]) {

	int i, found = 0;

	bzero(cmdline, sizeof(reset_cmdline_t));

	for (i = 1; i < argc; i++) {

		if (!strcmp(argv[i], "-w"))
			cmdline->flag_weekly = 1;
		else if (!strcmp(argv[i], "-d"))
			cmdline->flag_daily = 1;
		else if (!strcmp(argv[i], "-m"))
			cmdline->flag_monthly = 1;
		else {

			if (cmdline->ftpdata_folder != 0) {
				err_str = "You can only specify one ftp-data folder!";
				return -1;
			}
			else {
				cmdline->ftpdata_folder = argv[i];
			}

		}
	}

	if ((cmdline->flag_daily || cmdline->flag_weekly || cmdline->flag_monthly) &&
		cmdline->ftpdata_folder)
		return 0;

	err_str = "Missing parameters";
	return -1;
}

strlist_t * _reset_build_types(reset_cmdline_t *cmdline) {
	strlist_t *l = 0;

	if (cmdline->flag_daily) {
		l = str_add(l, "DAYUP");
		l = str_add(l, "DAYDN");
	}

	if (cmdline->flag_weekly) {
		l = str_add(l, "WKUP");
		l = str_add(l, "WKDN");
	}

	if (cmdline->flag_monthly) {
		l = str_add(l, "MONTHUP");
		l = str_add(l, "MONTHDN");
	}

	return l;
}


// callback method for collecting stats on gl_userfile_reset_stat ..
int _reset_collect_stats(gl_stat_t *stat) {

	int hs = 0;
	stat_sum_t *sum = 0;
	gl_section_stat_t *ss;

	// update the sums
	sum = (stat_sum_t*) ht_get(&collected, stat->cmd);

	if (!sum) {
		sum = malloc(sizeof(stat_sum_t));

		sum->files = 0;
		sum->kbytes = 0;
		sum->seconds = 0;
		sum->users = 0;

		ht_put_obj(&collected, stat->cmd, sum);
	}

	// add stats for all sections.
	for (ss = stat->stats; ss; ss = ss->next) {
		sum->files += ss->files;
		sum->kbytes += ss->kbytes;
		sum->seconds += ss->seconds;

		if (ss->files)
			hs = 1;
	}

	if (hs)
		sum->users++;

	return 0;
	
}


int reset_execute(reset_cmdline_t *cmdline) {

	DIR *dh;
	struct dirent *dent;
	char buf[1024], userd[1024];
	strlist_t *types;

	sprintf(userd, "%s/users", cmdline->ftpdata_folder);
	dh = opendir(userd);

	if (!dh) {
		reset_error_log(cmdline, "Could not open userfile-folder");
		return -1;
	}

	ht_init(&collected);
	types = _reset_build_types(cmdline);

	printf("Reading dir ..\n");

	while (dent = readdir(dh)) {

		// skip . files.
		if (dent->d_name[0] == '.')
			continue;

		// skip default.*
		if (!strncmp(dent->d_name, "default.", 8))
			continue;

		sprintf(buf, "%s/%s", userd, dent->d_name);

		printf("setting stats: %s\n", buf);

		gl_userfile_set_stats(buf, 0, 0, 0, types, _reset_collect_stats);
	}

	closedir(dh);

	reset_stats_log(cmdline);
}


int main(int argc, char *argv[]) {

	int rc;
	reset_cmdline_t cmdline;
	FILE *lock;

	rc = reset_get_cmdline(&cmdline, argc, argv);

	if (rc == -1) {
		printf("syntax: %s [options] <ftpdata-folder>\nError: %s\n", argv[0], err_str);

		return 1;
	}

	// accuire lock.
	lock = filelock_accuire(RESET_LOCKFILE);

	if (!lock) {
		reset_error_log(&cmdline, "Could not accuire lock file !");
		return 1;
	}

	rc = reset_execute(&cmdline);

	// release lock
	filelock_release(lock);
}

