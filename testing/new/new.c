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

#include <time.h>
#include <string.h>

#include "dirlog.h"
#include "dirlog_wrap.h"

#include <collection/sortedlist.h>
#include <collection/strlist.h>
#include <lib/dirlist.h>
#include <lib/sfv.h>
#include <util/date.h>
#include <lib/pwdfile.h>


#define CACHEDIR "/tmp/new-cache"


// small struct to use for getting winners.
struct bytesum {
	char *user;
	int files;
	float bytes;
};
typedef struct bytesum bytesum_t;


// used for calcing ages.
static time_t NOW;



int show_make_percent(int ok, int total, int width, char uncheck, char check, char *out) {
	int percent, i;

	if (total == 0) {

		for (i = 0; i < width; i++)
			*(out++) = uncheck;

		*out = 0;
	} else {
		percent = (width * ok) / total;

		for (i = 0; (i < percent) && (i < width); i++)
			*(out++) = check;

		for (i = percent; i < width; i++)
			*(out++) = uncheck;

		*out = 0;
	}

	return 1;
}


/*
 * Finds files missing in the dir according to sfv file.
 */
strlist_t * find_missing(dirlist_t *dl, sfv_list_t *sfv) {
	strlist_t *l = 0;
	dirlist_item_t *sf;
	sfv_list_t *tmp;
	int is_x;

	for (tmp = sfv; tmp; tmp = tmp->next) {
		sf = dirlist_get_file(dl, tmp->filename);

		if (sf && !(sf->st.st_mode & S_IXUSR))
			continue;

		l = str_add(l, tmp->filename);
	}

	return l;
}


int is_complete(di_t *log) {
	dirlist_t *dl;
	sfv_list_t *sfv;
	char sfvfile[256], buf[1024];
	strlist_t *tmp = 0;

	dl = (dirlist_t*)log->di.nxt;

	// init.
	log->di.prv = 0;

	// no sfv, return -1
	if (!dirlist_get_sfv(dl, sfvfile))
		return -1;

	sprintf(buf, "%s/%s", dl->dir, sfvfile);
	sfv = sfv_list_load(buf);

	// save for later.
	(sfv_list_t*)log->di.prv = sfv;

	if (!sfv)
		return -1;

	tmp = find_missing(dl, sfv);

	if (str_count(tmp) == 0)
		return 1;

	return 0;
}

int show_get_cds(release_t *rel, char *esubdir, char *extras) {
	int s = 0;
	di_t *di;
	hashtable_item_t *hi;
	char *sn;

	esubdir[0] = 0;
	extras[0] = 0;

	// figure if there are subdirs.
	for (ht_reset(&rel->subdirs); ht_hasnext(&rel->subdirs); ) {
		hi = ht_next(&rel->subdirs);

		di = (di_t*)hi->value;
		sn = hi->key;

		if (!strncasecmp(sn, "CD", 2) || !strncasecmp(sn, "DISC", 4))
			s++;
		else if (!strncasecmp(sn, "Approve", 7)) {
			strcpy(extras, "-APPROVED-");
		} else {
			strcat(esubdir, sn);
			strcat(esubdir, " ");
		}
	}

	return s;
}


void show_get_complete_status(release_t *rel, char *buf, char *extras) {

	char esubdir[200];
	int s = 0, i = 0;

	s = show_get_cds(rel, esubdir, extras);

	// if there is a sfv.
	if (rel->maindir.di.prv != 0) {
		sprintf(buf, "%d x %.0fM", sfv_list_count((sfv_list_t*)rel->maindir.di.prv),
				(float)dirlist_get_filesize((dirlist_t*)rel->maindir.di.nxt)/(1000*1000));
	} else if (s > 0) {
		sprintf(buf, "%d CDs, %s", s, esubdir);
	} else if (rel->maindir.di.files > 0) {
		sprintf(buf, "%dF %.0fM", rel->maindir.di.files,
				(float)rel->maindir.di.bytes/(1024*1024));
	} else {
		sprintf(buf, "-EMPTY-");
	}
}

