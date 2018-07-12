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
 *  foo.Pre [C-version]  (c)  tanesha team, <tanesha@tanesha.net>
 slv 20170414 - mp3 genre added to PRE output (instead of in mod_idmp3)
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <fnmatch.h>
#include <stdarg.h>
#include <dlfcn.h>

#include <lib/gllogs.h>
#include <lib/pwdfile.h>
#include <collection/strlist.h>
#include <lib/stringtokenizer.h>
#include <collection/hashtable.h>
#include <util/linefilereader.h>

#include "foo-pre.h"
#include "gl_userfile.h"

#define VERSION "$Id: foo-pre.c,v 1.20 2018/07/12 07:33:10 sorend, slv Exp $"
#define USAGE " * Syntax: SITE PRE <RELEASEDIR> [SECTION]\n"

void quit(char *s, ...);
extern int errno;

hashtable_t *_config = 0;
hashtable_t *_envctx = 0;

char mp3_genre[40] = "Unknown";

/*
 * Acessor method for configuration.
 */
hashtable_t * get_config() {
	if (!_config) {
		_config = malloc(sizeof(hashtable_t));
		bzero(_config, sizeof(hashtable_t));
	}

	return _config;
}

/*
 * Accessor method for context of the pre.
 */
hashtable_t * get_context() {
	if (!_envctx) {
		_envctx = malloc(sizeof(hashtable_t));
		ht_init(_envctx);
	}

	return _envctx;
}

/*
 * Puts something to the logfile.
 */
void pre_log(char *type, char *fmt, ...) {
	hashtable_t *env = get_context();
	hashtable_t *cfg;
	va_list va;
	FILE *f;

	char fdate[12], ftime[10];
	time_t now;
	struct tm *tm_now;

	now = time(0);
	tm_now = localtime(&now);

	strftime(fdate, 1024, "%Y-%m-%d", tm_now);
	strftime(ftime, 1024, "%H:%M:%S", tm_now);

	f = (FILE*)ht_get(env, "logfh");

	if (!f) {
		f = fopen(PRE_LOGFILE, "a");
		ht_put_obj(env, "logfh", f);
	}
	// slv: added date/time
	//fprintf(f, "%s: ", type);
	fprintf(f, "%s %s %s: ", fdate, ftime, type);
	va_start(va, fmt);
	vfprintf(f, fmt, va);
	va_end(va);
	fprintf(f, "\n");
	fflush(f);
}

/*
 * Returns a given property for a group.
 */
char *group_get_property(char *grp, char *prop) {
	hashtable_t *cfg = get_config();
	char buf[300], *tmp;

	sprintf(buf, "group.%s.%s", grp, prop);

	tmp = ht_get(cfg, buf);

	if (!tmp) {
		// try to get default property.
		sprintf(buf, "group.DEFAULT.%s", prop);
		tmp = ht_get(cfg, buf);
	}

	return tmp;
}


/*
 * Returns a property for a section.
 */
char * section_get_property(char *sec, char *prop) {
	hashtable_t *cfg = get_config();
	char buf[300];

	sprintf(buf, "section.%s.%s", sec, prop);

	return ht_get(cfg, buf);
}

/*
 * Find chown info for a group.
 */
chowninfo_t * chowninfo_find_by_group(char *group) {
	char *tmp;
	hashtable_t *env = get_context();
	chowninfo_t * i = malloc(sizeof(chowninfo_t));
	pwdfile *pass;
	grpfile_t *grp;

	i->uid = i->gid = -1;

	tmp = group_get_property(group, PROPERTY_GROUP_CHOWN_USER);
	if (tmp) {
		if (!strcmp(tmp, "USER"))
			pass = pwd_getpwnam(ht_get(env, PROPERTY_USER));
		else
			pass = pwd_getpwnam(tmp);

		if (pass)
			i->uid = pass->uid;
		else
			printf(" * WARNING: Could not get user.chown info for %s\n", group);
	}

	tmp = group_get_property(group, PROPERTY_GROUP_CHOWN_GROUP);
	if (tmp) {
		if (!strcmp(tmp, "GROUP"))
			grp = pwd_getgpnam(group);
		else
			grp = pwd_getgpnam(tmp);

		if (grp)
			i->gid = grp->gid;
		else
			printf(" * WARNING: Could not get grp.chown info for %s\n", group);
	}

	return i;
}

creditlist_t *creditlist_find_by_uid(creditlist_t *l, int uid) {
	creditlist_t *t;

	for (t = l; t; t = t->next)
		if (t->uid == uid)
			break;

	return t;
}

struct subdir_list *subdirlist_find(struct subdir_list *l, char *s) {
	struct subdir_list *t;

	for (t = l; t; t = t->next)
		if (!strcmp(t->dir, s))
			break;

	return t;
}

int subdirlist_count(struct subdir_list *l) {
	struct subdir_list *t;
	int i = 0;

	for (t = l; t; t = t->next)
		if (!strncmp(t->dir, "CD", 2) || !strncmp(t->dir, "DISC", 4))
			i++;

	return i;
}

/*
 * Returns a strlist_t with the groups of an user.
 */
