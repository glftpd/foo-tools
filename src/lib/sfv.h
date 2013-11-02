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
/**
 * Library to handle misc. sfv tasks. As of now only handles calculating sfv-checksum of
 * a file, but should be extended to atleast read filelist from .sfv files.
 *
 **
 * $Id: sfv.h,v 1.2 2003/01/22 14:31:29 sorend Exp $
 * $Source: /home/cvs/footools/footools/src/lib/sfv.h,v $
 * Author: Flower (c) Tanesha Team
 */

#ifndef _sfv_h
#define _sfv_h

#include <stdio.h>
#include <stdlib.h>

/*
 * Structure holding info about a sfv-file.
 */
struct sfv_list {
	char *filename;
    unsigned long crc;

    struct sfv_list *next;
} sfv_list;

typedef struct sfv_list sfv_list_t;

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
sfv_list_t *sfv_list_load_path(char *path);

/*
 * Load a specific sfv file.
 */
sfv_list_t *sfv_list_load(char *sfvfile);

/*
 * Unload (free memory) of a loaded sfv-file.
 */
void sfv_list_unload(sfv_list_t *l);

/*
 * Count the number of files in the list of files in a sfv.
 */
int sfv_list_count(sfv_list_t *l);

/*
 * Check if a specific file is in the list of sfv-files.
 */
sfv_list_t *sfv_list_find(sfv_list_t *l, char *f);

/*
 * Converts a hexstr to a unsigned long.
 */
unsigned long sfv_hexstr_to_long(char *s, unsigned long *c);

#endif

