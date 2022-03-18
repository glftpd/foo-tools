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
 * $Source: /home/cvs/footools/footools/src/lib/common.c,v $
 * Author: Soren
 *
 * This module should be outphased.
 */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include "common.h"


char *lower(char *s) {
	int i;

	for (i=0;i<strlen(s);i++)
		s[i]=tolower(s[i]);

	return s;
}

char *fgetsnolfs(char *buf, int n, FILE *fh) {
        char *in;
        char *tmp;

        in=fgets(buf,n,fh);
        if (!in) return 0;
        tmp=buf;
        while (*tmp) {
                if ((*tmp=='\n')||(*tmp=='\r')) {
                        *tmp=0;
                        break;
                }
                tmp++;
        }
        return in;
}

int fileexists(char *f) {
	struct stat st;

	return (stat(f,&st)!=-1);
}

int replace(char *b, char *n, char *r) {
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

int ishiddendir(char *p) {
	int hidden=0;
	FILE *hf;
	char tmpfile[300];

	// check if the dir is in the list of hiddendirs.
	if (hf=fopen(HIDDENDIRFILE,"r")) {
		while (fgetsnolfs(tmpfile,300,hf))
			if (tmpfile[0] &&
			    !strncasecmp(tmpfile, p, strlen(tmpfile)))
				hidden=1;
		fclose(hf);
	}

	return hidden;
}

int get_dirs(char *d, char *p, char *r) {
	char *t, *t2;
	char buf[300];

	strcpy(buf, d);
	t=strrchr(buf, '/');
	if (!t) {
		strcpy(r, buf);
		strcpy(p, "??");
		return 0;
	} else
		t++;

	if (!strncasecmp(t, "cd", 2) ||
		 !strncasecmp(t, "disc", 4) ||
		 !strncasecmp(t, "sample", 6) ||
		 !strncasecmp(t, "cover", 5)) {

		*(t-1)=0;
		t2=strrchr(buf, '/')+1;
		*(t-1)='/';
		t=t2;
	}

	*(t-1)=0;
	t2=strrchr(buf, '/')+1;

	strcpy(p, t2);
	strcpy(r, t);

	return 1;
}



int common_make_percent(int ok, int total, int width, char uncheck, char check, char *out) {
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

char *readfile(char *fn) {
	FILE *f;
	char buf[1024], nbuf[10240], *b;
	long s;

	f = fopen(fn, "r");
	if (!f)
		return NULL;

	nbuf[0] = 0;
	while (fgetsnolfs(buf, 1024, f)) {
		strcat(nbuf, buf);
		strcat(nbuf, "\n");
	}
	fclose(f);

	b = (char*)malloc(strlen(nbuf)+1);
	strcpy(b, nbuf);

	return b;
}

/*
int touch(char *fname, int force) {
	int fd, needed_chmod, rval;
	unsigned char byte;
	struct stat st;

	if (stat(fname, &st) == -1)
		return 0;

	// Try regular files and directories.
	if (!S_ISREG(st.st_mode) && !S_ISDIR(st.st_mode))
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
*/


char *trim(char *s) {
	char *begin, *tmp;

	begin = s;

	while ((*begin == ' ') || (*begin == '\t') || (*begin == '\n') || (*begin == '\r'))
		begin++;

	tmp = begin;

	while (*tmp)
		tmp++;

	if (tmp == begin)
		return begin;

	tmp--;

	while ((tmp > begin) && ((*tmp == ' ') || (*tmp == '\t') || (*tmp == '\n') || (*tmp == '\r')))
		tmp--;

	*(tmp+1) = 0;

	return begin;
}



int common_copy(char *src, char *dest) {
	FILE *sf, *df;
	char buf[MAX_BUFSIZE];
	int len, ret = 1;

	sf = fopen(src, "rb");
	
	if (!sf)
		return 0;

	df = fopen(dest, "wb");

	if (!df) {
		fclose(sf);

		return 0;
	}

	while ((len = fread(&buf, 1, MAX_BUFSIZE, sf)) > 0) {

		if (fwrite(&buf, 1, len, df) != len) {
			ret = 0;

			break;
		}
	}

	fclose(sf);
	fclose(df);

	if (!ret)
		unlink(dest);

	return ret;
}

