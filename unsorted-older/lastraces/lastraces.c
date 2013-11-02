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
 * command to find latest races a user has participated in, <sorend@tanesha.net>
 *
 * $Id: lastraces.c,v 1.2 2003/01/22 14:31:30 sorend Exp $
 */

#include <stdio.h>
#include <string.h>
#include <lib/pwdfile.h>
#include <collection/strlist.h>
#include <collection/sortedlist.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <regex.h>

// this structure defines the 'default' dirs to look in.
struct dir {
	char dir[300];

} dirs[] = {
	"/site/incoming/games",
	"/site/incoming/utils",
	"/site/incoming/xxx",
	"/site/incoming/vcd",
	0
};

typedef struct dir dir_t;

// pattern for which dirs to skip.
#define SKIPREGEX "(narQd|NUKED|COMPLETE|INCOMPLETE)"

// root of files in gl (usually /site)
#define ROOTDIR "/site"


// NO NEED TO EDIT BELOW.


struct lastrace {
	char *dir;

	int mkdir;
	int files;
	long bytes;
	long age;

	strlist_t *put;

	struct lastrace *next;
};

typedef struct lastrace lastrace_t;

void makeage(long t, char *buf) {
	long age = time(0) - t;
	long days, hours, mins, secs;
	
	days  =  age / (3600 * 24);
	age   -= days * 3600 * 24;
	hours =  age / 3600;
	age   -= hours * 3600;
	mins  =  age / 60;
	secs  =  age - (mins * 60);
	
	if (days)
		sprintf(buf, "%dd %dh", days, hours);
	else if (hours)
		sprintf(buf, "%dh %dm", hours, mins);
	else
		sprintf(buf, "%dm %ds", mins, secs);
}


lastrace_t * races_find_dir(lastrace_t *lr, char *dir, pwdfile *user) {
	DIR *dh;
	struct dirent *dent;
	struct stat st;
	char *buf;
	lastrace_t *tmp = 0;
	long dir_age;
	regex_t re;
	int rc;

	rc = regcomp(&re, SKIPREGEX, REG_NOSUB | REG_ICASE | REG_EXTENDED);

	if (rc != 0) {
		printf("Bad regexp for skipping.. \n");
		exit(1);
	}

	rc = regexec(&re, dir, 0, 0, 0);

	if (rc == 0)
		return lr;

	dh = opendir(dir);

	if (!dh)
		return lr;

	// stat dir to check if user made it.
	stat(dir, &st);
	dir_age = st.st_mtime;

	if (st.st_uid == user->uid) {
		tmp = (lastrace_t*)malloc(sizeof(lastrace_t));

		tmp->age = dir_age;
		tmp->dir = strdup(dir);
		tmp->bytes = 0;
		tmp->files = 0;
		tmp->put = 0;
		tmp->mkdir = 1;
		tmp->next = lr;
		lr = tmp;
	}


	while (dent = readdir(dh)) {

		if (!strncmp(dent->d_name, ".", 1))
			continue;

		buf = (char*)malloc(strlen(dent->d_name) + strlen(dir) + 2);
		sprintf(buf, "%s/%s", dir, dent->d_name);

		if (stat(buf, &st) == -1) {
			free(buf);
			continue;
		}

		if (S_ISDIR(st.st_mode))
			lr = races_find_dir(lr, buf, user);
		else if (S_ISREG(st.st_mode) && (st.st_uid == user->uid)) {
			if (!tmp) {
				tmp = (lastrace_t*)malloc(sizeof(lastrace_t));
				tmp->files = 0;
				tmp->bytes = 0;
				tmp->age = dir_age;
				tmp->put = 0;
				tmp->mkdir = 0;
				tmp->dir = strdup(dir);
				tmp->next = lr;
				lr = tmp;
			}

			tmp->files++;
			tmp->bytes += st.st_size;
			tmp->put = str_add(tmp->put, dent->d_name);
		}

		free(buf);
	}

	closedir(dh);

	return lr;
}

int races_sort_by_age_rev(void *p, void *q) {
	lastrace_t *a, *b;

	a = (lastrace_t*)p;
	b = (lastrace_t*)q;

	return (a->age < b->age);
}


int find_races(pwdfile *pwd, char *dir) {
	int i = 0, p;
	strlist_t *tstr;
	lastrace_t *races = 0, *tmp;
	char buf[300];
	sortedlist_t sorted;

	if (dir) {
		printf("Searching for %s's races (in %s) pls hold .. \n\n", pwd->name, dir);
		races = races_find_dir(races, dir, pwd);
	} else {
		printf("Searching for %s's races (in all dirs), pls hold .. \n\n", pwd->name);
		while (dirs[i].dir[0] != 0)
			races = races_find_dir(races, dirs[i++].dir, pwd);
	}

	sortedlist_init(&sorted);

	for (tmp = races; tmp; tmp = tmp->next)
		sortedlist_add(&sorted, tmp);

	sortedlist_sort(&sorted, races_sort_by_age_rev);
	sortedlist_reset(&sorted);

	while (sortedlist_hasnext(&sorted)) {
		tmp = (lastrace_t*)sortedlist_next(&sorted);

		printf("-------------------------------------------------------------------------\n");

		if (!strncmp(tmp->dir, ROOTDIR, strlen(ROOTDIR)))
			printf("Dir  : %s\n", tmp->dir + strlen(ROOTDIR));
		else
			printf("Dir  : %s\n", tmp->dir);

		makeage(tmp->age, buf);

		printf("Flags: [Mkd:%s][Files:%d][Size:%.1fMb][Age:%s]\n",
			   (tmp->mkdir?"Yes":"No"), tmp->files,
			   (float)tmp->bytes/(1024*1024), buf);

		printf("Files: ");

		if (tmp->put) {
			i = 0;
			for (tstr = tmp->put; tstr; tstr = tstr->next) {
				printf("[%s]", tstr->data);

				i += strlen(tstr->data);
				if (tstr->next && (strlen(tstr->next->data) + i) > 55) {
					i = 0;
					printf("\n     : ");
				}
			}
			printf("\n");
		} else
			printf(" None, only made the dir!\n");

		if (tmp->dir)
			free(tmp->dir);

		str_close(tmp->put);

		free(tmp);

	}


	return 1;
}


int main(int argc, char *argv[]) {
	pwdfile *pwd;
	char dir[300];

	if (argc < 2) {
		printf("Syntax; SITE RACES <user> [dir]\n");

		return 0;
	}

	pwd = pwd_getpwnam(argv[1]);

	if (!pwd) {
		printf("User %s not found.\n");
		return 0;
	}

	if (argc > 2) {
		if (argv[2][0] == '/')
			sprintf(dir, "%s%s", ROOTDIR, argv[2]);
		else
			strcpy(dir, argv[2]);

		find_races(pwd, dir);
	} else
		find_races(pwd, 0);

	return 0;
}
