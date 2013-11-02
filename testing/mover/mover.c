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
 * foo.Mover, the mover ;>
 */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <dirent.h>
#include "mover.h"
#include <collection/hashtable.h>
#include <lib/stringtokenizer.h>
#include <collection/strlist.h>

int interactive = 0;
config_t cfg;

#define DO_ARCH 1
#define DO_INC  1

#define BUFSIZE 1024

void open_lockfile(char *s);
void quit(int i);

int do_continue() {
	if (interactive == 0)
		return 1;

	printf(" .. hit any key to continue with this operation (ctrl-c to abort) ..\n");
	getchar();

	return 1;
}

archive_item_t * load_archive(hashtable_t *ht, char *name) {

	archive_item_t *ai;
	char *tmp, buf[300];
	stringtokenizer st;
	
	sprintf(buf, "archive.%s", name);
	tmp = ht_get(ht, buf);

	if (!tmp)
		return 0;

	st_initialize(&st, tmp, ",");

	if (st_count(&st) != 2) {
		printf("Illegal archive entry 'archive.%s'\n", name);
		return 0;
	}

	ai = malloc(sizeof(archive_item_t));

	ai->name = strdup(name);
	ai->dir = strdup(st_next(&st));
	strcpy(buf, st_next(&st));
	sscanf(buf, "%lf", &ai->margin);

	st_finalize(&st);

	return ai;
}


archive_item_t * find_archivegrp(config_t *c, char *n) {
	archive_item_t* tmp;

	tmp = (archive_item_t*)ht_get(&c->archives, n);

	// not found, try to load it from config.
	if (!tmp) {
		tmp = load_archive(&c->configfile, n);
		if (tmp)
			ht_put_obj(&c->archives, n, tmp);
	}

	return tmp;
}



int read_config(char *cf, config_t *cfgt) {
	char *tmp, buf[30];
	hashtable_t ht;
	stringtokenizer st;
	int got = 0, i, rules = 0;
	mover_item_t *tmi;
	archive_item_t *ta;



	ht_init(&cfgt->archives);
	sortedlist_init(&cfgt->rules);

	cfgt->nuked_age = 0;

	ht_init(&cfgt->configfile);

	if (!ht_load(&cfgt->configfile, cf))
		return 0;

	printf("Loaded config ..\n");

	if (tmp = ht_get(&cfgt->configfile, "move.bufsize")) {
		cfgt->bufsize = atoi(tmp);
		got++;
	}

	if (tmp = ht_get(&cfgt->configfile, "move.usleept")) {
		cfgt->usleept = atoi(tmp);
		got++;
	}

	if (tmp = ht_get(&cfgt->configfile, "incoming.dir")) {
		cfgt->incomingdir = strdup(tmp);
		got++;
	}

	if (tmp = ht_get(&cfgt->configfile, "incoming.free")) {
		sscanf(tmp, "%lf", &cfgt->incomingfree);
		got++;
	}

	if (tmp = ht_get(&cfgt->configfile, "lockfile")) {
		cfgt->lockfile = strdup(tmp);
		got++;
	}

	if (tmp = ht_get(&cfgt->configfile, "nuked.age"))
		sscanf(tmp, "%lu", &cfgt->nuked_age);

	if (tmp = ht_get(&cfgt->configfile, "nuked.dirstyle"))
		cfgt->nuked_dirstyle = strdup(tmp);

	tmp = ht_get(&cfgt->configfile, "rules");
	if (!tmp) {
		printf("No 'rules' property in config file.\n");
		return 0;
	}

	rules = atoi(tmp);

	printf("Reading rules ..\n");

	for (i = 0; i < rules; i++) {
		sprintf(buf, "rule.%d", (i + 1));
		tmp = ht_get(&cfgt->configfile, buf);

		// no rule was defined at this offset.
		if (!tmp)
			continue;

		st_initialize(&st, tmp, ",");

		if (st_count(&st) != 4) {
			printf("Bad rule in config '%s'\n", tmp);
			st_finalize(&st);
			continue;
		}

		tmi = (mover_item_t*)malloc(sizeof(mover_item_t));

		tmi->rule_order = i;
		tmi->incoming_dir = strdup(st_next(&st));
		tmi->archive_dir = strdup(st_next(&st));

		tmi->dir_mask = strdup(st_next(&st));

		// get the archive section for this to use.
		strcpy(buf, st_next(&st));

		tmi->archivegroup = find_archivegrp(cfgt, buf);

		if (!strcasecmp(tmi->archive_dir, "delete"))
			tmi->delete = 1;
		else
			tmi->delete = 0;

		sortedlist_add(&cfgt->rules, tmi);

		got++;
	}

	ht_finalize(&cfgt->configfile);

	if (got > 7)
		return 1;

	return 0;
};