void show_get_incomplete_status(di_t *di, char *buf) {

	char perbuf[300];
	sfv_list_t *sfv;
	dirlist_t *dl;
	strlist_t *tmp = 0;
	int sc, st;

	sfv = (sfv_list_t*)di->di.prv;
	dl = (dirlist_t*)di->di.nxt;

	if (sfv && dl) {

		tmp = find_missing(dl, sfv);

		if (str_count(tmp) == 0) {

			sprintf(buf, "%d x %.0fM");
		} else {
			st = sfv_list_count(sfv);
			sc = st - str_count(tmp);
			
			show_make_percent(sc, st, 8, '#', '-', perbuf);

			sprintf(buf, "[%s] %d/%dx%.0f",
					perbuf,
					sc,
					st,
					(float)dirlist_get_filesize(dl)/(1000*1000));
		}
	} else if (di->di.files > 0) {
		sprintf(buf, "%dF %.0fM", di->di.files, (float)di->di.bytes/(1024*1024));
	} else {
		strcpy(buf, "-EMPTY-");
	}



}


void show_add_bytesum(hashtable_t *ht, dirlist_t *dl) {
	dirlist_item_t *ti;
	bytesum_t *tmp;
	pwdfile *pwd;

	dirlist_reset(dl);
	while (dirlist_hasnext(dl)) {
		ti = dirlist_next(dl);

		if (!strncmp(ti->file, ".", 1) || !S_ISREG(ti->st.st_mode))
			continue;

		// find pwdentry with username of the file.
		pwd = pwd_getpwuid(ti->st.st_uid);

		if (!pwd)
			continue;

		tmp = (bytesum_t*)ht_get(ht, pwd->name);

		// create new item.
		if (!tmp) {
			tmp = (bytesum_t*)malloc(sizeof(bytesum_t));
			tmp->bytes = 0;
			tmp->files = 0;
			tmp->user = strdup(pwd->name);
			ht_put_obj(ht, pwd->name, tmp);
		}

		// update.
		tmp->bytes += ti->st.st_size;
		tmp->files++;
	}
}


int bytesum_sort(void *a, void *b) {
	bytesum_t *p, *q;

	p = (bytesum_t*)a;
	q = (bytesum_t*)b;

	if (p->bytes > q->bytes)
		return 1;

	return 0;
}

void show_get_leading(release_t *rel, char *winner) {
	sortedlist_t sl;
	hashtable_t ht;
	bytesum_t *tmp;
	di_t *di;

	ht_init(&ht);

	strcpy(winner, "-NONE-");

	if (rel->maindir.di.nxt != 0)
		show_add_bytesum(&ht, (dirlist_t*)rel->maindir.di.nxt);

	for (ht_reset(&rel->subdirs); ht_hasnext(&rel->subdirs); ) {

		di = (di_t*)ht_next(&rel->subdirs)->value;

		if (di->di.nxt != 0)
			show_add_bytesum(&ht, (dirlist_t*)di->di.nxt);
	}

	sortedlist_init(&sl);
	for (ht_reset(&ht); ht_hasnext(&ht); )
		sortedlist_add(&sl, ht_next(&ht)->value);

	sortedlist_sort(&sl, bytesum_sort);

	sortedlist_reset(&sl);

	if (sortedlist_hasnext(&sl))
		strcpy(winner, ((bytesum_t*)sortedlist_next(&sl))->user);
}

strlist_t * show_rel_complete(strlist_t *out, release_t *rel, char *startdir) {
	char agebuf[100];
	char statbuf[100];
	char leaderbuf[100];
	char extras[100];
	char dirbuf[300];
	char linebuf[300];

	extras[0] = 0;


	date_makeage_delim(rel->time, NOW, agebuf, "");
	show_get_complete_status(rel, statbuf, extras);
	show_get_leading(rel, leaderbuf);

	sprintf(dirbuf, "%s%s%s", extras[0] == 0 ? "" : extras,
			extras[0] == 0 ? "" : "/",
			rel->rel + strlen(startdir) + 1);

	sprintf(linebuf, "<%2d>%7.7s %-9.9s%10.10s %-43.43s\n",
			rel->maindir.pos,
			agebuf,
			statbuf,
			leaderbuf,
			dirbuf);

	return str_add_last(out, linebuf);
}