strlist_t * user_find_groups(char *u) {
	linefilereader_t lfr;
	char buf[1024], *tmp, *spc2;
	strlist_t *groups = 0;
	hashtable_t *cfg = get_config();

	tmp = ht_get(cfg, PROPERTY_GL_USERDIR);

	if (!tmp)
		quit("  * Could not find '%s' property in config", PROPERTY_GL_USERDIR);

	sprintf(buf, "%s/%s", tmp, u);

	if (lfr_open(&lfr, buf) < 0)
		quit("  * Could not open your userfile %s", buf);

	while (lfr_getline(&lfr, buf, 1024) > -1) {
		if (!strncasecmp(buf, "GROUP ", 6) ||
				!strncasecmp(buf, "PRIVATE ", 8)) {

			tmp = strchr(buf, ' ') + 1;

			// fix for glftpd2.x which has 'GROUP group num'
			spc2 = strchr(tmp, ' ');
			if (spc2)
				*spc2 = 0;

			groups = str_add(groups, tmp);
		}
	}

	lfr_close(&lfr);

	return groups;
}

/*
 * Returns the list of pre groups from config.
 *
 */
strlist_t * groups_find_all() {
	strlist_t * l = 0;
	char *tmp, buf[300], *grp;
	hashtable_t *cfg = get_config();
	hashtable_t *env = get_context();
	hashtable_item_t *htn;

	// check if we have made it already.
	if (ht_get(env, "ALLGROUPS"))
		return (strlist_t*)ht_get(env, "ALLGROUPS");

	ht_reset(cfg);

	// look through all properties in config.
	while (htn = ht_next(cfg)) {
		tmp = htn->key;

		if (strcmp(tmp, "group."))
			continue;

		grp = strdup(tmp + 6);

		tmp = strchr(grp, '.');

		if (!tmp) {
			free(grp);
			continue;
		}

		*tmp = 0;

		if (!str_search(l, grp, 0)) {
			l = str_add(l, grp);
		}
	}

	ht_put_obj(env, "ALLGROUPS", l);

	return l;
}


/*
 * Gets configuration properties in the a|b|c|d format as a strlist.
 */
strlist_t * config_get_split_property(char *prop) {
	hashtable_t *cfg = get_config();
	char *tmp;
	stringtokenizer st;
	strlist_t *l = 0;

	tmp = ht_get(cfg, prop);

	st_initialize(&st, tmp, "|");
	while (st_hasnext(&st)) {
		tmp = st_next(&st);

		l = str_add(l, tmp);
	}

	return l;
}


/*
 * Returns the group of the dir 'path'.
 */
char * group_find_by_dir(strlist_t *grps, char *path) {
	strlist_iterator_t *iter;
	char *tmpgroup, *tmpdir, buf[300];
	strlist_t *grpdirs = 0;
	int found = 0;

	// only look through the groups of the user.
	for (iter = str_iterator(grps); str_iterator_hasnext(iter); ) {
		tmpgroup = str_iterator_next(iter);

		// get groupdirs as a list.
		sprintf(buf, "group.%s.%s", tmpgroup, PROPERTY_GROUP_DIR);
		grpdirs = config_get_split_property(buf);

		if (grpdirs && str_search(grpdirs, path, 0)) {
			free(iter);
			str_close(grpdirs);
			return strdup(tmpgroup);
		}
	}

	str_close(grpdirs);
	free(iter);

	return 0;
}

/*
 *
 * Outputs info about a user's groups and the groupdirs.
 *
 */
int show_groupdirs(strlist_t *grps) {
	strlist_iterator_t *i;
	char *tmpgroup, *tmppredir, *tmpallowed, *sitedir;
	hashtable_t *cfg = get_config();
	stringtokenizer dirst, allowst;
	char buf[300];
	int found = 0;

	sitedir = ht_get(cfg, PROPERTY_SITEDIR);
	if (!sitedir)
		printf(" * WARNING: '%s' not defined in config file\n", PROPERTY_SITEDIR);

	printf("Group           Predirs                                  Allowed sections\n");
	printf("--------------- ---------------------------------------- ----------------\n");

	for (i = str_iterator(grps); str_iterator_hasnext(i); ) {
		tmpgroup = str_iterator_next(i);

		tmppredir = group_get_property(tmpgroup, PROPERTY_GROUP_DIR);
		tmpallowed = group_get_property(tmpgroup, PROPERTY_GROUP_ALLOW);

		if (!(tmppredir && tmpallowed))
			continue;

		found++;

		st_initialize(&dirst, tmppredir, "|");
		st_initialize(&allowst, tmpallowed, "|");
		strcpy(buf, tmpgroup);

		while (st_hasnext(&dirst) || st_hasnext(&allowst)) {
			tmppredir = st_next(&dirst);
			tmpallowed = st_next(&allowst);

			printf("%-15.15s %-40.40s %-15.15s\n", buf, (tmppredir?tmppredir + strlen(sitedir):""), (tmpallowed?tmpallowed:""));

			strcpy(buf, "");
		}

		st_finalize(&dirst);
		st_finalize(&allowst);
	}

	if (!found)
		printf("Hm, you arent in any groups that can pre ! \n");

	return found;
}

/*
 * Returns section name of a 'requested' section, if its allowed
 * for 'group'.
 */
