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
 * $Id: mod_symlink.c,v 1.10 2004/04/13 21:03:53 sorend Exp $
 */

#include <string.h>
#include <stdio.h>
// project includes
#include "mod_symlink.h"
#include "../foo-pre.h"

// footools includes
#include <collection/hashtable.h>
#include <lib/stringtokenizer.h>

// in foo-pre
hashtable_t *_mod_symlink_cfg = 0;
hashtable_t *_mod_symlink_env = 0;

void set_config(hashtable_t *cfg) {
	_mod_symlink_cfg = cfg;
}
hashtable_t *get_config() {
	return _mod_symlink_cfg;
}

void set_env(hashtable_t *env) {
	_mod_symlink_env = env;
}
hashtable_t *get_env() {
	return _mod_symlink_env;
}

// prototype for file handling function.
int mod_symlink_rel_func(char *dir, char *argv[]);

module_list_t mod_symlink_info = {
	// module name
	"symlinker",

	// module dir func
	0,

	// module file func
	0,

	// module rel func
	mod_symlink_rel_func,

	// struct module_list entry
	0
};




// function to return module info of this module.
module_list_t *module_loader() {
	return &mod_symlink_info;
}

// dir func
int mod_symlink_rel_func(char *dir, char *argv[]) {

  int i;
  //char buflink[1024], buf[1024], bufto[1024], *tmp, *section, *s_linkpath, *s_path;
  char buflink, buf[1024], bufto[1024], *tmp, *section, *s_linkpath, *s_path;

  // get section from context (undocumented feature :-).
  section = ht_get(get_env(), "section");

  // get the symlink setting from the config.
  sprintf(buf, "section.%s.%s", section, PROPERTY_MOD_SYMLINK_SECTION_PROP);
  s_linkpath = ht_get(get_config(), buf);

  // get the dir of the section.
  sprintf(buf, "section.%s.%s", section, "dir");
  s_path = ht_get(get_config(), buf);

  // return if missing configuration.
  if (!s_linkpath || !s_path)
	  return 1;

  // get the rlsname
  tmp = strrchr(dir, '/');
  if (!tmp)
	  return 1;
  tmp++;

  i = _symlink_find_relative(s_path, s_linkpath, &buflink);

  if (!i)
	  return 1;

  sprintf(buf, "%s/%s", s_linkpath, argv[1]);
  sprintf(bufto, "%s/%s", buflink, argv[1]);

  i = symlink(bufto, buf);

  printf(" created link -> %s\n  < from > %s\n  < to > %s\n", (i == -1 ? "Error" : "Ok"), buf, bufto);

  return 1;
}


int _symlink_find_relative(char *from, char *to, char *relative) {
	char *site_dir;
	int back = 0, i;
	stringtokenizer st;

	/*
	site_dir = ht_get(get_config(), "sitedir");

	if (!site_dir)
		return 0;

	from += strlen(site_dir);
	*/

	strcpy(relative, "");

	// skip the 'equal'.
	while (*from && *to) {
		if (*from != *to)
			break;

		from++;
		to++;
	}

	st_initialize(&st, to, "/");

	// reset relative.
	relative[0] = 0;

	// append back dirs.
	for (i = 0; i < st_count(&st); i++)
		strcat(relative, "../");

	// append forward dirs.
	strcat(relative, from);

	return 1;
}
/*


/site/Games/The.Game-FLT

/site/pre/The.Game-FLT

/site/
Games/The.Game-FLT
pre/The.Game-FLT

../pre


/site/Games/The.Game-FLT
/site/pre/weekly/The.Game-FLT

/site/
Games/The.Game-FLT
pre/weekly/The.Game-FLT


/site/The.Game-FLT
/site/pre/weekly/The.Game-FLT

/site/

*/
