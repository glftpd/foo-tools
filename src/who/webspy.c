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


// system includes
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

// libhttpd includes
#include <httpd.h>

// project includes
#include <lib/who.h>
#include <collection/hashtable.h>
#include <lib/macro.h>
#include <lib/pwdfile.h>
#include <util/date.h>
#include <collection/strlist.h>

#include "webspy.h"
#include "webspy_modules.h"

#define ONLINE_ATTR "online"
#define SERVER_ATTR "server"
#define REFRESH_ATTR "refresh"


// contexts.
hashtable_t *_cfg = 0;
hashtable_t *_ctx = 0;


// get config
hashtable_t *get_config() {

	if (_cfg == 0) {
		_cfg = malloc(sizeof(hashtable_t));
		ht_init(_cfg);
	}

	return _cfg;
}

hashtable_t *get_context() {

	if (_ctx == 0) {
		_ctx = malloc(sizeof(hashtable_t));
		ht_init(_ctx);
	}

	return _ctx;
}

void spy_makeage(time_t t, time_t age, char *buf) {
	time_t days=0,hours=0,mins=0,secs=0;
        
	if (t>=age)
		sprintf(buf,"0m 0s");
	else {
		age-=t;
		days=age/(3600*24);
		age-=days*3600*24;
		hours=age/3600;
		age-=hours*3600;
		mins=age/60;
		secs=age-(mins*60);
		
		if (days)
			sprintf(buf,"%dd %dh",days,hours);
		else if (hours)
			sprintf(buf,"%dh %dm",hours,mins);
		else
			sprintf(buf,"%dm %ds",mins,secs);
	}
}


// initialize the who structure.
void spy_who_init() {

	hashtable_t *cfg;
	char *tmp;
	key_t ipckey;
	int rc;
	online_t *who;

	who = malloc(sizeof(online_t));

	cfg = get_config();

	tmp = ht_get(cfg, PROPERTY_GLFTPD_IPC_KEY);

	if (!tmp || strncmp(tmp, "0x", 2)) {
		printf("missing/bad property %s in config.\n", PROPERTY_GLFTPD_IPC_KEY);
		exit(1);
	}

	sscanf(tmp + 2, "%lx", &ipckey);

	rc = who_init(who, ipckey);

	if (rc != 1) {
		printf("could not attach shared memory of glftpd.\n");
		exit(1);
	}

	// put to context.
	ht_put_obj(get_context(), ONLINE_ATTR, who);
	
}


void spy_util_init() {
	char *tmp;

	tmp = ht_get(get_config(), PROPERTY_GLFTPD_ETCDIR);

	if (tmp)
		pwd_set_etcdir(tmp);

}

int spy_http_auth(httpd *server) {
	char *user, *pass;

	user = ht_get(get_config(), PROPERTY_HTTP_USER);
	pass = ht_get(get_config(), PROPERTY_HTTP_PASS);

	if (httpdAuthenticate(server, "foo-spy auth?") == 0) {
		httpdOutput(server, "Authentication failure(1).");
		return 0;
	}

	if (strcmp(server->request.authUser, user) != 0 ||
	    strcmp(server->request.authPassword, pass) != 0) {
		httpdForceAuthenticate(server, "foo-spy auth?");
		httpdOutput(server, "Authentication failure (2).");
		return 0;
	}

	return 1;
}


char * spy_var_join(httpd *server) {
	httpVar *tv;
	hashtable_t ht;
	hashtable_item_t *i;
	int nlen = 0;
	char *nv;

	ht_init(&ht);

	tv = httpdGetVariableByName(server, "pid");
	if (tv)
		ht_put(&ht, tv->name, tv->value);

	ht_reset(&ht);
	while (ht_hasnext(&ht)) {
		i = ht_next(&ht);
		
		nlen += strlen(i->key) + strlen(i->value) + 2;
	}

	nv = malloc(nlen + 1);
	*nv = 0;

	ht_reset(&ht);
	while (ht_hasnext(&ht)) {
		i = ht_next(&ht);

		strcat(nv, i->key);
		strcat(nv, "=");
		strcat(nv, i->value);

		if (ht_hasnext(&ht))
			strcat(nv, "&");
	}

	return nv;
}