char * section_find_by_name(char *group, char *requested) {
	char *tmpsecdir, *tmpallow, *tmpreq;
	stringtokenizer st;

	tmpreq = requested;

	// if no requested, then try to set it to the 'default' section.
	if (!tmpreq) {
		tmpreq = group_get_property(group, PROPERTY_GROUP_DEFSEC);

		if (!tmpreq) {
			printf(" * Hm, section not specified, and no default section for group '%s'\n", group);
			printf("%s\n", USAGE);

			return 0;
		}
	}

	st_initialize(&st, group_get_property(group, PROPERTY_GROUP_ALLOW), "|");

	while (st_hasnext(&st)) {
		tmpallow = strdup(st_next(&st));

		if (!strcasecmp(tmpreq, tmpallow))
			break;

		free(tmpallow);
		tmpallow = 0;
	}

	st_finalize(&st);

	if (!tmpallow)
		return 0;

	// do basic check if section is in the config.
	tmpsecdir = section_get_property(tmpallow, PROPERTY_SECTION_DIR);

	if (!tmpsecdir) {
		printf(" * Hm, you are allowed to pre in section '%s' but the section is not.\n   configured. Bug siteop to fix his pre configuration please! :)\n", tmpreq);

		return 0;
	}

	return tmpallow;
}

/*
 * Returns an integer property for a section, returns -1 if not set or
 * set to 'none'.
 */
int section_get_int_property(char *s, char *p) {
	char *tmp;

	tmp = section_get_property(s, p);

	if (!tmp)
		return -1;

	if (!strcasecmp(tmp, "none"))
		return -1;

	return atoi(tmp);
}

/*
 * Takes care of updating userfile, by help of gl_userfile module.
 */
char * userfile_update(creditlist_t *l) {
	hashtable_t *cfg = get_config();
	hashtable_t *env = get_context();
	long spend, addkb;
	char buf[1024], userfile[1024], *section, *userdir, *tmp, *group;
	pwdfile *pass;
	int gl_stat_section = 0, gl_credit_section = 0, prespeed, rc, ratio;

	// get pre-section from env.
	section = ht_get(env, "section");
	group = ht_get(env, PROPERTY_GROUP);

	// find the user.
	pass = pwd_getpwuid(l->uid);
	if (!pass)
		return "";

	// get ftp-data/users from conf.
	userdir = ht_get(cfg, PROPERTY_GL_USERDIR);
	if (!userdir)
		quit(" * ERROR: Userdir not defined in pre config, fuck the sysop ;(\n");

	sprintf(userfile, "%s/%s", userdir, pass->name);

	// get the section config from conf.
	gl_stat_section = section_get_int_property(section, PROPERTY_GROUP_GL_SS);
	gl_credit_section = section_get_int_property(section, PROPERTY_GROUP_GL_CS);

	if ((gl_stat_section == -1) && (gl_credit_section == -1)) {
		printf(" * ERROR: No gl_stat_section or gl_credit_section defined for\n");
		printf("          section %s, %s got no stats/credits for pre !\n", section,
				pass->name);

		return "";
	}

	// try and figure a ratio to use.
	tmp = group_get_property(group, PROPERTY_GROUP_RATIO);

	if (!tmp) {
		printf(" * ERROR: No ratio defined for group %s, no credits given!\n", group);
		return "";
	}

	if (!strcasecmp(tmp, "USERFILE")) {
		ratio = gl_userfile_get_ratio(userfile, gl_credit_section);

		if (ratio == -1) {
			printf(" * ERROR: No ratio found in userfile for %s\n", pass->name);

			return "";
		}
	} else
		ratio = atoi(tmp);

	if (prespeed == 0)
		prespeed = 1024;

	spend = (long)l->bytes / prespeed;
	addkb = l->bytes;

	if (spend == 0)
		spend++;

	printf("   -- %10.10s: ", pass->name);

	if (gl_stat_section > -1)
		printf("%d file/s, %.1f Mb (s:%d)", l->files, (float)addkb/1024, gl_stat_section);
	else
		printf("No stats given");

	if (gl_credit_section > -1)
		printf(", Credits: %.1f Mb, Ratio: %d (s:%d)\n", (float)(ratio * addkb)/1024, ratio, gl_credit_section);
	else
		printf(", No credits given\n");

	// call gl_userfile_* to get the userfile updated.
	rc = gl_userfile_add_stats(userfile, l->files, addkb, spend, (long)ratio * addkb, gl_stat_section, gl_credit_section);

	if (rc > -1) {
		sprintf(buf, "%s/%.1fMb, ", pass->name, (float)l->bytes/1024);
		return strdup(buf);
	} else
		printf(" * ERROR: Couldnt update userfile for %s\n", pass->name);

	return "";
}

/*
 * Chowns a file from chowninfo.
 */
int chowninfo_apply_to_file(char *file, chowninfo_t *ch) {

	if (!ch)
		return 0;

	// check if no chown needed.
	if ((ch->gid == -1) && (ch->uid == -1))
		return 1;

	chown(file, ch->uid, ch->gid);

	return 1;
}

/*
 * Loads a filelist from a dir.
 *
 * Also chowns all files in the dir if chown is != 0.
 */
