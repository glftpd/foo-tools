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

#ifndef _frame_h
#define _frame_h

// config file
#define CONFIGFILE "/etc/foo-checker.conf"

// what the frame binary must be linked to in order to work.
#define BINARY_ZIPSCRIPT "foo-zipscript"
#define BINARY_DUPESCRIPT "foo-dupescript"

#define PROPERTY_ERRORMSG "error_msg"
#define UNKNOWNERROR "Unknown error"

// max filename length to use.
#define MAX_FILELEN 256

#define PROPERTY_DUPE_TMPL_ERROR "dupe_errormsg"
#define PROPERTY_TMPL_ERRORMSG "check_errormsg"

#define PROPERTY_PATH "path"
#define PROPERTY_USERNAME "user"
#define PROPERTY_GROUP "group"
#define PROPERTY_FILE "file"
#define PROPERTY_DIR "dir"
#define PROPERTY_TIME "time"

// holds textual release name, eg Release-Grp/Cd2
#define PROPERTY_RELEASE_NAME "release"

// holds dir or release's main dir, eg. /site/Games/Release-Grp
#define PROPERTY_RELEASE_MAIN "releasemain"


#define PROPERTY_CHECK_FOOTER "check_footer"
#define PROPERTY_CHECK_HEADER "check_header"


#endif