void spy_add_refresh_var(char *varname) {

	strlist_t *l = 0;

	l = (strlist_t*) ht_get(get_context(), REFRESH_ATTR);

	l = str_add(l, varname);

	// put updated list, since 'l' has changed.
	ht_put_obj(get_context(), REFRESH_ATTR, l);
}



void spy_http_create_view(httpd *server) {

	char *index, *moduleout;
	webspy_module_t *module;
	struct macro_list *ml = 0;
	int i;

	for (i = 0; webspy_modules[i].name != 0; i++) {

		// get output from module.
		moduleout = webspy_modules[i].runfunc(server);

		// add replacer for module output.
		ml = ml_addstring(ml, webspy_modules[i].name, moduleout);

		// free the output.
		free(moduleout);

		printf("done with module: %s\n", webspy_modules[i].name);
	}

	// handle refresh tag.
	moduleout = spy_var_join(server);
	ml = ml_addstring(ml, INDEX_REFRESH, moduleout);
	free(moduleout);

	index = ml_replacebuf(ml, ht_get(get_config(), PROPERTY_HTML_INDEX));

	ml_free(ml);

	httpdOutput(server, index);

	free(index);

}


void spy_http_serve(httpd *server) {

	int rc;

	rc = spy_http_auth(server);

	if (!rc)
		return;

	spy_http_create_view(server);

}

void spy_http_init() {

	httpd *server;
	char *tmp, *host = 0;
	int port = 80;
	FILE *f;

	tmp = ht_get(get_config(), PROPERTY_HTTP_PORT);
	if (!tmp) {
		printf("missing config setting: %s\n", PROPERTY_HTTP_PORT);
		exit(1);
	}

	port = atoi(tmp);
	host = ht_get(get_config(), PROPERTY_HTTP_HOST);

	server = httpdCreate(host, port);

	if (!server) {
		printf("error creating http server.\n");
		exit(1);
	}

	httpdAddCContent(server, "/", "index.html", HTTP_TRUE, 0, spy_http_serve);

	tmp = ht_get(get_config(), PROPERTY_HTTP_ACCESS_LOG);
	if (tmp) {

		f = fopen(tmp, "a");
		if (f)
			httpdSetAccessLog(server, f);
	}

	printf("+ Http access on: http://%s:%d/index.html\n", host, port);

	ht_put_obj(get_context(), SERVER_ATTR, server);

}

int spy_daemonize() {
	int rc;

	printf("+ Daemonizing to background .. \n");
	
	rc = daemon(1, 0);

	if (rc < 0) {
		printf("! Error, could not go to background .. \n");
		exit(1);
	}
}


int spy_http_run() {
	struct timeval timeout;
	httpd *server;
	int result;

	// init shared memory
	spy_who_init();

	// init http server
	spy_http_init();

	// go to background.
	// spy_daemonize();

	// delay of 60sec between timeouts.
	timeout.tv_sec = 60;
	timeout.tv_usec = 0;

	server = (httpd*)ht_get(get_context(), SERVER_ATTR);

	// serve http loop.
	while(1) {
		result = httpdGetConnection(server, &timeout);

		// timeout or error.
		if (result <= 0)
			continue;

		if(httpdReadRequest(server) < 0) {
			httpdEndRequest(server);
			continue;
		}
		httpdProcessRequest(server);
		httpdEndRequest(server);
	}

}




int main(int argc, char *argv[]) {

	int rc;

	printf("-- %s --\n", SPY_VERSION);

	if (argc < 2) {
		printf("syntax: %s <config-file>\n", argv[0]);

		return 1;
	}

	// load config
	ht_load_prop(get_config(), argv[1], ' ');

	spy_http_run();

}