filelist_t * filelist_find_by_dir(filelist_t *l, char *base, char *path, chowninfo_t *chown) {
	DIR *dh;
	char buf[1024], *group;
	struct dirent *dent;
	struct stat st;
	filelist_t *tmp;

	group = ht_get(get_context(), PROPERTY_GROUP);

	sprintf(buf, "%s/%s", base, path);
	dh = opendir(buf);

	if (!dh)
		return l;

	while ((dent = readdir(dh))) {
		if ((!strcmp(dent->d_name, ".")) ||
				(!strcmp(dent->d_name, "..")))
			continue;

		sprintf(buf, "%s/%s/%s", base, path, dent->d_name);

		if (stat(buf, &st) == -1)
			continue;

		// chown file
		chowninfo_apply_to_file(buf, chown);

		if (S_ISDIR(st.st_mode)) {

			// touch dir
			touch_dir(buf);

			if (strlen(path) > 0)
				sprintf(buf, "%s/%s", path, dent->d_name);
			else
				strcpy(buf, dent->d_name);

			// find recursively.
			l = filelist_find_by_dir(l, base, buf, chown);

		} else {
			// touch file
			touch_file(buf);

			tmp = (filelist_t*)malloc(sizeof(filelist_t));

			if (strlen(path) > 0)
				sprintf(tmp->file, "%s/%s", path, dent->d_name);
			else
				strcpy(tmp->file, dent->d_name);

			memcpy(&tmp->st, &st, sizeof(st));

			tmp->next = l;

			l = tmp;
		}
	}

	closedir(dh);

	return l;
}


int strlist_match(char *file, strlist_t *l) {
	strlist_iterator_t *iter;
	int found = 0;
	char *pattern;

	iter = str_iterator(l);

	while (pattern = str_iterator_next(iter)) {
		if (fnmatch(pattern, file, 0) == 0)
			found++;
	}

	free(iter);

	return found;
}

/*
 * Method to find total size of filelist.
 */
int flist_size(filelist_t *filelist, float *bytes, long *files) {
	filelist_t *t;
	strlist_t *tmp;
	strlist_iterator_t *iter;
	char *tt;
	int found;

	// get countables.
	tmp = config_get_split_property(PROPERTY_COUNTABLE);

	*bytes = 0; *files = 0;

	for (t = filelist; t; t = t->next) {

		if (strlist_match(t->file, tmp) == 0)
			continue;

		(*bytes) += t->st.st_size;
		(*files)++;
	}

	return 1;
}

char *flist_getfilename(filelist_t *l) {
	char *buf;

	buf = strrchr(l->file, '/');
	if (!buf)
		buf = l->file;
	else
		buf++;

	return buf;
}

/*
 * Create list of subdirs from filelist.
 *
 */
struct subdir_list *subdir_flistcollect(filelist_t *f) {
	struct subdir_list *l = 0, *tmp, *mdir;
	filelist_t *t;
	char *d, buf[1024];

	mdir = (struct subdir_list*)malloc(sizeof(struct subdir_list));
	mdir->bytes = 0;
	mdir->files = 0;
	strcpy(mdir->dir, "");
	mdir->next = 0;
	l = mdir;

	for (t = f; t; t = t->next) {
		strcpy(buf, t->file);
		d = strrchr(buf, '/');
		if (!d)
			strcpy(buf, "");
		else
			*d = 0;

		if ((tmp = subdirlist_find(l, buf)) == 0) {
			tmp = (struct subdir_list*)malloc(sizeof(struct subdir_list));
			tmp->bytes = 0;
			tmp->files = 0;
			strcpy(tmp->dir, buf);
			tmp->next = l;
			l = tmp;
		}

		tmp->bytes += t->st.st_size;
		tmp->files++;

		// update the main dir (if it aint us).
		if (strcmp(buf, "")) {
			mdir->bytes += t->st.st_size;
			mdir->files++;
		}
	}

	return l;
}

creditlist_t *creditlist_create_from_filelist(filelist_t *f) {
	creditlist_t *l = 0, *t;
	filelist_t *tfl;
	pwdfile *pass;
	strlist_t *creditables;

	creditables = config_get_split_property(PROPERTY_CREDITABLE);

	for (tfl = f; tfl; tfl = tfl->next) {

		if (strlist_match(tfl->file, creditables) == 0)
			continue;

		t = creditlist_find_by_uid(l, tfl->st.st_uid);

		if (!t) {
			t = (creditlist_t*)malloc(sizeof(creditlist_t));
			t->uid = tfl->st.st_uid;
			t->files = 0;
			t->bytes = 0;
			t->next = l;
			l = t;
		}

		t->files += 1;
		t->bytes += (long)tfl->st.st_size/1024;
	}

	return l;
}

void reverse_dirlog_add(struct subdir_list *tsub, char *dest, char *rel, pwdfile *pass, chowninfo_t *chown) {
	char buf[300], *tmp;
	hashtable_t *cfg = get_config();
	int uid, gid, addsub;

	tmp = ht_get(cfg, PROPERTY_ADDSUB);

	if (tmp)
		addsub = atoi(tmp);
	else
		addsub = 0;

	if (tsub)
		reverse_dirlog_add(tsub->next, dest, rel, pass, chown);
	else
		return;

	if (!addsub && (strlen(tsub->dir) > 0))
		return;

	if (strlen(tsub->dir) > 0)
		sprintf(buf, "%s/%s", dest, tsub->dir);
	else
		strcpy(buf, dest);

	uid = (chown->uid == -1) ? pass->uid : chown->uid;
	gid = (chown->gid == -1) ? pass->gid : chown->gid;

	if (!gl_dirlog_add(buf, uid, gid, tsub->files, tsub->bytes))
		printf(" * Error adding to dirlog: %s/%s (%dF %.1fMb)\n",
				rel, tsub->dir,
				tsub->files, (float)tsub->bytes/(1024*1024));
}

/*
 * Method to replace in strings.
 */
int pre_replace(char *b, char *n, char *r) {
	char *t, *save;
	int i=0;

	while (t=strstr(b, n)) {
		save=(char*)malloc(strlen(t)-strlen(n)+1);
		strcpy(save, t+strlen(n));
		*t=0;
		strcat(b, r);
		strcat(b, save);
		free(save);
		i++;
	}
}

