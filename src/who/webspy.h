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



#ifndef _webspy_h
#define _webspy_h

#include <stdio.h>
#include <sys/types.h>
#include <httpd.h>
#include <collection/hashtable.h>

// glftpd config properties
#define PROPERTY_GLFTPD_IPC_KEY "glftpd_ipc_key"
#define PROPERTY_GLFTPD_ETCDIR "glftpd_etcdir"
#define PROPERTY_GLFTPD_USERDIR "glftpd_userdir"
#define PROPERTY_GLFTPD_SITEDIR "glftpd_sitedir"

// http related config properties
#define PROPERTY_HTTP_PORT "http_port"
#define PROPERTY_HTTP_HOST "http_host"
#define PROPERTY_HTTP_ACCESS_LOG "http_access_log"

#define PROPERTY_HTTP_USER "http_user"
#define PROPERTY_HTTP_PASS "http_pass"

// html/template related config properties
#define PROPERTY_HTML_INDEX "html_index"
#define PROPERTY_HTML_NODEVIEW "html_nodeview"
#define PROPERTY_HTML_NODEVIEW_ITEM "html_nodeview_item"

// internal properties for macros in modules.
#define INDEX_REFRESH "REFRESH"

#define NODEVIEWITEM_POS "POS"
#define NODEVIEWITEM_PID "PID"
#define NODEVIEWITEM_USERNAME "USERNAME"
#define NODEVIEWITEM_ACTION "ACTION"
#define NODEVIEWITEM_IDLE "IDLE"
#define NODEVIEWITEM_CWD "CWD"

#define NODEVIEW_NODEVIEWITEMS "NODEVIEWITEMS"

// version property
#define SPY_VERSION "webspy [foo-spy v2.o] (c) Tanesha FTPD Project"

// misc property for username
#define ONLINE_ATTR "online"


// structure for modules.
struct webspy_module {
	char *name;
	char *(*runfunc)(httpd *server);
};

typedef struct webspy_module webspy_module_t;


// accessor for modules to get config
hashtable_t *get_config();

// accessor for modules to get context
hashtable_t *get_context();


// utility functions
void spy_makeage(time_t t, time_t age, char *buf);

void spy_add_refresh_var(char *varname);

#endif