/*
 * Gets archive dirs for a specific section.
 */
strlist_t *get_archive_dirs(config_t *c, mover_item_t *incoming) {
	strlist_t *dirs = 0;
	archive_item_t *agrp;
	mover_item_t *tmp;

	if (incoming->delete)
		return 0;

	agrp = incoming->archivegroup;

	if (!agrp) {
		printf("Fail -> No archive grp found for %s\n", incoming->incoming_dir);

		quit(1);
	}

	// add the archive dirs for the sections that uses same archive grp.
	for (sortedlist_reset(&c->rules); tmp = (mover_item_t*)sortedlist_next(&c->rules); ) {
		if (tmp->archivegroup == agrp)
			dirs = str_add(dirs, tmp->archive_dir);
	}

	return dirs;
}

/*
 * Moves a file 'src' to destination 'dest'.
 */
int move_file(char *src, char *dest) {
	FILE *in, *out;
	char buf[cfg.bufsize];
	int n = 0;
	struct stat st;

	if (stat(src, &st) == -1) {
		printf("FAIL -> statting src in move_file %s\n", src);

		return 0;
	}

	out = fopen(dest, "wb");

	if (!out) {
		printf("FAIL -> opening dest in move_file %s\n", dest);

		return 0;
	}

	in = fopen(src, "rb");

	if (!in) {
		fclose(out);
		unlink(dest);

		printf("FAIL -> opening src in move_file %s\n", src);

		return 0;
	}

	// set permissions.
	fchmod(fileno(out), st.st_mode);
	fchown(fileno(out), st.st_uid, st.st_gid);

	// copy file at slower speed.
	while ((n = fread(buf, 1, cfg.bufsize, in)) > 0) {
		fwrite(buf, n, 1, out);

		usleep(cfg.usleept);
	}

	fflush(out);

	fclose(in);
	fclose(out);

	if (unlink(src) == -1) {
		printf("FAIL -> unlinking in move_file %s\n", src);

		return 0;
	}

	return 1;
}


/*
 * Finds the size of a dir with subdirs.
 */
double find_dir_size(char *d, int multiplier) {
	DIR *dh;
	struct dirent *de;
	struct stat st;
	double tmp = 0;
	char buf[BUFSIZE];

	dh = opendir(d);

	if (!dh)
		return 0;

	while (de = readdir(dh)) {
		if (!strncmp(de->d_name, ".", 1))
			continue;

		sprintf(buf, "%s/%s", d, de->d_name);

		if (stat(buf, &st) == -1)
			continue;

		if (S_ISDIR(st.st_mode))
			tmp += find_dir_size(buf, multiplier);
		else if (S_ISREG(st.st_mode))
			tmp += (double)st.st_size / multiplier;
	}

	closedir(dh);

	return tmp;
}


/*
 * Finds free space in Kb for the filesystem of a 'dir'.
 */
double find_df(char *dir, int multiplier) {
	struct statvfs fst;
	double free = 0;

	if (statvfs(dir, &fst) == -1) {
		printf("Fatal -> Cannot get free space for '%s', Check config!\n", dir);
		quit(1);
	}

	printf("Free space %s -> %.1f\n", dir, ((double)fst.f_bsize * fst.f_bavail) / multiplier);

	free = ((double) fst.f_bsize * fst.f_bavail) / multiplier;

	return free;
}


/*
 * Moves a dir and its subdirs to destination dir.
 */
