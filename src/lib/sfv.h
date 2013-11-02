/**
 * Library to handle misc. sfv tasks. As of now only handles calculating sfv-checksum of
 * a file, but should be extended to atleast read filelist from .sfv files.
 *
 **
 * $Id: sfv.h,v 1.2 2001/10/03 19:44:28 sd Exp $
 * $Source: /var/cvs/foo/src/lib/sfv.h,v $
 * Author: Flower (c) Tanesha Team
 */

#ifndef _sfv_h
#define _sfv_h

#include <stdio.h>

/*
 * Structure holding info about a sfv-file.
 */
struct _sfv_list {
    char filename[256];
    unsigned long crc;

    struct _sfv_list *next;
} _sfv_list;

typedef struct _sfv_list sfv_list;

/*
 * Slower but less mem-consuming method to calc crc of a file.
 */
int sfv_calc_crc32( char *fname, unsigned long *crc );

/*
 * Faster and more memory consuming method to calc crc of a file.
 */
int sfv_mmap_calc_crc32(char *fname, unsigned long *crc);

/*
 * Find sfv file in a dir, and load it.
 */
sfv_list *sfv_list_load_path(char *path);

/*
 * Load a specific sfv file.
 */
sfv_list *sfv_list_load(char *sfvfile);

/*
 * Unload (free memory) of a loaded sfv-file.
 */
void sfv_list_unload(sfv_list *l);

/*
 * Count the number of files in the list of files in a sfv.
 */
int sfv_list_count(sfv_list *l);

/*
 * Check if a specific file is in the list of sfv-files.
 */
sfv_list *sfv_list_find(sfv_list *l, char *f);

/*
 * Converts a hexstr to a unsigned long.
 */
unsigned long sfv_hexstr_to_long(char *s, unsigned long *c);

#endif

