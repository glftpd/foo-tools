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
 * Library to handle adding/updating some glftpd logfiles.
 *
 * $Id: gllogs.c 41 2003-06-23 07:32:29Z sorend $
 * Maintained by: Flower
 */

#include "gllogs.h"
#include "glupdate.c"
#include <stdio.h>
#include <time.h>
#include <sys/file.h>
#include <unistd.h>

int gl_dupefile_add(char *fn, char *u) {
	FILE *f;
	struct dupefile df;

	bzero(&df, sizeof(df));
	strcpy(df.filename, fn);
	strcpy(df.uploader, u);
	df.timeup = (time32_t)time(NULL);
	
	f = fopen(DUPEFILE, "ab");

	if (!f)
		return 0;

	fwrite(&df, sizeof(df), 1, f);
	fclose(f);

	return 1;
}

int gl_dirlog_add(char *dn, ushort uid, ushort gid, ushort files, long bytes) {
    FILE *f;
    struct dirlog dl;
    
    strcpy(dl.dirname, dn);
    dl.uploader = uid;
    dl.group = gid;
    dl.files = files;
    dl.bytes = bytes;
    dl.uptime = (time32_t)time(NULL);
    dl.status = 0;
    
    update_log(dl);
    return 1;
}

int gl_dirlog_update(char *dn, ushort uid, ushort gid, ushort files, long bytes, int status) {
    
    FILE *f;
    struct dirlog dl;
    
    strcpy(dl.dirname, dn);
    dl.uploader = uid;
    dl.group = gid;
    dl.files = files;
    dl.bytes = bytes;
    dl.uptime = (time32_t)time(NULL);
    dl.status = 0;
    
    update_log(dl);
    return 1;
}

/*
int gl_dirlog_add(char *dn, ushort uid, ushort gid, ushort files, long bytes) {
	FILE *f;
	struct dirlog dl;

	bzero(&dl, sizeof(dl));
	strcpy(dl.dirname, dn);
	dl.uploader = (uint16_t)uid;
	dl.group = (uint16_t)gid;
	dl.files = (uint16_t)files;
	dl.bytes = (uint64_t)bytes;
	dl.uptime = (time32_t)time(NULL);
	dl.status = (uint16_t)0;

	f = fopen(DIRLOG, "ab");

	if (!f)
		return 0;

	fwrite(&dl, sizeof(dl), 1, f);
	fclose(f);

	return 1;
}

*/

/*
int gl_dirlog_update(char *dn, ushort uid, ushort gid, ushort files, long bytes, int status) {
	FILE *f;
	struct dirlog dl;
	int found;

	f = fopen(DIRLOG, "rb+");

	if (!f)
		return 0;

#ifndef WIN32
	flock(fileno(f), LOCK_EX);
#endif

	while (fread(&dl, sizeof(dl), 1, f)) {
		if (!strcmp(dl.dirname, dn)) {
			dl.files += files;
			dl.bytes += bytes;
			dl.status = (status == -1) ? dl.status : status;

			fseek(f, 0-sizeof(dl), SEEK_CUR);
			fwrite(&dl, sizeof(dl), 1, f);
			found++;
		}
	}

	if (!found) {
		bzero(&dl, sizeof(dl));
	    dl.uploader = (uint16_t)uid;
        dl.group = (uint16_t)gid;
        dl.files = (uint16_t)files;
    	dl.bytes = (uint64_t)bytes;
		strcpy(dl.dirname, dn);
		dl.status = (status == -1) ? 0 : status;
		dl.uptime = (time32_t)time(NULL);

		fwrite(&dl, sizeof(dl), 1, f);
	}

#ifndef WIN32
	flock(fileno(f), LOCK_UN);
#endif

	fclose(f);

	return 1;
}
*/

int gl_dupelog_add(char *rel) {
	FILE *f;
	char timebuf[50];
	long now;
	struct tm *tm;

	time(&now);
	tm = localtime(&now);
	if (!tm)
		return 0;

	strftime(timebuf, 50, "%m%d%g", tm);

	f = fopen(DUPELOG, "a");
	if (!f)
		return 0;

	fprintf(f, "%s %s\n", timebuf, rel);
	fclose(f);

	return 1;
}

int gl_gllog_add_alt(char *str, char *logfile) {
    FILE *f;
    time_t t;
    char buf[300], *p;

    t = time(0);

    strftime(buf, 300, "%a %b %d %T %Y", localtime(&t));

    f = fopen(logfile, "a");
    if (!f)
		return 0;

    fprintf(f, "%s %s\n", buf, str);
    fclose(f);

    return 1;
}

int gl_gllog_add(char *str) {
	
	return gl_gllog_add_alt(str, GLFTPDLOG);
}

int gl_gllog_announce(char *type, char *str) {
    char buf[1024];

	if (str[0] == '"')
		sprintf(buf, "%s: %s", type, str);
	else
		sprintf(buf, "%s: \"%s\"", type, str);

    return gl_gllog_add(buf);
}

int gl_site_msg(char *from, char *to, char *msg) {
	FILE *f;
	char buf[300], *p;
	long t;

	if (!to)
		return 0;

	sprintf(buf, "%s/%s", GLMSGPATH, to);
	f = fopen(buf, "a");

	if (!f)
		return 0;

	t = time(0);
	sprintf(buf, "%s", ctime(&t));
	p = (char*)&buf;
	while (*p)
		if (*p == '\n')
			*p = 0;
		else
			p++;

	fprintf(f, "\
From: %s (%s)\
--------------------------------------------------------------------------\
\
%s\
", from, buf, msg);

	fclose(f);

	return 1;
}