int pre_move_catalog(char *src, char *dest) {
	char *tmp, movebuf[1024];
	int rc;
	hashtable_t *cfg = get_config();
	int mv_force_ext;

	tmp = ht_get(cfg, PROPERTY_MOVE_FORCE_EXT);

	if (tmp)
		mv_force_ext = atoi(tmp);
	else
		mv_force_ext = 0;

	tmp = ht_get(get_config(), PROPERTY_MOVE_EXTERNAL);

        //only rename if 'force ext mv' option is not set in cfg
        if (!mv_force_ext) {
		// try rename.
		rc = rename(src, dest);

		if (rc == 0)
			return 0;
	}

	if (!tmp)
		return -1;

	printf(", external..");

	strcpy(movebuf, tmp);
	pre_replace(movebuf, "%S", src);
	pre_replace(movebuf, "%D", dest);

	// try external move
	rc = system(movebuf);

	// return -1 if command did not return '0' = success.
	if (rc != 0)
		return -1;

	return 0;
}

int pre_exec_module(module_list_t *mfunc, filelist_t *files, char *path, char *argv[], struct subdir_list *subdirs) {
	filelist_t *ftmp;
	struct subdir_list *stmp;
	hashtable_t *cfg;
	char *tmpf;
	int rc;

	// run mod_func_dir on each dir
	for (stmp = subdirs; stmp; stmp = stmp->next) {

		tmpf = malloc(strlen(path) + strlen(stmp->dir) + 2);
		if (strlen(stmp->dir) > 0)
			sprintf(tmpf, "%s/%s", path, stmp->dir);
		else {
			// we got the top-dir
			strcpy(tmpf, path);

			// run the mod_func_rel on the rlsdir.
			if (mfunc->mod_func_rel != 0) {
				pre_log("MODULE-REL", "%s %s", mfunc->mod_name, stmp->dir);
				rc = mfunc->mod_func_rel(tmpf, argv);
			}
		}

		if (mfunc->mod_func_dir != 0) {
			pre_log("MODULE-DIR", "%s %s", mfunc->mod_name, stmp->dir);

			rc = mfunc->mod_func_dir(tmpf, argv);
		}

		free(tmpf);

		// break if mod_func_dir signals not to continue.
		if (rc == 0)
			break;
	}


	// run mod_func_file on each file.
	for (ftmp = files; mfunc->mod_func_file && ftmp; ftmp = ftmp->next) {

		pre_log("MODULE-FILE", "%s %s", mfunc->mod_name, ftmp->file);

		tmpf = malloc(strlen(ftmp->file) + strlen(path) + 2);
		sprintf(tmpf, "%s/%s", path, ftmp->file);

		rc = mfunc->mod_func_file(tmpf, argv);

		free(tmpf);

		// if module returns 0, then break.
		if (rc == 0)
			break;
	}

	return 1;
}

int pre_do_module(char *module, filelist_t *files, char *path, char *argv[], struct subdir_list *subdirs) {

	void *handle;
	module_list_t *module_func;
	module_list_t* (*module_loader)();
	void (*set_config)(hashtable_t *ht);
	char *err;

	pre_log("MODULE", "%s", module);

	handle = dlopen(module, RTLD_LAZY);
	if (!handle) {
		err = dlerror();
		pre_log("MODULE-ERROR", "%s \"%s\"", module, err);
		printf("Error loading module %s: %s\n", module, err);
		return 0;
	}

	module_loader = dlsym(handle, MODULE_LOADER_FUNC);
	set_config = dlsym(handle, MODULE_SETCONFIG_FUNC);

	if (!module_loader || !set_config) {
		pre_log("MODULE-ERROR", "%s %s", module, "No loader func found");
		printf("Error loading module %s: No loader func found\n");
		dlclose(handle);
		return 0;
	}

	pre_log("MODULE-RUN", "%s %s", module, path);

	set_config(get_config());

	// try to set environment if module allows.
	set_config = dlsym(handle, MODULE_SETENV_FUNC);
	if (set_config)
		set_config(get_context());

	pre_exec_module(module_loader(), files, path, argv, subdirs);

	pre_log("MODULE-DONE", "%s %s", module, path);

	dlclose(handle);

	return 1;
}

int pre_do_modules(filelist_t *files, char *path, char *argv[], struct subdir_list *subdirs) {
	hashtable_t *cfg, *env;
	stringtokenizer st;
	char *tmp;

	tmp = ht_get(get_config(), PROPERTY_MODULES);

	// no modules in config, return
	if (!tmp)
		return 0;

	st_initialize(&st, tmp, "|");

	while (st_hasnext(&st)) {
		tmp = st_next(&st);

		pre_do_module(tmp, files, path, argv, subdirs);
	}

}


