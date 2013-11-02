/*
 * $Source: /var/cvs/foo/src/lib/common.h,v $
 * Author: Soren
 *
 * This module should be outphased!
 */
#include <stdio.h>

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
