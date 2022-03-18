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

#include "webspy_dirview.h"
#include "webspy.h"

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <lib/macro.h>
#include <lib/who.h>
#include <collection/strlist.h>
#include <collection/hashtable.h>
#include <lib/pwdfile.h>


unsigned int _next_dirid = 1;

void _webspy_dirview_getcwd(char *broken) {

	char *sitedir, buf[1024], *tmp;
	struct stat st;

	sitedir = ht_get(get_config(), PROPERTY_GLFTPD_SITEDIR);

	sprintf(buf, "%s/%s", sitedir, broken);

	stat(buf, &st);

	if (!S_ISDIR(st.st_mode)) {
		tmp = strrchr(buf, '/');

		if (!tmp)
			return;

		*tmp = 0;
	}	

}

hashtable_item_t *webspy_dirview_get_direntry(char *dir) {
	hashtable_t *dis;
	hashtable_item_t *hti;
	char dirbuf[1024], idbuf[30];

	// make clone of dir and fix it up.
	strcpy(dirbuf, dir);
	_webspy_dirview_getcwd(dirbuf);

	dis = (hashtable_t*) ht_get(get_context(), DIRVIEW_DIRMAP_ATTR);

        hashtable_item_t *_ht_find_item(hashtable_item_t *t, char *k);

	// create if not existent.
	if (!dis) {
		dis = malloc(sizeof(hashtable_t));
		ht_init(dis);
		ht_put_obj(get_context(), DIRVIEW_DIRMAP_ATTR, dis);
	}

	ht_reset(dis);
	while (ht_hasnext(dis)) {

		hti = ht_next(dis);

		if (!strcmp(hti->value, dirbuf))
			return hti;
	}

	// not found, so put new one.
	sprintf(idbuf, "%d", _next_dirid++);
	ht_put(dis, idbuf, dirbuf);

	// HACK! call into hashtable internals, fixme in future
	hti = (hashtable_item_t*)_ht_find_item(dis->list, idbuf);

	return hti;
}




char *webspy_dirview_item(char *path, struct dirent *dent) {

	pwdfile *pw;
	grpfile_t *grp;
	char *ret, buf[1024];
	struct stat st;
	int gid, downloads;
	struct macro_list *ml = 0;

	sprintf(buf, "%s/%s", path, dent->d_name);

	stat(buf, &st);

	downloads = st.st_gid % 100;
	gid = st.st_gid - downloads;

	grp = pwd_getgpgid(gid);
	pw = pwd_getpwuid(st.st_uid);

	ml = ml_addint(ml, DIRVIEW_DOWNLOADS, downloads);
	ml = ml_addfloat(ml, DIRVIEW_SIZE, (float)st.st_size);

	ml = ml_addstring(ml, DIRVIEW_DATE, "1969");

	ml = ml_addstring(ml, DIRVIEW_OWNER, (pw?pw->name:"unknown"));
	ml = ml_addstring(ml, DIRVIEW_GROUP, (grp?grp->group:"unknown"));

	ml = ml_addstring(ml, DIRVIEW_FILENAME, dent->d_name);

	ret = ml_replacebuf(ml, ht_get(get_config(), PROPERTY_HTML_DIRVIEW_ITEM));

	ml_free(ml);

	return ret;
}

char *webspy_dirview(httpd *server) {

	httpVar *dir;
	DIR *dh;
	struct dirent *dent;
	char *sitedir, buf[1024], *mapped_dir, *tmpitem;
	struct macro_list *ml = 0;
	hashtable_t *dirmap;
	strlist_t *items = 0;

	dir = httpdGetVariableByName(server, DIRVIEW_DIR_ATTR);

	if (!dir || !dir->value)
		return strdup(DIRVIEW_NOSELECTED);

	dirmap = (hashtable_t*) ht_get(get_context(), DIRVIEW_DIRMAP_ATTR);
	mapped_dir = ht_get(dirmap, dir->value);

	if (!mapped_dir)
		return strdup(DIRVIEW_NOSELECTED);

	// we have a pid, add it to refresh vars.
	spy_add_refresh_var(DIRVIEW_DIR_ATTR);

	sitedir = ht_get(get_config(), PROPERTY_GLFTPD_SITEDIR);

	if (!sitedir)
		return strdup("Configuration error of dirview module!");

	sprintf(buf, "%s%s", sitedir, mapped_dir);

	printf("dir = %s\n", buf);
	dh = opendir(buf);

	if (!dh)
		return strdup("Could not open selected dir!");

	while (dent = readdir(dh)) {

		printf("item .. 1\n");
		tmpitem = webspy_dirview_item(buf, dent);

		printf("item .. 2\n");
		items = str_add(items, tmpitem);

		printf("item .. 3\n");
		free(tmpitem);
	}

	closedir(dh);

	printf("item .. 4\n");
	
	tmpitem = str_join(items, "");

	ml = ml_addstring(ml, DIRVIEW_DIRVIEWITEMS, tmpitem);

	printf("item .. 5\n");

	free(tmpitem);

	ml = ml_addstring(ml, DIRVIEW_DIR, mapped_dir);

	printf("item .. 6\n");

	tmpitem = ml_replacebuf(ml, ht_get(get_config(), PROPERTY_HTML_DIRVIEW));

	printf("item .. 7\n");
	
	ml_free(ml);

	return tmpitem;

}