int pre(char *section, char *dest, char *src, char *rel, char *group, char *argv[]) {
	hashtable_t *cfg, *env;

	char buf[1024], tbuf[50], ubuf[300], *tmp;
	filelist_t *files, *ftmp;
	creditlist_t *credits, *tcred;
	struct subdir_list *subdirs;
	float bcount;
	long fnum;
	int olduid, disks = 1;
	pwdfile *pass;
	chowninfo_t *chown;

	cfg = get_config();
	env = get_context();

	pass = pwd_getpwnam(ht_get(env, PROPERTY_USER));

	char *unit = "B";
	float bconv;
	int addmp3genre;
	int debugmp3 = 0;

	if (!pass) {
		printf(" * Error, cannot get your passwd entry! \n");
		return 0;
	}

	tmp = ht_get(cfg, PROPERTY_ADDMP3GENRE);

	if (tmp)
		addmp3genre = atoi(tmp);
	else
		addmp3genre = 0;

	//char mp3_genre[40] = "Unknown";
	char *tmpf = NULL;
	
	chown = chowninfo_find_by_group(group);

	olduid = getuid();
	if (setuid(0) == -1)
		quit(" * Error changing uid, bug sysop to fix perms on foo-pre !\n");

	// find filenames, and collect user's stats + do chowning of files.
	files = filelist_find_by_dir(0, src, "", chown);

	// get totals.
	flist_size(files, &bcount, &fnum);

	// convert bytes.
	if (bcount>=(1024*1024) && bcount<(1024*1024*1024)) { unit = "Mb"; bconv = (float)bcount/(1024*1024); }
	if (bcount>=(1024*1024*1024)) { unit = "Gb"; bconv = (float)bcount/(1024*1024*1024); }
	printf(" * Totals of this pre for announce: %dF %.1f%s\n", fnum, bconv, unit);

	// get credits.
	credits = creditlist_create_from_filelist(files);

	printf(" * Moving files to destination dir.. \n");
	printf("   -- %10.10s: %s\n", "From", src);
	printf("   -- %10.10s: %s", "To", dest);

	/*
	 * slv - get filename.mp3 and call get_mp3_genre(filename).
	 */
	// get genre
	if (addmp3genre) {
		for (ftmp = files; ftmp; ftmp = ftmp->next) {
			if (debugmp3) {
				printf ("\nDEBUG0: ftmp->file: %s\n", ftmp->file);
				printf ("\nDEBUG0: strrchr, strcmp flist_getfilename(ftmp) \".mp3\": %s %i\n",
					flist_getfilename(ftmp), strcmp(strrchr(flist_getfilename(ftmp), '.'), ".mp3"));
			}
			if (strcmp(strrchr(flist_getfilename(ftmp), '.'), ".mp3") == 0) {
				if (debugmp3) { printf ("\nDEBUG0: got mp3 - flist_getfilename(tfmp): %s ftmp->file: %s\n", flist_getfilename(ftmp), ftmp->file); }
				//tmpf = malloc(strlen(flist_getfilename(ftmp))+2);
				tmpf = malloc(strlen(ftmp->file)+2);
				sprintf(tmpf,"%s", flist_getfilename(ftmp));
				if (debugmp3) { printf ("\nDEBUG1: tmpf: %s, break %s\n", tmpf); }
				break;
			}
		}
		if (debugmp3) { printf ("\nDEBUG1: strnlen(tmpf): %s\n", strlen(tmpf)); }
		if ((tmpf != NULL) && (strlen(tmpf) > 0)) {
			if (debugmp3) {
				printf ("\nDEBUG2: if tmpf - flist_getfilename(ftmp): %s\n", flist_getfilename(ftmp));
				printf ("\nDEBUG2: if tmpf - ftmp->file: %s\n", ftmp->file);
				printf ("\nDEBUG2: tmpf %s\n", tmpf);
			}
			sprintf(buf, "%s/%s", src, tmpf);
			if (debugmp3) { printf ("\nDEBUG3: tmpf: %s\nDEBUG3: buf: %s\nDEBUG3: flist_getfilename(ftmp): %s\n", tmpf, buf, flist_getfilename(ftmp)); }
			//tmp = NULL;
			tmp = get_mp3_genre(buf);
			if ((tmp != NULL) && (strlen(tmp) > 0))
				sprintf(mp3_genre, "%s", tmp);
			if (debugmp3) {printf ("\nDEBUG3: tmp: %s\n", tmp); }
			free(tmpf);
		}	
	}

	// dont forget to chown maindir
	chowninfo_apply_to_file(src, chown);

	// do the actual moving of dir.
	if (pre_move_catalog(src, dest) == -1)
		quit(" Failed!\n");
	else
		printf(" Done\n");

	// give credits to original uploaders
	printf(" * Updating userfiles ..\n");
	strcpy(ubuf, "");
	for (tcred = credits; tcred; tcred = tcred->next) {
		tmp = userfile_update(tcred);
		if (tmp)
			strcat(ubuf, tmp);
	}

	setuid(olduid);

	if (strlen(ubuf) > 2)
		ubuf[strlen(ubuf) - 2] = 0;

	// add to dupefile.
	tmp = ht_get(env, PROPERTY_USER);
	for (ftmp = files; ftmp; ftmp = ftmp->next)
		if (!gl_dupefile_add(flist_getfilename(ftmp), tmp))
			printf(" * Error adding to dupefile: %s\n", ftmp->file);

	subdirs = subdir_flistcollect(files);
	reverse_dirlog_add(subdirs, dest, rel, pass, chown);

	disks = subdirlist_count(subdirs);
	if (disks == 0)
		disks++;

	// create announce.
	tmp = group_get_property(group, PROPERTY_GROUP_ANNOUNCE);

	if (tmp) {

		strcpy(buf, tmp);
		sprintf(tbuf, "%.1f", (float)bcount/(1024*1024));
		pre_replace(buf, "%S", tbuf);
		sprintf(tbuf, "%ld", fnum);
		pre_replace(buf, "%W", ubuf);
		pre_replace(buf, "%F", tbuf);
		sprintf(tbuf, "%d", disks);
		pre_replace(buf, "%C", tbuf);
		pre_replace(buf, "%PP", ht_get(env, "RESOLVEDDESTINATION"));
		pre_replace(buf, "%P", section_get_property(section, PROPERTY_SECTION_DIR));
		pre_replace(buf, "%U", ht_get(env, PROPERTY_USER));
		pre_replace(buf, "%T", ht_get(env, PROPERTY_TAGLINE));
		pre_replace(buf, "%G", ht_get(env, PROPERTY_GROUP));
		pre_replace(buf, "%g", ht_get(env, PROPERTY_USERGROUP));
		pre_replace(buf, "%D", section_get_property(section, PROPERTY_SECTION_NAME));
		pre_replace(buf, "%R", rel);
		if (addmp3genre)
			pre_replace(buf, "%I", mp3_genre);

		gl_gllog_add(buf);

	} else
		printf(" * WARNING: No announce setup ! Go bug sysop :( \n");

	// add to dupelog.
	if (!gl_dupelog_add(rel))
		printf(" * Error adding to dupelog !  (prolly wrong perms, ask sysop to fix)\n");

	sprintf(buf, "%s/%s", ht_get(env, "RESOLVEDDESTINATION"), rel);

	// do additional module stuff
	pre_do_modules(files, buf, argv, subdirs);

	return 1;
}

