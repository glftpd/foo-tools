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
 * Library for reading passwd files.
 */

#ifndef _pwdfile_h
#define _pwdfile_h

/*
 * passwd structure.
 */
struct pwdfile_t {
	char name[30];
	char pass[30];

	int uid;
	int gid;

	char longname[50];
	char homedir[300];
	char shell[300];

	struct pwdfile_t *next;
};

typedef struct pwdfile_t pwdfile;

struct grpfile {
	char group[30];
	char pass[30];

	int gid;

	char *users;

	struct grpfile *next;
};

typedef struct grpfile grpfile_t;

/*
 * Location of passwd file.
 */

// old deprecated
#define PASSWDFILE "/etc/passwd"
#define GROUPFILE "/etc/group"

// new settings
#define DEFAULT_ETCDIR "/etc"
#define DEFAULT_PASSWD "passwd"
#define DEFAULT_GROUP "group"

/*
 * getpwnam method.
 */
pwdfile *pwd_getpwnam(char *u);

/*
 * getpwuid method.
 */
pwdfile *pwd_getpwuid(int uid);


grpfile_t *pwd_getgpnam(char *g);

grpfile_t *pwd_getgpgid(int gid);

void pwd_set_etcdir(char *etcdir);

#endif