int move_recursive(char *src, char *dest) {
	DIR *dh;
	struct dirent *de;
	struct stat st, srcst;
	char srcfile[BUFSIZE], destfile[BUFSIZE];

	printf("moving -> %s, %s\n", src, dest);

	if (stat(src, &srcst) == -1) {
		printf("FAIL -> statting %s\n", src);
		return 0;
	}

	// try to create destination dir if it doesnt exist.
	if (stat(dest, &st) == -1) {
		if (mkdir(dest, srcst.st_mode) == -1) {
			printf("FAIL -> error mkdir %s\n", dest);
			return 0;
		}

		if (chown(dest, srcst.st_uid, srcst.st_gid) == -1) {
			printf("FAIL -> error chown'ing %s\n", dest);
			return 0;
		}
	}

	dh = opendir(src);

	if (!dh) {
		printf("FAIL -> error opening dir %s\n", src);
		return 0;
	}

	while (de = readdir(dh)) {
		if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
			continue;

		sprintf(srcfile, "%s/%s", src, de->d_name);
		sprintf(destfile, "%s/%s", dest, de->d_name);

		if (stat(srcfile, &st) == -1) {
			printf("FAIL -> statting %s\n", srcfile);
			closedir(dh);

			return 0;
		}

		if (S_ISDIR(st.st_mode)) {
			if (!move_recursive(srcfile, destfile)) {
				closedir(dh);

				return 0;
			}
		} else {
			if (!move_file(srcfile, destfile)) {
				closedir(dh);
				return 0;
			}
		}
	}

	closedir(dh);

	rmdir(src);

	return 1;
}

/*
 * Initializes destination for move of the dir 'dir'.
 */
int move_dir(config_t *c, char *dir, mover_item_t *section) {
	char buf[BUFSIZE], oldest[BUFSIZE], parent[BUFSIZE];
	double srcsize = 0, free = 0;
	strlist_t *arcdirs = 0;

	sprintf(buf, "%s/%s", section->incoming_dir, dir);

	srcsize = find_dir_size(buf, 1024);

	free = find_df(section->archive_dir, 1024);

	if (free == 0) {
		printf("FAIL -> error getting free space for %s (or 0 bytes free).\n", section->archive_dir);

		return 0;
	}

	printf("Size -> %s, %.1f (%.1f)\n", buf, srcsize, free);

	do_continue();

	arcdirs = get_archive_dirs(c, section);

	if (!arcdirs) {
		printf("No archive dirs found for %s\n", section->incoming_dir);
		quit(0);
	}

	// make sure that we have enough free space for it.
	while (find_df(section->archive_dir, 1024) < (srcsize + section->archivegroup->margin)) {
		if (!find_oldest_dir_list(arcdirs, oldest, parent)) {
			printf("FAIL -> couldnt find oldest in %s\n", section->archive_dir);
			return 0;
		}

		sprintf(buf, "%s/%s", parent, oldest);

		if (!unlink_dir(buf)) {
			printf("Fail -> Coudlnt unlink oldest dir in archive: %s\n", buf);
			return 0;
		}
	}

	// start moving the stuff.
	sprintf(oldest, "%s/%s", section->archive_dir, dir);
	sprintf(buf, "%s/%s", section->incoming_dir, dir);

	return move_recursive(buf, oldest);
}


/*
 * Unlinks a dir with subdirs.
 */
int unlink_dir(char *dir) {
	DIR *dh;
	struct dirent *dent;
	struct stat st;
	char buf[1024];

	printf("unlink dir -> %s\n", dir);

	do_continue();

	dh = opendir(dir);

	if (!dh) {
		printf("FAIL -> opening dir in unlink_dir %s\n", dir);

		return 0;
	}

	while (dent = readdir(dh)) {
		if (!strcmp(dent->d_name, "..") || !strcmp(dent->d_name, "."))
			continue;

		sprintf(buf, "%s/%s", dir, dent->d_name);

		if (stat(buf, &st) == -1) {
			printf("FAIL -> statting in unlink_dir %s\n", buf);

			return 0;
		}

		if (S_ISDIR(st.st_mode)) {
			if (!unlink_dir(buf)) {
				closedir(dh);

				return 0;
			}
		} else {
			if (unlink(buf) == -1) {
				printf("FAIL -> unlinking in unlink_dir %s\n", buf);

				closedir(dh);

				return 0;
			}
		}
	}

	closedir(dh);

	if (rmdir(dir) == -1) {
		printf("FAIL -> unlinking dir in unlink_dir %s\n", dir);

		return 0;
	}

	return 1;
}

