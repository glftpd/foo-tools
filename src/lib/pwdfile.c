

#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <util/linefilereader.h>
#include "stringtokenizer.h"
#include "pwdfile.h"

long pwd_lastupdate = 0;
pwdfile *pwd_userlist = 0;
grpfile_t *pwd_grouplist = 0;

void _pwd_finalize(pwdfile *l) {
	pwdfile *t;

	while (l) {
		t = l;
		l = l->next;
		free(t);
	}
}

pwdfile *_pwd_reload() {
	struct stat stt;
	char buf[500], *t, idbuf[10];
	pwdfile *tmp;
	linefilereader_t lfr;
	stringtokenizer st;
	int i;

	if (stat(PASSWDFILE, &stt) == -1)
		return pwd_userlist;

	if (stt.st_mtime == pwd_lastupdate)
		return pwd_userlist;

#ifdef DEBUG
	printf("Debug -> passwd file loading ..\n");
#endif

	_pwd_finalize(pwd_userlist);

	if (lfr_open(&lfr, PASSWDFILE) < 0)
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
	pwdfile *tmp;

	pwd_userlist = _pwd_reload();

	for (tmp = pwd_userlist; tmp; tmp = tmp->next)
		if (!strcmp(tmp->name, u))
			break;

	return tmp;
}

pwdfile *pwd_getpwuid(int uid) {
	pwdfile *tmp;

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
	grpfile_t *tmp;

	if (pwd_grouplist)
		return pwd_grouplist;

	if (lfr_open(&lfr, GROUPFILE) < 0)
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
	grpfile_t *t;

	for (t = _pwd_grpreload(); t; t = t->next)
		if (!strcmp(t->group, g))
			break;

	return t;
}

grpfile_t *pwd_getgpgid(int gid) {
	grpfile_t *t;

	for (t = _pwd_grpreload(); t; t = t->next)
		if (t->gid = gid)
			break;

	return t;
}