int touch_dir(char *dir) {
	FILE *f;
	char buf[1024];

	sprintf(buf, "%s/.touched", dir);
	f = fopen(buf, "w");

	if (!f)
		return 0;

	fprintf(f, "touched\n");
	fclose(f);

	unlink(buf);

	return 1;
}

// hacky file touch method
int touch_file(char *fname) {
	int fd, needed_chmod, rval, force = 0;
	unsigned char byte;
	struct stat st;

	if (stat(fname, &st) == -1)
		return 0;

	// Try regular files only.
	if (!S_ISREG(st.st_mode))
		return 0;

	needed_chmod = rval = 0;
	if ((fd = open(fname, O_RDWR, 0)) == -1) {
		if (!force || chmod(fname, DEFFILEMODE))
			goto err;
		if ((fd = open(fname, O_RDWR, 0)) == -1)
			goto err;
		needed_chmod = 1;
	}

	if (st.st_size != 0) {
		if (read(fd, &byte, sizeof(byte)) != sizeof(byte))
			goto err;
		if (lseek(fd, (off_t)0, SEEK_SET) == -1)
			goto err;
		if (write(fd, &byte, sizeof(byte)) != sizeof(byte))
			goto err;
	} else {
		if (write(fd, &byte, sizeof(byte)) != sizeof(byte)) {
err:
			rval = 1;
		} else if (ftruncate(fd, (off_t)0)) {
			rval = 1;
		}
	}

	if (close(fd) && rval != 1) {
		rval = 1;
	}

	if (needed_chmod && chmod(fname, st.st_mode) && rval != 1) {
		rval = 1;
	}

	return (rval);
}

/*
 * Gets the real path of a section, trying to expands symlink if nessecary.
 */
char *section_expand_path(char *sec) {
	char tmp[1024], buf[1024], *tmpp;
	int reps;
	time_t now;
	struct tm *tm_now;
	struct stat st;

	strcpy(tmp, section_get_property(sec, PROPERTY_SECTION_DIR));

	now = time(0);
	tm_now = localtime(&now);

	strftime(buf, 1024, "%d", tm_now);
	pre_replace(tmp, "DD", buf);

	strftime(buf, 1024, "%m", tm_now);
	pre_replace(tmp, "MM", buf);

	strftime(buf, 1024, "%Y", tm_now);
	pre_replace(tmp, "YYYY", buf);

	strftime(buf, 1024, "%y", tm_now);
	pre_replace(tmp, "YY", buf);

	strftime(buf, 1024, "%w", tm_now);
	pre_replace(tmp, "WW", buf);

	strftime(buf, 1024, "%W", tm_now);
	pre_replace(tmp, "WOY", buf);

	strftime(buf, 1024, "%V", tm_now);
	pre_replace(tmp, "CW", buf);
	pre_replace(tmp, "KW", buf);

	// if its a link then expand it.
	reps = readlink(tmp, buf, 1024);

	if (reps != -1) {
		if (buf[0] == '/') {
			strncpy(tmp, buf, reps);
			tmp[reps] = '\0'; /* ensure null terminated */
			buf[reps] = 0;
		}
		else {
			tmpp= strrchr(tmp, '/');
			if (!tmpp)
				quit(" * Symlink expansion error..");

			strncpy(tmpp + 1, buf, reps);
			*(tmpp + 1 + reps) = 0;
		}
	}

	if (stat(tmp, &st) == -1) {
		sprintf(buf,
				" * Hm, destination section's path (%s) doesnt exist ?\n",
				tmp);

		quit(buf);
	}

	return strdup(tmp);
}



