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
 * Module that runs pzs-ng's audiosort on pre release
 * Author, slv.
 * $Id: mod_audiosort.c,v 1.1 2018/06/04 09:00:00 slv Exp $
 */

/* TODO:
- use mod_func_rel instead of dir ?
- test and verify section env var is working with ht_get + real pre's
- make sure * is checked, no segfaults
- cleanup;)
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// project includes
#include "mod_audiosort.h"
#include "../foo-pre.h"

// footools includes
#include <collection/hashtable.h>

// in foo-pre
hashtable_t *_mod_audiosort_cfg = 0;
hashtable_t *_mod_audiosort_env = 0;

void set_config(hashtable_t *cfg) {
	_mod_audiosort_cfg = cfg;
}
void set_env(hashtable_t *env) {
        _mod_audiosort_env = env;
}
/*
hashtable_t *get_config() {
	return _mod_audiosort_cfg;
*/
hashtable_t *get_env() {
        return _mod_audiosort_env;;
}

hashtable_t *cfg = 0;
hashtable_t *get_config() {
        return cfg;
}

// prototype for file handling function.
//int mod_audiosort_file_func(char *filepath, char *argv[]);
int mod_audiosort_dir_func(char *dir, char *argv[]);

module_list_t mod_audiosort_info = {
	// module name
	"audiosorter",

	// module dir func
	mod_audiosort_dir_func,

	// module file func
	//mod_audiosort_file_func,

	// module rel func
	0,

	// struct module_list entry
	0
};




// function to return module info of this module.
module_list_t *module_loader() {
	return &mod_audiosort_info;
}

//int _mod_chmod_dchmod = -1;
//int _mod_audiosort_bin = -1;
//char _mod_audiosort_bin;

/*
int _mod_chmod_getdchmod() {
  char *tmp;

  if (_mod_chmod_dchmod == -1) {

    tmp = ht_get(get_config(), PROPERTY_MOD_CHMOD_DIRS);
    if (tmp)
      _mod_chmod_dchmod = atoi(tmp);

  }

  return _mod_chmod_dchmod;
}
int _mod_audiosort_getbin() {
  char *tmp;

  if (_mod_audiosort_bin == -1) {

    tmp = ht_get(get_config(), PROPERTY_MOD_AUDIOSORT_BIN);
    if (tmp)
      _mod_audiosort_bin = atoi(tmp);

  }

  return _mod_audiosort_bin;
}
char _mod_audiosort_getbin() {
  char *tmp;
  hashtable_t *cfg;
  tmp = ht_get(cfg, PROPERTY_MOD_AUDIOSORT_BIN);
  return *tmp;
}
*/
char * section_get_property(char *sec, char *prop) {
        hashtable_t *cfg = get_config();
        char buf[300];

        sprintf(buf, "section.%s.%s", sec, prop);

        return ht_get(cfg, buf);
}
int section_get_int_property(char *s, char *p) {
        char *tmp;

        tmp = section_get_property(s, p);

        if (!tmp)
                return -1;

        if (!strcasecmp(tmp, "none"))
                return -1;

        return atoi(tmp);
}
int pre_replace(char *b, char *n, char *r) {
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


int mod_audiosort_dir_func(char *dir, char *argv[]) {
  char buf[1024], *tmp, *audiosort_bin, *section, *s_path;
  FILE *f;
/*
//  hashtable_t *cfg = get_config();
//  hashtable_t *env = get_env();

        hashtable_t *_envctx = 0;

        hashtable_t * get_context() {
                if (!_envctx) {
                        _envctx = malloc(sizeof(hashtable_t));
                       ht_init(_envctx);
                }

                return _envctx;
        }

//        hashtable_t *cfg = get_context();
        hashtable_t *env = get_context();
*/

cfg = malloc(sizeof(hashtable_t));
ht_init(cfg);
ht_load(cfg, "testpre.cfg");

//  section = ht_get(get_env(), "section");
//  section = "mp3";
//  section = ht_get(env,"section");
  printf("section: %s\n", getenv("section"));
  section = getenv("section");
  printf("DEBUG: dir: %s section: %s\n", dir, section);

  // get the dir of the section.
  sprintf(buf, "section.%s.%s", section, PROPERTY_SECTION_DIR);
  printf("DEBUG: buf: %s\n", buf);

  s_path = ht_get(cfg, buf);
/*
  s_path = "/jail/glftpd/site/incoming/mp3/0604";
  int i = section_get_int_property(section, PROPERTY_SECTION_DIR);
  printf("DEBUG: section get int prop: %s\n", i);
  s_path = section_get_property(section, PROPERTY_SECTION_DIR);
*/
  // return if missing configuration.
  if (!s_path) {
    printf("DEBUG: missing cfg, s_path: %s\n", s_path);
    return 1;
  }

  printf("DEBUG: s_path: %s\n", s_path);
//  exit(0);

  // get the rlsname
  tmp = strrchr(dir, '/');
  if (!tmp)
      return 1;
  tmp++;
  printf("DEBUG: tmp: %s\n", tmp);

  audiosort_bin = ht_get(cfg, PROPERTY_MOD_AUDIOSORT_BIN);

  if (!audiosort_bin)
    audiosort_bin = "/bin/audiosort";
  
  printf("DEBUG: audiosort_bin: %s\n", audiosort_bin);
  f = fopen(audiosort_bin, "r");
  if (!f)
    return 1;

  //int mode;

  //mode = _mod_chmod_getdchmod();

  //if (mode == -1)
  //  return 0;

  //if (chmod(dir, mode) == -1)
  //  printf("chmod %s failed!\n", dir);
  //printf("DEBUG: dir: %s argv0 %s argv1: %s argv2: %s argv3: %s\n", dir, argv[0], argv[1], argv[2], argv[3]);
  printf("DEBUG: dir: %s argv0 %s argv1: %s argv2: %s section: %s s_path: %s\n", dir, argv[0], argv[1], argv[2], section, s_path);

        time_t now;
        struct tm *tm_now;

        now = time(0);
        tm_now = localtime(&now);

        strftime(buf, 1024, "%d", tm_now);
        pre_replace(s_path, "DD", buf);

        strftime(buf, 1024, "%m", tm_now);
        pre_replace(s_path, "MM", buf);

        strftime(buf, 1024, "%Y", tm_now);
        pre_replace(s_path, "YYYY", buf);

        strftime(buf, 1024, "%y", tm_now);
        pre_replace(s_path, "YY", buf);

        strftime(buf, 1024, "%w", tm_now);
        pre_replace(s_path, "WW", buf);

        strftime(buf, 1024, "%W", tm_now);
        pre_replace(s_path, "WOY", buf);

  sprintf(buf, "/bin/audiosort %s/%s", s_path, argv[1]);
  if (system(buf) == -1)
    printf("audiosorting %s failed!\n", dir);

  return 1;
}

/*
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
*/

