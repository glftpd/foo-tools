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
 * Module to extract mp3info from a pred release
 * Author, Soren.
 * $Id: mod_chmod.c,v 1.1 2003/08/15 08:09:38 sorend Exp $
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// project includes
#include "mod_chmod.h"
#include "../foo-pre.h"

// footools includes
#include <collection/hashtable.h>

// in foo-pre
hashtable_t *_mod_chmod_cfg = 0;

void set_config(hashtable_t *cfg) {
	_mod_chmod_cfg = cfg;
}
hashtable_t *get_config() {
	return _mod_chmod_cfg;
}

// prototype for file handling function.
int mod_chmod_file_func(char *filepath, char *argv[]);
int mod_chmod_dir_func(char *dir, char *argv[]);

module_list_t mod_chmod_info = {
	// module name
	"file chmodder",

	// module dir func
	mod_chmod_dir_func,

	// module file func
	mod_chmod_file_func,

	// module rel func
	0,

	// struct module_list entry
	0
};




// function to return module info of this module.
module_list_t *module_loader() {
	return &mod_chmod_info;
}

int _mod_chmod_dchmod = -1;
int _mod_chmod_fchmod = -1;

int _mod_chmod_getdchmod() {
  char *tmp;

  if (_mod_chmod_dchmod == -1) {

    tmp = ht_get(get_config(), PROPERTY_MOD_CHMOD_DIRS);
    if (tmp)
      _mod_chmod_dchmod = atoi(tmp);

  }

  return _mod_chmod_dchmod;
}


int _mod_chmod_getfchmod() {
  char *tmp;

  if (_mod_chmod_fchmod == -1) {

    tmp = ht_get(get_config(), PROPERTY_MOD_CHMOD_FILES);
    if (tmp)
      _mod_chmod_fchmod = atoi(tmp);

  }

  return _mod_chmod_fchmod;
}


int mod_chmod_dir_func(char *dir, char *argv[]) {

  int mode;

  mode = _mod_chmod_getdchmod();

  if (mode == -1)
    return 0;

  if (chmod(dir, mode) == -1)
    printf("chmod %s failed!\n", dir);

  return 1;
}

// file func.
int mod_chmod_file_func(char *filepath, char *argv[]) {

  hashtable_t *cfg;
  int mode;

  mode = _mod_chmod_getfchmod();

  if (mode == -1)
    return 0;

  if (chmod(filepath, mode) == -1)
    printf("chmod %s failed!\n", filepath);

  return 1;
}


