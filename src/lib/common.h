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
 * $Source: /home/cvs/footools/footools/src/lib/common.h,v $
 * Author: Soren
 *
 * This module should be outphased!
 */
#include <stdio.h>
#include <stdlib.h>

#define HIDDENDIRFILE "/ftp-data/misc/f00-hiddendirs.txt"

#define MAX_BUFSIZE 4096

int get_dirs(char *d, char *p, char *r);
int ishiddendir(char *p);
char *lower(char *s);
char *fgetsnolfs(char *buf, int n, FILE *fh);
int fileexists(char *f);
int replace(char *b, char *n, char *r);
char *readfile(char *fn);
char *trim(char *s);

/*
 * Makes a percent bar in out.
 */
int common_make_percent(int ok, int total, int width, char uncheck, char check, char *out);

/*
 * Copies a file.
 */
int common_copy(char *src, char *dest);
