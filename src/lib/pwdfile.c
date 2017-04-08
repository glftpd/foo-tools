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


#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <util/linefilereader.h>
#include <stdio.h>
#include "stringtokenizer.h"
#include "pwdfile.h"

long pwd_lastupdate = 0;
pwdfile *pwd_userlist = 0;
grpfile_t *pwd_grouplist = 0;
char *_etcdir = 0;

void _pwd_finalize(pwdfile *l) {
	pwdfile *t;

	while (l) {
		t = l;
		l = l->next;
		free(t);
	}
}

void pwd_set_etcdir(char *etcdir) {
	if (_etcdir != 0)
		free(_etcdir);

	_etcdir = strdup(etcdir);
}

char *_pwd_get_etcdir() {
	if (_etcdir == 0)
		_etcdir = strdup(DEFAULT_ETCDIR);

	return _etcdir;
}

pwdfile *_pwd_reload() {
	struct stat stt;
	char buf[500], *t, idbuf[10];
	pwdfile *tmp;
	linefilereader_t lfr;
	stringtokenizer st;
	int i;

	sprintf(buf, "%s/%s", _pwd_get_etcdir(), DEFAULT_PASSWD);

	if (stat(buf, &stt) == -1)
		return pwd_userlist;

	if (stt.st_mtime == pwd_lastupdate)
		return pwd_userlist;

#ifdef DEBUG
	printf("Debug -> passwd file loading ..\n");
#endif

	_pwd_finalize(pwd_userlist);

	if (lfr_open(&lfr, buf) < 0)
		return pwd_userlist;

	while (lfr_getline(&lfr, buf, 300) > -1) {
		
		st_initialize(&st, buf, ":");

		if (st_count(&st) != 7) {
			st_finalize(&st);
			continue;
		}

		tmp = (pwdfile*)malloc(sizeof(pwdfile));

		strcpy(tmp->name, st_next(&st));
		strcpy(tmp->pass, st_next(&st));

		strcpy(idbuf, st_next(&st));
		tmp->uid = atoi(idbuf);

		strcpy(idbuf, st_next(&st));
		tmp->gid = atoi(idbuf);

		strcpy(tmp->longname, st_next(&st));
		strcpy(tmp->homedir, st_next(&st));
		strcpy(tmp->shell, st_next(&st));

		tmp->next = pwd_userlist;
		pwd_userlist = tmp;
	}

	lfr_close(&lfr);

	pwd_lastupdate = stt.st_mtime;

	return pwd_userlist;
}

pwdfile *pwd_getpwnam(char *u) {
	pwdfile *tmp = 0;

	pwd_userlist = _pwd_reload();

	for (tmp = pwd_userlist; tmp; tmp = tmp->next)
		if (!strcmp(tmp->name, u))
			break;

	return tmp;
}

pwdfile *pwd_getpwuid(int uid) {
	pwdfile *tmp = 0;

	pwd_userlist = _pwd_reload();

	for (tmp = pwd_userlist; tmp; tmp = tmp->next)
		if (tmp->uid == uid)
			break;

	return tmp;
}

grpfile_t *_pwd_grpreload() {
	linefilereader_t lfr;
	char buf[300], gidbuf[30];
	stringtokenizer st;
	grpfile_t *tmp = 0;

	if (pwd_grouplist)
		return pwd_grouplist;

	sprintf(buf, "%s/%s", _pwd_get_etcdir(), DEFAULT_GROUP);

	if (lfr_open(&lfr, buf) < 0)
		return 0;

	while (lfr_getline(&lfr, buf, 300) > 0) {
		st_initialize(&st, buf, ":");

		if (st_count(&st) < 3) {
			st_finalize(&st);
			continue;
		}

		tmp = (grpfile_t*)malloc(sizeof(grpfile_t));

		strcpy(tmp->group, st_next(&st));
		strcpy(tmp->pass, st_next(&st));
		strcpy(gidbuf, st_next(&st));

		if (st_hasnext(&st))
			tmp->users = strdup(st_next(&st));
		else
			tmp->users = 0;

		tmp->gid = atoi(gidbuf);

		tmp->next = pwd_grouplist;

		pwd_grouplist = tmp;

		st_finalize(&st);
	}
	lfr_close(&lfr);

	return pwd_grouplist;
}

grpfile_t *pwd_getgpnam(char *g) {
	grpfile_t *t = 0;

	for (t = _pwd_grpreload(); t; t = t->next)
		if (!strcmp(t->group, g))
			break;

	return t;
}

grpfile_t *pwd_getgpgid(int gid) {
	grpfile_t *t;

	for (t = _pwd_grpreload(); t; t = t->next)
		if (t->gid == gid)
			break;

	return t;
}
