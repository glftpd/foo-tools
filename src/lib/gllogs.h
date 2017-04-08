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
 * Library to handle adding/updating some glftpd logfiles.
 *
 * $Id: gllogs.h 70 2004-09-05 08:09:33Z sorend $
 * Maintained by: Flower
 */

#include <fcntl.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>

typedef unsigned short int ushort;

// define some locations for where to find glftpd logs.
#define DUPEFILE "/ftp-data/logs/dupefile"
#define DUPELOG "/ftp-data/logs/dupelog"
#define DIRLOG "/ftp-data/logs/dirlog"
#define GLFTPDLOG "/ftp-data/logs/glftpd.log"
#define GLMSGPATH "/ftp-data/msgs"

/*
 * Adds an entry to the dupefile log.
 */
int gl_dupefile_add(char *fn, char *u);

/*
 * Adds an entry to dirlog (faster than dirlog, but only use if no entry in dirlog already).
 */
int gl_dirlog_add(char *dn, ushort uid, ushort gid, ushort files, long bytes);

/*
 * Update an entry in the dirlog (adds if it doesnt exist already).
 *
 * Note: status == -1 -> leave status flag untouched.
 *
 * Example: gl_dirlog_update("/site/games/Hello-OKE", 10, 100, files+1, bytes+100, -1);
 */
int gl_dirlog_update(char *dn, ushort uid, ushort gid, ushort files, long bytes, int status);

/*
 * Adds an entry to the dupelog.
 *
 * Example: gl_dupelog_add("Supa.Release.v1.0-Hello");
 */
int gl_dupelog_add(char *rel);

/*
 * Put something in glftpd.log
 *
 * Example: gl_gllog_add("My program is still alive");
 */
int gl_gllog_add(char *str);

// alternative gllog add with logfile specified.
int gl_gllog_add_alt(char *str, char *logfile);

/*
 * Special method to add a string to be announced.
 *
 * Example: gl_gllog_announce("CUSTOM", "Hello world");
 */
int gl_gllog_announce(char *type, char *str);

/*
 * Send a sitemsg to another user.
 */
int gl_site_msg(char *from, char *to, char *msg);