int pre_handler(int argc, char *argv[]) {
	hashtable_t *env, *cfg;
	strlist_t *groups;
	char *dest_section, *destpath;
	struct stat st;
	char source[1024], destination[1024], *tmp, *group;
	char buf[1024];
	int rc;

	int addmp3genre;

	env = get_context();
	cfg = get_config();

	if (argc > 2)
		pre_log("START", "\"%s\" \"%s\" \"%s\"", ht_get(env, PROPERTY_USER), argv[1], argv[2]);
	else
		pre_log("START", "\"%s\" \"%s\"", ht_get(env, PROPERTY_USER), argv[1]);

	// set etcdir for the pwd functions
	if (tmp = ht_get(cfg, PROPERTY_ETCDIR))
		pwd_set_etcdir(tmp);

	if (tmp = ht_get(cfg, PROPERTY_TEXT_HEAD))
		printf(tmp);

	groups = user_find_groups(ht_get(env, PROPERTY_USER));

	if (!groups)
		quit(" * Error finding your groups, go bug sysop!\n");

	tmp = ht_get(cfg, PROPERTY_ADDMP3GENRE);

	if (argc < 2) {
		printf(USAGE);

		show_groupdirs(groups);

		quit(0);
	}

	if (tmp)
		addmp3genre = atoi(tmp);
	else
		addmp3genre = 0;

	// check if someone are trying to fool us.
	if (strchr(argv[1], '/'))
		quit(" * You cant give paths in releasename ('/' not allowed)!\n");

	char *sourcebis = getcwd(NULL, 0);

	// check if we are in a position to pre.
	group = group_find_by_dir(groups, sourcebis);

	if (!group) {
		printf(" * You are not in the group-dir of any of your groups.\n\n");

		show_groupdirs(groups);

		quit(0);
	}

	pre_log("GROUP", "\"%s\" \"%s\"", sourcebis, group);

	printf(" * Looks like this is going to be a %s pre..\n", group);
	ht_put(env, PROPERTY_GROUP, group);

	// check if we have chosen a valid destination for our pre.
	dest_section = section_find_by_name(group, argc > 2 ? argv[2] : 0);

	if (!dest_section) {
		show_groupdirs(groups);

		quit(0);
	}

	printf(" * Destination for pre will be the %s section..\n", dest_section);
	ht_put(env, "section", dest_section);

	destpath = section_expand_path(dest_section);
	ht_put(env, "RESOLVEDDESTINATION", destpath);

	strcpy(source, sourcebis);
	strcat(source, "/");
	strcat(source, argv[1]);

	// check if source dir is okay.
	if ((stat(source, &st) == -1) || !S_ISDIR(st.st_mode)) {
		sprintf(source, " * Hm, '%s' doesnt exist or isnt a dir ?\n",
				argv[1]);
		quit(source);
	}

	// touch the source.
	touch_dir(source);

	// check if destination dir exists.
	sprintf(destination, "%s/%s", destpath, argv[1]);

	rc = stat(destination, &st);

	// try rename if requested
	if ((rc == 0) && (argc > 3) && (!strcasecmp(argv[3], "force"))) {

		sprintf(buf, "%s_TRADING", destination);
		rc = rename(destination, buf);

		if (rc == 0)
			printf(" + Renamed existing to %s_TRADING ..\n", argv[1]);
		else
			printf(" ! Failed rename existing to %s_TRADING ..\n", argv[1]);
	}

	// check if destination exists.
	if (stat(destination, &st) == -1)
		pre(dest_section, destination, source, argv[1], group, argv);
	else {
		tmp = "(this will rename the existing dir, which you can then nuke or wipe afterwards!)";
		sprintf(source, " * Hm destination already exists. You're too late with pre!\n + Use SITE PRE %s %s FORCE to force pre.\n   %s\n", argv[1], dest_section, tmp);
		quit(source);
	}

	// log DONE: "<preuser>" "<pregroup>" "<release>" "<destinationdir>" [ slv: added "<genre>" ]
	if (addmp3genre)
		pre_log("DONE", "\"%s\" \"%s\" \"%s\" \"%s\" \"%s\"",
			ht_get(env, PROPERTY_USER), group,
			argv[1], destpath, mp3_genre);
	else
		pre_log("DONE", "\"%s\" \"%s\" \"%s\" \"%s\"",
			ht_get(env, PROPERTY_USER), group,
			argv[1], destpath);
	

	return 0;
}

/*
 * Initialize the environment.
 */
int pre_init() {
	hashtable_t *tmp;
	char *sanity;

	// load config.
	tmp = get_config();
	ht_load_prop(tmp, PRE_CONFIGFILE, '=');

	// lame check on environment. prevents idiots from sending me
	// 'why does it segv when i run it ???' questions.
	sanity = getenv("TAGLINE");
	if (!sanity) {
		printf("Did not find environment for glftpd, please run only from within glftpd as 'site' command!\n");
		exit(1);
	}

	// put env variables into the context.
	tmp = get_context();
	ht_put(tmp, PROPERTY_USER, getenv("USER"));
	ht_put(tmp, PROPERTY_TAGLINE, getenv("TAGLINE"));
	ht_put(tmp, PROPERTY_USERGROUP, getenv("GROUP"));
}


/*
 * Main method.
 */
int main(int argc, char *argv[]) {
	int rc;

	pre_init();

	rc = pre_handler(argc, argv);

	quit(0);

	// never reached.
	return 0;
}


void quit(char *s, ...) {
	hashtable_t *cfg = get_config();
	char *tmp;
	va_list va;

	if (s) {
		va_start(va, s);

		vprintf(s, va);
		va_end(va);
	}

	if (tmp = ht_get(cfg, PROPERTY_TEXT_TAIL))
		printf(tmp, VERSION);

	exit(0);
}
/* vim: set noai tabstop=8 shiftwidth=8 softtabstop=8 noexpandtab: */
