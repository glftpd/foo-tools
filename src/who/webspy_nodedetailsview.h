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


#ifndef _webspy_nodedetailsview_h
#define _webspy_nodedetailsview_h


#include <stdio.h>
#include <sys/types.h>
#include <httpd.h>
#include "webspy.h"

#define NODEDETAILSVIEW_PID_ATTR "pid"

// properties offered by module
#define NODEDETAILSVIEW_NOSELECTED "No node selected."
#define NODEDETAILSVIEW_USERNAME "USERNAME"
#define NODEDETAILSVIEW_PID "PID"
#define NODEDETAILSVIEW_GID "GID"
#define NODEDETAILSVIEW_CWD "CWD"
#define NODEDETAILSVIEW_TAGLINE "TAGLINE"
#define NODEDETAILSVIEW_HOST "HOST"
#define NODEDETAILSVIEW_LOGIN "LOGIN"
#define NODEDETAILSVIEW_DIRID "DIRID"

#define PROPERTY_HTML_NODEDETAILSVIEW "html_nodedetailsview"


// implements the webspy_module_t
char *webspy_nodedetailsview(httpd *server);


#endif

