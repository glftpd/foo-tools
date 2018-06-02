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

#ifndef _foopre_h
#define _foopre_h

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

typedef unsigned short int ushort;

#define PRE_CONFIGFILE "/etc/pre.cfg"
#define PRE_LOGFILE "/ftp-data/logs/foo-pre.log"

#define PROPERTY_USER "user"
#define PROPERTY_TAGLINE "tagline"
#define PROPERTY_USERGROUP "usergroup"
#define PROPERTY_GROUP "group"

#define PROPERTY_GL_USERDIR "userdir"
#define PROPERTY_SITEDIR "sitedir"

#define PROPERTY_TEXT_HEAD "text.head"
#define PROPERTY_TEXT_TAIL "text.tail"

#define PROPERTY_GROUP_DIR "dir"
#define PROPERTY_GROUP_ALLOW "allow"

#define PROPERTY_GROUP_GL_CS "gl_credit_section"
#define PROPERTY_GROUP_GL_SS "gl_stat_section"
#define PROPERTY_GROUP_RATIO "ratio"
#define PROPERTY_GROUP_ANNOUNCE "announce"
#define PROPERTY_GROUP_DEFSEC "def_sec"

#define PROPERTY_SECTION_DIR "dir"
#define PROPERTY_SECTION_NAME "name"

#define PROPERTY_GROUP_CHOWN_USER "chown.user"
#define PROPERTY_GROUP_CHOWN_GROUP "chown.group"

#define PROPERTY_COUNTABLE "countable"
#define PROPERTY_CREDITABLE "creditable"
#define PROPERTY_ADDSUB "addsubdirstodirlog"

#define PROPERTY_MOVE_EXTERNAL "move.external"
#define PROPERTY_MOVE_FORCE_EXT "move.force.ext"

#define PROPERTY_ETCDIR "etcdir"

#define PROPERTY_MODULES "modules"

#define MODULE_LOADER_FUNC "module_loader"

#define MODULE_SETCONFIG_FUNC "set_config"
#define MODULE_SETENV_FUNC "set_env"

#define PROPERTY_ADDMP3GENRE "addmp3genretogllog"

/*
 * Holds info about chowns
 */
struct chowninfo {
	int uid;
	int gid;
};

typedef struct chowninfo chowninfo_t;

/*
 * Holds list of relative filenames.
 */
struct filelist {
	char file[300];
	struct stat st;
	char uname[30];

	struct filelist *next;
};

typedef struct filelist filelist_t;

struct creditlist {
	int uid;
	long bytes;
	long files;

	struct creditlist *next;
};

typedef struct creditlist creditlist_t;

struct subdir_list {
	char dir[500];
	ushort files;
	long bytes;

	struct subdir_list *next;
};

typedef struct subdir_list subdir_list_t;

/*
 * Structure for creating modules for foo-pre.
 */
struct module_list {
	/*
	 * the name of the module
	 */
	char *mod_name;

	/*
	 * function to use for each dir of the release.
	 *
	 * dirpath - complete path to the dir.
	 * args - the args from commandline
	 *
	 * NYI!
	 */
	int (*mod_func_dir)(char *dirpath, char *args[]);

	/*
	 * function to use for each file of the release
	 *
	 * filepath - path to the file.
	 * args - the args from commandline
	 */
	int (*mod_func_file)(char *filepath, char *args[]);

	/*
	 * function to use for all of the release
	 *
	 * releasepath - path to the release
	 * args - the args from commandline
	 */
	int (*mod_func_rel)(char *releasepath, char *args[]);

	struct module_list *next;
};

typedef struct module_list module_list_t;

int touch_dir(char *dir);
int touch_file(char *fname);
char *get_mp3_genre(const char* filename);

//TODO: cleanup
//DEBUG:
// not needed?
// char *mp3_genre = "Unknown";
//extern char* mp3_genre;
//extern char mp3_genre[40];


#endif
/* vim: set noai tabstop=8 shiftwidth=8 softtabstop=8 noexpandtab: */
