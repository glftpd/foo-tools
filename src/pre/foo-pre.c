/*
 *  foo.Pre [C-version]  (c)  tanesha team, <tanesha@tanesha.net>
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

#include <lib/gllogs.h>
#include <lib/pwdfile.h>
#include <collection/strlist.h>
#include <lib/stringtokenizer.h>
#include <collection/hashtable.h>
#include <util/linefilereader.h>

#include "foo-pre.h"

#define VERSION "$Id: foo-pre.c,v 1.20 2002/03/14 22:02:08 flower Exp $"
#define USAGE " * Syntax: SITE PRE <RELEASEDIR> <SECTION>\n"

void quit(char *s, ...);
extern int errno;

hashtable_t *_config = 0;
hashtable_t *_envctx = 0;

/*
 * Acecssor method for configuration.
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

	f = (FILE*)ht_get(env, "logfh");

	if (!f) {
		f = fopen(PRE_LOGFILE, "a");
		ht_put_obj(env, "logfh", f);
	}

	fprintf(f, "%s: ", type);
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
	char buf[300];

	sprintf(buf, "group.%s.%s", grp, prop);

	return ht_get(cfg, buf);
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
	char buf[1024], *tmp;
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
			groups = str_add(groups, strchr(buf, ' ') + 1);
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
	char *tmpsecdir, *tmpallow;
	stringtokenizer st;

	st_initialize(&st, group_get_property(group, PROPERTY_GROUP_ALLOW), "|");

	while (st_hasnext(&st)) {
		tmpallow = strdup(st_next(&st));

		if (!strcasecmp(requested, tmpallow))
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
		printf(" * Hm, you are allowed to pre in section '%s' but the section is not.\n   configured. Bug siteop to fix his pre configuration please! :)\n", requested);

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

	if (!tmp)
		tmp = group_get_property("DEFAULT", PROPERTY_GROUP_RATIO);

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

		chowninfo_apply_to_file(buf, chown);

		if (S_ISDIR(st.st_mode)) {

			if (strlen(path) > 0)
				sprintf(buf, "%s/%s", path, dent->d_name);
			else
				strcpy(buf, dent->d_name);

			// find recursively.
			l = filelist_find_by_dir(l, base, buf, chown);

		} else {
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


int pre(char *section, char *dest, char *src, char *rel, char *group) {
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

	if (!pass) {
		printf(" * Error, cannot get your passwd entry! \n");
		return 0;
	}

	chown = chowninfo_find_by_group(group);

	olduid = getuid();
	if (setuid(0) == -1)
		quit(" * Error changing uid, bug sysop to fix perms on foo-pre !\n");

	// find filenames, and collect user's stats + do chowning of files.
	files = filelist_find_by_dir(0, src, "", chown);

	// get totals.
	flist_size(files, &bcount, &fnum);
	printf(" * Totals of this pre for announce: %dF %.0fB\n", fnum, bcount);

	// get credits.
	credits = creditlist_create_from_filelist(files);

	printf(" * Moving files to destination dir.. \n");
	printf("   -- %10.10s: %s\n", "From", src);
	printf("   -- %10.10s: %s", "To", dest);

	// dont forget to chown maindir
	chowninfo_apply_to_file(src, chown);

	// do the actual moving of dir.
	if (rename(src, dest) == -1)
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

	// try default announce.
	if (!tmp)
		tmp = group_get_property("DEFAULT", PROPERTY_GROUP_ANNOUNCE);

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

		gl_gllog_add(buf);

	} else
		printf(" * WARNING: No announce setup ! Go bug sysop :( \n");

	// add to dupelog.
	if (!gl_dupelog_add(rel))
		printf(" * Error adding to dupelog !  (prolly wrong perms, ask sysop to fix)\n");

	return 1;
}

int touch(char *dir) {
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

/*
 * Gets the real path of a section, trying to expands symlink if nessecary.
 */
char *section_expand_path(char *sec) {
	char tmp[1024], buf[1024], *tmpp;
	int reps;
	struct stat st;

	strcpy(tmp, section_get_property(sec, PROPERTY_SECTION_DIR));

	if (stat(tmp, &st) == -1) {
		sprintf(buf,
				" * Hm, destination section's path (%s) doesnt exist ?\n",
				tmp);

		quit(buf);
	}

	// if its a link then expand it.
	reps = readlink(tmp, buf, 1024);

	if (reps != -1) {
		if (buf[0] == '/') {
			strncpy(tmp, buf, reps);
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

	return strdup(tmp);
}



int pre_handler(int argc, char *argv[]) {
	hashtable_t *env, *cfg;
	strlist_t *groups;
	char *dest_section, *destpath;
	struct stat st;
	char source[1024], destination[1024], *tmp, *group;
	char buf[1024];

	env = get_context();
	cfg = get_config();

	pre_log("START", "%s %s %s", ht_get(env, PROPERTY_USER), argv[1], argv[2]);

	if (tmp = ht_get(cfg, PROPERTY_TEXT_HEAD))
		printf(tmp);

	groups = user_find_groups(ht_get(env, PROPERTY_USER));

	if (!groups)
		quit(" * Error finding your groups, go bug sysop!\n");

	if (argc < 3) {
		printf(USAGE);

		show_groupdirs(groups);

		quit(0);
	}

	// check if someone are trying to fool us.
	if (strchr(argv[1], '/'))
		quit(" * You cant give paths in releasename ('/' not allowed)!\n");

	getcwd(source, 300);

	// check if we are in a position to pre.
	group = group_find_by_dir(groups, source);

	if (!group) {
		printf(" * You are not in the group-dir of any of your groups.\n\n");

		show_groupdirs(groups);

		quit(0);
	}

	pre_log("GROUP", "%s %s", source, group);

	printf(" * Looks like this is going to be a %s pre..\n", group);
	ht_put(env, PROPERTY_GROUP, group);

	// check if we have chosen a valid destination for our pre.
	dest_section = section_find_by_name(group, argv[2]);

	if (!dest_section) {
		show_groupdirs(groups);

		quit(0);
	}

	printf(" * Destination for pre will be the %s section..\n", dest_section);
	ht_put(env, "section", dest_section);

	destpath = section_expand_path(dest_section);
	ht_put(env, "RESOLVEDDESTINATION", destpath);

	sprintf(source, "%s/%s", source, argv[1]);

	// check if source dir is okay.
	if ((stat(source, &st) == -1) || !S_ISDIR(st.st_mode)) {
		sprintf(source, " * Hm, '%s' doesnt exist or isnt a dir ?\n",
				argv[1]);
		quit(source);
	}

	// touch the source.
	touch(source);

	// check if destination dir exists.
	sprintf(destination, "%s/%s", destpath, argv[1]);
	if (stat(destination, &st) == -1)
		pre(dest_section, destination, source, argv[1], group);
	else {
		sprintf(source, " * Hm destination already exists. You're too late with pre!");
		quit(source);
	}

	return 0;
}

/*
 * Initialize the environment.
 */
int pre_init() {
	hashtable_t *tmp;

	// load config.
	tmp = get_config();
	ht_load_prop(tmp, PRE_CONFIGFILE, '=');

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