int rule_order_sortfunc(void *p, void *q) {

	mover_item_t *a, *b;

	a = (mover_item_t*)p;
	b = (mover_item_t*)q;

	if (a->rule_order < b->rule_order)
		return 1;

	return 0;
}

/*
 * Finds the oldest dir out from a strlist containing some paths to look
 * through.
 */
int find_oldest_dir_list(strlist_t *c, char *dir, char *path) {

	int found = 0;
	DIR *dh;
	struct dirent *dent;
	struct stat st;
	strlist_iterator_t *i;
	char *tmpdir, buf[BUFSIZE];
	time_t oldest = 0;

	for (i = str_iterator(c); tmpdir = str_iterator_next(i); ) {

		dh = opendir(tmpdir);


		if (!dh)
			continue;


		while (dent = readdir(dh)) {

			if (!strncmp(dent->d_name, ".", 1))
				continue;

			sprintf(buf, "%s/%s", tmpdir, dent->d_name);

			if (stat(buf, &st) == -1)
				continue;

			if (!S_ISDIR(st.st_mode))
				continue;

			if ((st.st_mtime >= oldest) && (oldest > 0))
				continue;

			oldest = st.st_mtime;
			strcpy(dir, dent->d_name);
			strcpy(path, tmpdir);

			found = 1;

		}

		closedir(dh);

	}

	free(i);

}



/*
 * Finds oldest subdir in the directory 'checkdir'.
 *
 * returns the section of the found dir, or 0 if nothing was found.
 * also, on success, 'dir' is filled with the oldest dir.
 */
mover_item_t * find_oldest_dir_inc(config_t *c, char *dir) {
	int i = 0;
	DIR *dh;
    mover_item_t *tmp, *section = 0;
	struct dirent *dent;
	struct stat st;
	time_t oldest = 0;
	char buf[BUFSIZE];

	dir[0] = 0;

	sortedlist_sort(&c->rules, rule_order_sortfunc);
	sortedlist_reset(&c->rules);

	while (tmp = (mover_item_t*)sortedlist_next(&c->rules)) {

		printf("Finding oldest dir with rule: %d\n", tmp->rule_order);

		dh = 0;

		dh = opendir(tmp->incoming_dir);
		
		if (!dh) {
			printf("Warning -> Incoming dir not found '%s' -> Skipping.\n", tmp->incoming_dir);

			continue;
		}

		while (dent = readdir(dh)) {
			if (!strncmp(dent->d_name, ".", 1))
				continue;

			// check if this one matches the pattern.
			if (fnmatch(tmp->dir_mask, dent->d_name, 0) != 0)
				continue;

			sprintf(buf, "%s/%s", tmp->incoming_dir, dent->d_name);

			if (stat(buf, &st) == -1)
				continue;

			if (!S_ISDIR(st.st_mode))
				continue;

			if ((st.st_mtime >= oldest) && (oldest > 0))
				continue;

			// a previos rule already matched this dir.
			if (!strcmp(dir, dent->d_name))
				continue;

			oldest = st.st_mtime;
			strcpy(dir, dent->d_name);
			section = tmp;
		}


		closedir(dh);
	}

	return section;
}

strlist_t *get_incoming_dirs(config_t *c) {
	mover_item_t *tmp;
	strlist_t *ret = 0;

	sortedlist_reset(&c->rules);
	while (tmp = sortedlist_next(&c->rules))
		if (!str_search(ret, tmp->incoming_dir, 0))
			ret = str_add(ret, tmp->incoming_dir);

	return ret;
}

