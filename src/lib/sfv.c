/**
 * Library to handle misc. sfv tasks. As of now only handles calculating sfv-checksum of
 * a file, but should be extended to atleast read filelist from .sfv files.
 *
 * Dependencies:
 *   + common.o
 *
 * Changelog:
 *   + Added routines to read/parse sfv files. These are currently untested !
 *
 **
 * $Id: sfv.c,v 1.3 2001/10/30 14:41:06 sd Exp $
 * $Source: /var/cvs/foo/src/lib/sfv.c,v $
 * Author: Soren
 */
#include "sfv.h"
#include "common.h"
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <glob.h>
#include "sfv_table.h"

int sfv_calc_crc32( char *fname, unsigned long *crc ) {
	FILE *in;           /* input file */

	unsigned char *buf; /* pointer to the input buffer */

	size_t i, j;        /* buffer positions*/
	int k, t, tt;       /* generic integer */

	/* open file */
	if((in = fopen(fname, "rb")) == NULL) {
		printf("Can't open file %s\n", fname);
		return 1;
	}
	/* allocate buffer */
	if((buf = (unsigned char *) malloc(32766)) == NULL) {
		printf("Can't allocate memory!\n");
		fclose(in);
		return 1;
	}
	*crc = 0xFFFFFFFF; /* preconditioning sets non zero value */
	/* loop through the file and calculate CRC */
	while(1){
		i=fread(buf, 1, 32766, in);
		if(i==0) break;
		for(j=0; j<i; j++){
			k=(*crc ^ buf[j]) & 0x000000FFL;
			*crc=((*crc >> 8) & 0x00FFFFFFL) ^ crc32_table[k];
		}
	}
	*crc=~(*crc); /* postconditioning */
	fclose(in);

    return 0;
}

int sfv_calc_buf(char *buf, unsigned long *crc, int len, int init) {
	int i;

	if (init)
		*crc = 0xffffffffL;

	for (i = 0; i < len; i++)
		*crc = ((*crc >> 8) & 0x00ffffffL)^crc32_table[(*crc & 0xffL) ^ buf[i]];

	return 1;
}

int sfv_mmap_calc_crc32(char *fname, unsigned long *crc) {
    FILE *in;
    unsigned char *fmap;
    struct stat st;
    int j, k;

    in = fopen(fname, "rb");
    if (!in)
        return 1;
    fstat(fileno(in), &st);

    *crc = 0xffffffffL;
    fmap = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fileno(in), 0);
    fclose(in);
   
    for (j = 0; j < st.st_size; j++)
       *crc = ((*crc >> 8) & 0x00ffffffL)^ crc32_table[(*crc & 0xffL) ^ fmap[j]];

    *crc =~ (*crc);
    munmap(fmap, st.st_size);
   
    return 0;
}

unsigned long sfv_hexstr_to_long(char *s, unsigned long *c) {
	unsigned long l;

	sscanf(s, "%lx", &l);

	if (c)
		*c = l;

	return l;
}

sfv_list *sfv_list_add(sfv_list *l, char *s) {
    sfv_list *t = NULL;
    char buf[100], *tmp;
    unsigned long crc = 0;

    strcpy(buf, strchr(s, ' ') + 1);
	sfv_hexstr_to_long(buf, &crc);

    strcpy(buf, s);
    tmp = strchr(buf, ' ');
    *tmp = 0;

    tmp = strrchr(buf, '/');
    if (tmp)
	strcpy(buf, tmp + 1);

    // add file if we have what we need.
    if ((buf[0] != 0) && (crc > 0)) {
	t = (sfv_list *)malloc(sizeof(sfv_list));

	strcpy(t->filename, lower(buf));
	t->crc = crc;

	t->next = l;

	return t;
    }

    return l;
}

sfv_list *sfv_list_load_path(char *path) {
    glob_t glbuf;
    char buf[300];

    sprintf(buf, "%s/*.sfv", path);

    glob(buf, 0, 0, &glbuf);

    if (!glbuf.gl_pathc)
	return NULL;

    return sfv_list_load(glbuf.gl_pathv[0]);
}

sfv_list *sfv_list_load(char *sfvfile) {
    FILE *sfv;
    char buf[300],*t;
    sfv_list *l = NULL;
    
    sfv = fopen(sfvfile, "r");

    if (!sfv)
	return NULL;
    
    while (fgetsnolfs(buf, 300, sfv))
	if ((buf[0] != ';') && (strchr(buf, ' ')))
	    l = sfv_list_add(l, buf);

    fclose(sfv);
    
    return l;
}

sfv_list *sfv_list_find(sfv_list *l, char *f) {
    sfv_list *t;

    if (!f)
	return NULL;

    for (t = l; t; t = t->next)
	if (!strcmp(lower(f), t->filename))
	    break;

    return t;
}

int sfv_list_count(sfv_list *l) {
    if (l)
	return 1 + sfv_list_count(l->next);
    else
	return 0;
}

void sfv_list_unload(sfv_list *l) {
    sfv_list *t;

    while (l) {
	t = l;
	l = l->next;
	free(t);
    }
}
