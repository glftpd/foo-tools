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


#ifndef _webspy_dirview_h
#define _webspy_dirview_h

#include <collection/hashtable.h>
#include <stdio.h>
#include <sys/types.h>
#include <httpd.h>
#include "webspy.h"


#define DIRVIEW_DIR_ATTR "dir"

#define DIRVIEW_DIRMAP_ATTR "dirmap"

#define DIRVIEW_NOSELECTED "No directory selected."

#define DIRVIEW_DIRVIEWITEMS "DIRVIEWITEMS"
#define DIRVIEW_DIR "DIR"

#define DIRVIEW_FILENAME "FILENAME"
#define DIRVIEW_SIZE "SIZE"
#define DIRVIEW_OWNER "OWNER"
#define DIRVIEW_GROUP "GROUP"
#define DIRVIEW_DATE "DATE"
#define DIRVIEW_DOWNLOADS "DOWNLOADS"


#define PROPERTY_HTML_DIRVIEW "html_dirview"
#define PROPERTY_HTML_DIRVIEW_ITEM "html_dirview_item"


// implements the webspy_module_t
char *webspy_dirview(httpd *server);


// for nodeview to access dir-ids.
hashtable_item_t *webspy_dirview_get_direntry(char *dir);

#endif