void show_config(config_t *c) {
	mover_item_t *t;

	printf("incoming: %s / %.1f\n", c->incomingdir, c->incomingfree);
	printf("move-usleept: %d, move-bufsize: %d\n", c->usleept, c->bufsize);
	printf("lockfile: %s\n", c->lockfile);

	sortedlist_reset(&c->rules);

	while (sortedlist_hasnext(&c->rules)) {
		t = (mover_item_t*)sortedlist_next(&c->rules);

		printf("rule: %s -> %s (Arc: %s)\n", t->incoming_dir,
			   (t->delete?"Delete files":"Move to archive"),
			   (t->archivegroup ? t->archivegroup->name : "No archivegroup"));
	}
}

int unlink_nuked_dirs(config_t *c) {
	mover_item_t *i;
	DIR *dh;
	struct dirent *dent;
	char buf[300];
	struct stat st;
	int found = 0;
	time_t now, oldest;

	now = time(0);
	oldest = now - (c->nuked_age * 3600);

	sortedlist_reset(&c->rules);

	while (sortedlist_hasnext(&c->rules)) {
		i = (mover_item_t*)sortedlist_next(&c->rules);

		dh = opendir(i->incoming_dir);

		if (!dh)
			continue;

		while (dent = readdir(dh)) {
			if (fnmatch(c->nuked_dirstyle, dent->d_name, 0) != 0)
				continue;

			sprintf(buf, "%s/%s", i->incoming_dir, dent->d_name);
			if (stat(buf, &st) == -1)
				continue;

			if (st.st_mtime > oldest)
				continue;

			printf("Unlink Nuked-rls (Age: %d h) -> %s\n", (now - st.st_mtime) / 3600, buf);

			found++;

			if (!unlink_dir(buf))
				printf("FAIL -> Couldnt unlink dir: %s\n", buf);

		}
	}

	return found;
}


/*
 * Main program loop.
 */
int main(int argc, char *argv[]) {
	double free;
	char src[500], arc[500];
	struct strlist *incomingdirs;
	mover_item_t *tmpdirs, *rule;

	if (argc < 2) {
		printf("syntax -> ./mover <configfile>\n");

		exit(1);
	}

	if (argc > 2)
		interactive = 1;
	else
		interactive = 0;

	printf("Reading config ..\n");

	if (!read_config(argv[1], &cfg)) {
		printf("Error reading config, check that your configfile got all needed stuff.\n");
		exit(1);
	}

	if (interactive)
		show_config(&cfg);

	open_lockfile(cfg.lockfile);

	if (cfg.nuked_age > 0)
		unlink_nuked_dirs(&cfg);

	free = find_df(cfg.incomingdir, 1024);

	if (free > cfg.incomingfree) {
		printf("Ok -> %s, %.1f, %.1f\n", cfg.incomingdir, free, cfg.incomingfree);
		quit(0);
	}
	else if (free == -1) {
		printf("FAIL -> error getting free space for %s\n", cfg.incomingdir);
		quit(0);
	}

	incomingdirs = get_incoming_dirs(&cfg);

	printf("Info -> searching for oldest dir ..\n");
	while (find_df(cfg.incomingdir, 1024) < cfg.incomingfree) {

		// set rule to 0
		rule = 0;

		rule = find_oldest_dir_inc(&cfg, src);

		if (!rule) {
			printf("FAIL -> find_oldest_dir didnt find oldest dir %s\n", cfg.incomingdir);
			break;
		}

		if (rule->delete) {
			sprintf(arc, "%s/%s", rule->incoming_dir, src);
			printf("No-archive delete -> %s\n", arc);
			if (!unlink_dir(arc))
				break;
		} else {
			printf("Move -> %s/%s, %s\n", rule->incoming_dir, src, rule->archive_dir);
			if (!move_dir(&cfg, src, rule))
				break;
		}
	}

	printf("Done -> %s, %.1f\n", cfg.incomingdir, find_df(cfg.incomingdir, 1024));

	quit(0);
}

void open_lockfile(char *s) {
	FILE *f;
	struct stat st;

	if (stat(s, &st) != -1) {
		printf("FAIL -> already running, %s exists\n", s);

		exit(0);
	}

	f = fopen(s, "w");

	if (!f) {
		printf("FAIL -> error opening pidfile %s\n", s);

		exit(0);
	}

	fprintf(f, "%d\n", getpid());
	fclose(f);
}

void quit(int i) {
	unlink(cfg.lockfile);
	exit(i);
}
