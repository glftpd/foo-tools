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


#include <lib/who.h>
#include <collection/hashtable.h>
#include <util/date.h>
#include "fwho.h"
#include <lib/dirlist.h>
#include <lib/stringtokenizer.h>



hashtable_t *_cfg = 0, *_ctx = 0;


hashtable_t * get_config() {
	if (_cfg == 0) {
		_cfg = malloc(sizeof(hashtable_t));
		ht_init(_cfg);
	}
	return _cfg;
}

hashtable_t * get_context() {
	if (_ctx == 0) {
		_ctx = malloc(sizeof(hashtable_t));
		ht_init(_ctx);
	}
	return _ctx;
}

unsigned long fwho_get_ipckey() {
	char *tmp;
	hashtable_t *cfg = get_config();
	unsigned long ipckey = 0;

	tmp = ht_get(PROPERTY_WHO_IPCKEY);

	if (!tmp) {
		printf("Could not get property '%s' from config.\n", PROPERTY_WHO_IPCKEY);
		return 0;
	}

	// skip 0x if exists.
	if (!strncasecmp(tmp, "0x", 2))
		tmp += 2;

	sscanf(tmp, "%lx", &ipckey);

	return ipckey;
}


dirlist_t * fwho_get_dirlist(char *p) {
	hashtable_t *ht, *dls;
	dirlist_t *dl;

	ht = get_context();
	dls = (hashtable_t*)ht_get(ht, PROPERTY_WHO_DIRLISTS);
	if (dls == 0) {
		dls = malloc(sizeof(hashtable_t));
		ht_put_obj(ht, PROPERTY_WHO_DIRLISTS, dls);
	}

	dl = (dirlist_t*)ht_get(dls, p);
	if (dl == 0) {
		dl = malloc(sizeof(dirlist_t));
		if (dirlist_init(dl, p))
			dirlist_readdir(dl);

		ht_put_obj(dls, p, dl);
	}

	return dl;
}

int fwho_get_text(char *cmd, int isuser, int ishidden) {
	char *upcmd, *tmp;
	hashtable_t *cfg = get_config();

	upcmd = strdup(cmd);
	tmp = upcmd;

	tmp = cmd;
	while (*tmp) {
		*tmp = toupper(*tmp);
		tmp++;
	}

	// build the key for text lookup.
	if (isuser)
		sprintf(buf, "who_text_%s_user", upcmd);
	else if (ishidden)
		sprintf(buf, "who_text_%s_hidden", upcmd);
	else
		sprintf(buf, "who_text_%s", upcmd);

	text = ht_get(cfg, buf);

	if (text || !strcmp(upcmd, "DEFAULT")) {
		free(upcmd);
		return text;
	} else {
		free(upcmd);
		return fwho_get_text("DEFAULT", ppid, ishidden);
	}
}

struct macro_list * fwho_prepare_macros(struct ONLINE *o, int num, double speed) {
	struct macro_list *t = 0;
	hashtable_t *ctx = get_context();
	char buf[1024];

	t = ml_addfloat(t, "speed", speed);
	t = ml_addstring(t, "user", o->username);
	t = ml_addstring(t, "tagline", o->tagline);
	t = ml_addstring(t, "status", o->status);
	t = ml_addstring(t, "currentdir", o->currentdir);
	t = ml_addstring(t, "group", buf);
	t = ml_addstring(t, "transbytes", buf);

	return t;
}


int fwho_view_single(struct ONLINE *o, int ppid, int num) {
	dirlist_t *dl;
	char *dir, *cmdline, *cmd, buf[300];
	double speed;
	stringtokenizer st;
	int ishidden = 0;
	struct macro_list *ml;

	dl = fwho_get_dirlist(o->currentdir);

	speed = who_transfer_speed(o);
	if (speed < 0)
		speed = 0;

	cmdline = strdup(o->status);
	tmp = cmdline;
	while (*tmp)
		if (*tmp=='\r'||*tmp=='\n')
			*tmp = 0;
		else
			tmp++;

	st_initialize(&st, tmp, " ");

	if (st_count(&st) < 1)
		cmd = "DEFAULT";
	else
		cmd = st_next(&st);

	text = fwho_get_text(cmd, (ppid == o->procid), ishidden, speed);

	if (!text)
		return 0;

	ml = fwho_prepare_macros(o, num);

	outbuf = ml_replacebuf(ml, text);

	printf(outbuf);

	free(outbuf);
	ml_free(ml);
}


int fwho_view(online_t *who) {
	hashtable_t * ht = get_context();
	int rc, max, i, j = 1, ppid;
	struct ONLINE *ou;

	ppid = getppid();

	max = who_online_max(who);
	for (i = 0; i < max; i++) {
		ou = who_getnode(who, i);

		if (ou == 0)
			continue;

		fwho_view_prepare_single(ou, ppid, j++);
	}


}


int fwho_exec(int argc, char *argv[]) {
	online_t who;
	int rc;
	unsigned long ipckey;

	ipckey = fwho_get_ipckey();
	if (!ipckey)
		return 0;

	rc = who_init(&who, ipckey);

	if (!rc) {
		printf("Could not attach glftpd who shared memory, wrong ipckey in config?\n");
		return 0;
	}

	fwho_view(&who);

	who_deinit(&who);

	return 0;
}



void fwho_init(int argc, char *argv[]) {
	hashtable_t *cfg = get_config();

	ht_load_prop(cfg, WHO_CONFIGFILE, '=');

	ht_put(cfg, "user", getenv("USER"));
	ht_put(cfg, "group", getenv("GROUP"));
}


int main(int argc, char *argv[]) {

	fwho_init(argc, argv);

	return fwho_exec(argc, argv);
}