strlist_t * show_rel_incomplete(strlist_t *out, release_t *rel, char *startdir) {
	int subdirs = 0;
	char extras[300], esubdirs[300], mainstat[300], outtmp[1024], *sd, dirbuf[300];
	pwdfile *pwd;
	di_t *di;
	hashtable_item_t *hti;

	subdirs = show_get_cds(rel, esubdirs, extras);

	// race header/
	out = str_add_last(out, "-----------------------------------------------------------------/RACE/----\n");

	pwd = pwd_getpwuid(rel->maindir.di.uploader);

	if (subdirs > 0)
		sprintf(mainstat, "%d CDs, Init: %s", subdirs,
				(pwd ? pwd->name : "-NONE-"));
	else
		show_get_incomplete_status(&rel->maindir, mainstat);

	date_makeage_delim(rel->time, NOW, esubdirs, "");

	sprintf(dirbuf, "%s%s%s",
			extras[0]==0?"":extras,
			extras[0]==0?"":"/",
			rel->rel + strlen(startdir) + 1);

	sprintf(outtmp, ">%2.2d>>%6.6s %-19.19s %-43.43s\n",
			rel->maindir.pos,
			esubdirs,
			mainstat,
			dirbuf);

	out = str_add_last(out, outtmp);

	for (ht_reset(&rel->subdirs); ht_hasnext(&rel->subdirs); ) {

		hti = ht_next(&rel->subdirs);
		di = (di_t*)hti->value;
		sd = hti->key;

		date_makeage_delim(di->di.uptime, NOW, esubdirs, "");
		show_get_incomplete_status(di, mainstat);

		sprintf(dirbuf, " `-> %s, Lead: sorend\n", sd);

		sprintf(outtmp, "  %2.2d  %6.6s %-19.19s %-43.43s\n",
				di->pos,
				esubdirs,
				mainstat,
				dirbuf);

		out = str_add_last(out, outtmp);

	}


	out = str_add_last(out, "---------------------------------------------------------------------------\n");

	return out;
}


strlist_t * show_rel(strlist_t *out, release_t *rel, char *startdir) {
	int complete = 1, rc;
	dirlist_t *dl;

	rc = is_complete(&rel->maindir);

	if (rc == 0)
		complete = 0;

	ht_reset(&rel->subdirs);
	while (complete && ht_hasnext(&rel->subdirs)) {
		rc = is_complete((di_t*)ht_next(&rel->subdirs)->value);

		if (rc == 0)
			complete = 0;
	}

	// show depending on that.
	if (complete)
		return show_rel_complete(out, rel, startdir);
	else
		return show_rel_incomplete(out, rel, startdir);

}

int load_cache(time_t age, char *dir) {
	FILE *f;
	char outfn[1024], tmpdir[1024], *tmp;
	time_t tmpage;

	strcpy(tmpdir, dir);
	for (tmp = tmpdir; *tmp; tmp++)
		if (*tmp == '/') *tmp = '_';

	sprintf(outfn, "%s/%s", CACHEDIR, tmpdir);

	f = fopen(outfn, "r");

	if (!f)
		return 0;

	// coldnt read the line.
	if (fgets(tmpdir, 1024, f) == 0) {
		fclose(f);
		return 0;
	}

	// couldnt parse the line
	if (sscanf(tmpdir, "%lu", &tmpage) < 1) {
		fclose(f);
		return 0;
	}

	// old.
	if (age > tmpage) {
		fclose(f);
		return 0;
	}

	// all good, display.
	while (fgets(tmpdir, 1024, f) > 0)
		printf(tmpdir);

	fclose(f);

	return 1;
}


void save_cache(time_t age, char *dir, strlist_t *out) {
	FILE *f;
	char outfn[1024], tmpdir[1024], *tmp;
	strlist_t *tmpout;

	/*
	strcpy(tmpdir, dir);
	for (tmp = tmpdir; *tmp; tmp++)
		if (*tmp == '/') *tmp = '_';

	sprintf(outfn, "%s/%s", CACHEDIR, tmpdir);

	f = fopen(outfn, "w");

	if (!f)
		return;

	fprintf(f, "%lu\n", age);
	*/

	for (tmpout = out; tmpout; tmpout = tmpout->next) {
		//fprintf(f, tmpout->data);
		printf(tmpout->data);
	}

	//fclose(f);
}


int main(int argc, char *argv[]) {

	dirlog_t dl;
	sortedlist_t sl;
	release_t *rel;
	int i = 0, num = 10;
	strlist_t *out = 0;
	char pwd[1024];

	NOW = time(0);
	getcwd(pwd, 1024);

	if (!dirlog_init(&dl, DIRLOGFILE)) {

		printf("dirlog_init failed, something is not right.\n");
		return 0;
	}

	if (load_cache(dirlog_getage(&dl), pwd))
		return 0;

	sortedlist_init(&sl);

	dirlog_wrap_getnew(&dl, &sl, num, pwd);

	dirlog_finalize(&dl);

	sortedlist_reset(&sl);

	while (i < num && sortedlist_hasnext(&sl)) {
		rel = (release_t*)sortedlist_next(&sl);

		out = show_rel(out, rel, pwd);
		i++;
	}

	save_cache(dirlog_getage(&dl), pwd, out);

	return 0;
}
