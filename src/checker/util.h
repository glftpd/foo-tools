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

#ifndef _util_h
#define _util_h

#include <collection/hashtable.h>
#include <lib/sfv.h>
#include <lib/dirlist.h>
#include <lib/stringtokenizer.h>


#define PROPERTY_SFVLIST "tmp_sfv"
#define PROPERTY_DIRLIST "tmp_dir"

#define PROPERTY_DEFAULT_REPLACER_ENV "frame_replacer"

sfv_list_t * util_get_sfvlist(hashtable_t *conf, char *dir);

dirlist_t * util_get_dirlist(hashtable_t *conf, char *dir);

char * util_replacer_env(hashtable_t *env, char *buf);

char * util_replacer(hashtable_t *conf, char *buf);

void util_replacer_set(hashtable_t *conf, char *key, char *val);

void error_printf(hashtable_t *conf, char *buf, char *err);

void msg_printf(hashtable_t *conf, char *buf, char *msg, char *status);

void util_printf(hashtable_t *conf, char *buf);

int util_path_match(char *p, char *dir);

int util_path_stmatch(stringtokenizer *s, char *dir);


#endif
