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
 * This is the very basic version of the entry bouncer.  All datanode
 * and ssl stuff has been removed.
 *
 * $Id: foobnc.c,v 1.5 2003/09/27 14:59:55 sorend Exp $
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>


#include <collection/hashtable.h>
#include <lib/common.h>
#include <lib/stringtokenizer.h>
#include <lib/sockop.h>
#include <util/linereaderbuffer.h>
#include <collection/strlist.h>

// admin port use httpd
#include <httpd.h>

#include "foobnc.h"

#define VERSION "3.3"
#define CVSVERSION "$Id: foobnc.c,v 1.5 2003/09/27 14:59:55 sorend Exp $"

#define MAX_BUFSIZE 4096
#define RUNTIME_CONNECT_ALLOW "runtime_connect_allow"
#define RUNTIME_HAMMER_LIST "runtime_hammer_list"

#define LOGFILE_HANDLE "logfile_handle"

struct kv {
	char *k;
	char *v;
	int fd;
	pthread_t t;
};

typedef struct kv kv_t;

hashtable_t _bouncer_context;
pthread_mutex_t _bouncer_mutex;
hashtable_t cfg;
time_t start_time;

extern int errno;

char *prog_name;
struct sockaddr_in admin_host;

cycleip_t * entry_cycleip_next(hashtable_t *c);

void entry_sig_block(int sig) {
	sigset_t ss;
	sigemptyset(&ss);
	sigaddset(&ss,sig);
	sigprocmask(SIG_BLOCK,&ss,(sigset_t *) 0);
}

int entry_log(char *fmt, ...) {
	FILE *f = 0;
	va_list ap;
	char *tmp, tmpbuf[400], timebuf[30];
	time_t now;
	struct tm *tm_now;

	f = (FILE*)ht_get(&cfg, LOGFILE_HANDLE);

	if (!f) {
		tmp = ht_get(&cfg, PROPERTY_LOGFILE);

		if (tmp)
			f = fopen(tmp, "a");

		if (f)
			ht_put_obj(&cfg, LOGFILE_HANDLE, f);
	}

	if (f) {
		now = time(0);
		tm_now = localtime(&now);

		strftime(timebuf, 30, "%a %b %d %T %Y", tm_now);


		va_start(ap, fmt);
		vsprintf(tmpbuf, fmt, ap);
		va_end(ap);

		tmp = strchr(tmpbuf, '\n');
		if (tmp)
			*tmp = 0;

		fprintf(f, "%s [%5d] %s\n", timebuf, pthread_self(), tmpbuf);
		fflush(f);
	}

	return 0;
}


void entry_makeage(long t, char *buf) {
	long age = time(0) - t;
	long days, hours, mins, secs;
	
	days  =  age / (3600 * 24);
	age   -= days * 3600 * 24;
	hours =  age / 3600;
	age   -= hours * 3600;
	mins  =  age / 60;
	secs  =  age - (mins * 60);
	
	if (days)
		sprintf(buf, "%dd %dh", days, hours);
	else if (hours)
		sprintf(buf, "%dh %dm", hours, mins);
	else
		sprintf(buf, "%dm %ds", mins, secs);
}

int entry_init_hostport(struct sockaddr_in *sa, char *hostport) {
	char *tmp, *buf;
	int port, rc;

	buf = strdup(hostport);
	tmp = strrchr(buf, ':');

	if (!tmp) {
		free(buf);
		return 0;
	}

	*tmp = 0;

	// convert port.
	port = atoi(tmp + 1);

	rc = init_sockaddr(sa, buf, port);

	free(buf);

	return rc;
}


cycleip_t *entry_get_remotes(context_t *ctx) {
    int port, max, i;
    char buf[30], *tmp, tbuf[100];
    cycleip_t *r = 0, *tr, *lastt = 0;
	char *mstr;
	hashtable_t *c;

	c = ctx->cfg;

	mstr = ht_get(c, PROPERTY_ENTRY_HOST_MAX);

	if (!mstr) {
		entry_log("error, no '%s' in configfile.", PROPERTY_ENTRY_HOST_MAX);
		return 0;
	}

	max = atoi(mstr);

    for (i = 0; i < max + 1; i++) {
		sprintf(buf, PROPERTY_ENTRY_HOST, i);

		tmp = ht_get(c, buf);

		if (!tmp)
			continue;

		tr = (cycleip_t*)malloc(sizeof(cycleip_t));

		if (entry_init_hostport(&tr->sockname, tmp)) {
			tr->is_available = 1;
			entry_log("bouncer %s -> cycle-ip: %s", ctx->cfg_name, tmp);
			tr->next = 0;

			if (!r)
				lastt = r = tr;
			else {
				lastt->next = tr;
				lastt = tr;
			}
		} else
			free(tr);
	}
    
    // make the list cyclic.
	lastt->next = r;
    
    return r;
}


int entry_data_read(int fd, char *buf, int len, bouncer_arg_t *bnc) {
	return read(fd, buf, len);
}


int entry_data_write(int fd, char *buf, int len, bouncer_arg_t *bnc) {
	return write(fd, buf, len);
}

int entry_data_push(int fd, char *str, int len, bouncer_arg_t *b) {
	int nleft, nwritten;

	nleft = len;

	while (nleft > 0) {
		nwritten = entry_data_write(fd, str, nleft, b);

		if (nwritten < 0) {
			if (errno != EAGAIN)
				return nwritten;
		}
		else {

			// sfv_calc_buf(str, crc, nwritten, 0);
		
			nleft -= nwritten;
			str += nwritten;
		}
	}
	
	return (len-nleft);
}


int entry_msg_send(bouncer_arg_t *b, int fd, char *fmt, ...) {
	char buf[MAX_BUFSIZE];
	va_list ap;

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);

	return entry_data_push(fd, buf, strlen(buf), b);
}

int entry_bind_socket(struct sockaddr_in *name, int no_port) {
    int sock;
    int on = 1;
	struct sockaddr_in tmp;

    // create socket.
    sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
		return -1;
    }
    
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0) {
		return -1;
    }

	memcpy(&tmp, name, sizeof(struct sockaddr_in));

	if (no_port)
		tmp.sin_port = 0;
    
    // bind socket.
    if (bind (sock, (struct sockaddr *) &tmp, sizeof (struct sockaddr_in)) < 0) {
		return -1;
    }
    
    return sock;
}

int entry_find_ident(context_t *ctx, struct sockaddr_in *client, int servport, long timeout, char *ident) {
    char *tmp, *ttmp, buf_hostname[500], buf_ip[500], buf_ident[300];
    int ident_fd, on = 1, rc;
    struct sockaddr_in identname, servname;
    struct hostent *hostname;
    selectfds_t *sfd = 0;
    
    strcpy(buf_ip, inet_ntoa(client->sin_addr));
    sprintf(buf_hostname, "ip-%s", buf_ip);
    
    hostname = gethostbyaddr((char*)&client->sin_addr, sizeof(struct in_addr), AF_INET);
    
    if (hostname)
		sprintf(buf_hostname, hostname->h_name);
    
    if (strchr(buf_hostname, '@'))
		sprintf(buf_hostname, "hacker-%s", buf_ip);
    
    if (strchr(buf_hostname, '*'))
		sprintf(buf_hostname, "hacker-%s", buf_ip);
    
    sprintf(ident, "*@%s:%s", buf_ip, buf_hostname);
    
	ident_fd = entry_bind_socket(&ctx->bind_to, 1);

    if (ident_fd < 0)
		return 0;
    
    if (!init_sockaddr(&identname, (char*)inet_ntoa(client->sin_addr), 113)) {
		close(ident_fd);
		return 0;
	}
    
    if (sock_connect(ident_fd, (struct sockaddr *) &identname, sizeof(identname), timeout/1000) < 0) {
		close(ident_fd);
		return 0;
	}
    
	sprintf(buf_ident, "%u, %u\r\n", htons(client->sin_port), servport);
	
	// send request.
	rc = sock_data_push(ident_fd, buf_ident, strlen(buf_ident));

	if (rc <= 0) {
		close(ident_fd);
		return 0;
	}

    sfd = sock_fd_add(0, ident_fd, 0);

	// wait for answer.
	rc = sock_fd_select(sfd, timeout);

	free(sfd);

	if (rc <= 0) {
		close(ident_fd);
		return 0;
	}

	rc = read(ident_fd, buf_ident, 300);
    
    close(ident_fd);

	if (rc <= 0)
		return 0;

	// make it a terminated string.
	buf_ident[rc] = 0;

	// entry_log("raw ident: '%s'", buf_ident);

    if (strstr(buf_ident, "ERROR"))
		return 0;
    
    tmp = strrchr(buf_ident, ':');
    
    if (!tmp)
		return 0;
    
    tmp++;
    
    while (*tmp == ' ')
		tmp++;
    
    ttmp = tmp;
    
	while (*tmp) {
		if ((*tmp == ' ') || (*tmp == '\n') || (*tmp == '\r'))
			*tmp = 0;
		else
			tmp++;
	}

    // guard against users putting wildcards in their ident.
    if (strchr(ttmp, '@') || strchr(ttmp, '*') || strchr(ttmp, ']') || strchr(ttmp, '}'))
		sprintf(ident, "hacked-ident@%s:%s", buf_ip, buf_hostname);
    else
		sprintf(ident, "%s@%s:%s", ttmp, buf_ip, buf_hostname);

	// return true cause we actually got an ident.
	return 1;
}

void entry_close(bouncer_arg_t *me) {
	// close socket.
	if (me->client_fd > -1)
		close(me->client_fd);

	if (me->server_fd > -1)
		close(me->server_fd);

	// free memory.
	// .. nothing in it to free.

	// decrease the running integer.
	pthread_mutex_lock(&me->context->mutex);
	me->context->running--;
	pthread_mutex_unlock(&me->context->mutex);

}

void entry_bouncer_error(bouncer_arg_t *a, char *msg) {
	char *tmp;

	tmp = ht_get(a->context->cfg, PROPERTY_MSG_ERROR);

	if (tmp)
		entry_msg_send(a, a->client_fd, tmp, msg);
	else
		entry_msg_send(a, a->client_fd, DEFAULT_ERRORTMPL, msg);

	entry_close(a);

	entry_log("error: %s", msg);
}

int entry_max_sim_get(context_t *ctx) {
	char *tmpstr;
	int rc;

	// get max simul.
	tmpstr = ht_get(ctx->cfg, PROPERTY_ENTRY_SIMUL_MAX);
	if (tmpstr)
		rc = atoi(tmpstr);
	else
		rc = 0;

	return rc;
}

/*
 * This does the actual bouncing.
 */
void *entry_bounce_handler(bouncer_arg_t *bnc) {
	selectfds_t *fds, *iter_fd = 0;
	char xferbuf[BUFSIZE], tmpbuf[BUFSIZE], *tmp;
    char ident[300], ip[50], ftpport[50];
	int len, valid_connection = 0, rc, port, isbounced, i;
	long data_timeout;

	// setup select fds.
	fds = sock_fd_add(0, bnc->client_fd, bnc->server_fd);
	fds = sock_fd_add(fds, bnc->server_fd, bnc->client_fd);

    // get timeout val.
	tmp = ht_get(bnc->context->cfg, PROPERTY_DATA_TIMEOUT);
	data_timeout = atol(tmp);

#ifdef DEBUG
	printf("Data timeout: %d\n", data_timeout);
#endif

    while (1) {
		rc = sock_fd_select(fds, data_timeout);

		// if there was timeout, then end this one.
		if (rc <= 0)
			break;

		for (iter_fd = fds; iter_fd; iter_fd = iter_fd->next) {

			if (!iter_fd->has_data)
				continue;

			len = read(iter_fd->rfd, xferbuf, BUFSIZE);

			if (len <= 0) {
				if (!valid_connection) {
					tmp = ht_get(bnc->context->cfg, PROPERTY_MSG_DENY);

					if (tmp)
						entry_msg_send(bnc, bnc->client_fd, tmp);
				}

				sock_fd_close(fds);
				return;
			}

			bnc->bytes_out += len;

			// if we got data from server the connection should be ok.
			if (iter_fd->rfd == bnc->server_fd)
				valid_connection = 1;

			rc = entry_data_push(iter_fd->wfd, xferbuf, len, bnc);

			if (rc <= 0) {
				sock_fd_close(fds);
				return;
			}
		}
    }
}

int entry_property_true(hashtable_t *cfg, char *p) {
	char *tmp;

	tmp = ht_get(cfg, p);

	if (!tmp)
		return 0;

	if (!strcasecmp(tmp, "yes"))
		return 1;

	if (!strncmp(tmp, "1", 1))
		return 1;

	return 0;
}

void *entry_setup_handler(void *arg) {
	bouncer_arg_t *a;
	char *tmp, ident[MAX_BUFSIZE], buf[MAX_BUFSIZE];
	int rc, on = 1, port;
	long connect_timeout;
    struct sockaddr_in name;

	a = (bouncer_arg_t*)arg;

	// get the port.
	port = ntohs(a->context->bind_to.sin_port);

	// get timeout
	tmp = ht_get(a->context->cfg, PROPERTY_CONNECT_TIMEOUT);
	connect_timeout = atol(tmp);

	strcpy(buf, inet_ntoa(a->client_name.sin_addr));
	entry_log("new worker, bouncing: %s:%d -> %s:%d",
			  buf, ntohs(a->client_name.sin_port),
			  inet_ntoa(a->server_name.sin_addr), ntohs(a->server_name.sin_port));

	a->bytes_in = a->bytes_out = 0;

#ifdef DEBUG
	printf("ConnectTimeout = %d\n", connect_timeout);
#endif

	// send welcome msg to client.
	tmp = ht_get(a->context->cfg, PROPERTY_MSG_WELCOME);
	if (tmp) {
		rc = entry_data_push(a->client_fd, tmp, strlen(tmp), a);

		if (rc < 0) {
			entry_close(a);

			return;
		}
	}

	// create connection to remote.
	a->server_fd = entry_bind_socket(&a->context->bind_to, 1);

	if (a->server_fd < 0) {
		entry_bouncer_error(a, MSG_ERROR_CONNECT_REMOTE);
		return;
	}

    if (sock_connect(a->server_fd, (struct sockaddr*)&a->server_name, sizeof(a->server_name), connect_timeout/1000) < 0) {
		entry_bouncer_error(a, MSG_ERROR_CONNECT_REMOTE);
		return;
	}

	// lookup+send ident if thats what we need.
	if (entry_property_true(a->context->cfg, PROPERTY_IDENT_ENABLE)) {

		rc = entry_find_ident(a->context, &a->client_name,
							  port, connect_timeout, ident);

		entry_log("ident lookup: '%s'", ident);

		sprintf(buf, "IDNT %s\n", ident);
		rc = sock_data_push(a->server_fd, buf, strlen(buf));

		if (rc <= 0) {
			entry_close(a);
			return;
		}
	}

	entry_bounce_handler(a);

	entry_close(a);

	entry_log("worker finished. session-traffic: %lu bytes", a->bytes_out);

	return;
}


cycleip_t * entry_cycleip_next(hashtable_t *c) {
	cycleip_t *tmp, *start;

	start = (cycleip_t*)ht_get(c, PROPERTY_CYCLEIPS);

	// the unlikely case of no ips.
	if (!start)
		return 0;

	// there is only one entry.
	if (start && start == start->next) {
		if (start->is_available)
			return start;
		else
			return 0;
	}

	// find the next available.
	for (tmp = start->next; tmp; tmp = tmp->next) {

		if (tmp == start)
			break;

		if (tmp->is_available == 1)
			break;
	}

	if (!tmp)
		return 0;

	if (tmp == start)
		return 0;

	// update ip in list.
	ht_put_obj_free(c, PROPERTY_CYCLEIPS, tmp, 0);

	return tmp;
}

int entry_is_hammer(context_t *ctx, struct sockaddr_in *sa) {
	char ipbuf[16];
	con_allow_t *ca;
	int rc;
	hashtable_t *hammer;

	ca = (con_allow_t*) ht_get(ctx->cfg, RUNTIME_CONNECT_ALLOW);

	if (!ca)
		return 0;

	hammer = (hashtable_t*) ht_get(ctx->cfg, RUNTIME_HAMMER_LIST);

	// hammer protection is setup, but cant find the hammer list, something
	// is up, so we deny !
	if (!hammer) {
		entry_log("something is up with hammer protection, denying ..");
		return 1;
	}

	// hp_show_state(hammer, ca->sec, entry_log);

	strcpy(ipbuf, inet_ntoa(sa->sin_addr));

	hp_add_connection(hammer, ipbuf, ca->sec);

	rc = hp_get_connections_since(hammer, ipbuf, ca->sec);

	if (rc > ca->con) {
		entry_log("hammering: %s has %d (allowed:%d) connects in last %d sec", ipbuf, rc, ca->con, ca->sec);
		return 1;
	}

	return 0;
}

/*
 * Get hammer protection config, put it back to local config.
 */
int entry_hp_config(hashtable_t *cfg) {
	hashtable_t *tht;
	con_allow_t *ct;
	char *tmp;
	stringtokenizer st;

	// get setting from conf.
	tmp = ht_get(cfg, PROPERTY_CONNECT_ALLOW);

	if (!tmp)
		return 0;

	st_initialize(&st, tmp, " ");

	if (st_count(&st) != 2)
		return 0;

	ct = malloc(sizeof(con_allow_t));

	tmp = st_next(&st);
	ct->con = atoi(tmp);

	tmp = st_next(&st);
	ct->sec = atoi(tmp);

	entry_log("configured hammer protection, %d con in %d sec",
			  ct->con, ct->sec);
	
	tht = malloc(sizeof(hashtable_t));
	ht_init(tht);

	ht_put_obj(cfg, RUNTIME_CONNECT_ALLOW, ct);
	ht_put_obj(cfg, RUNTIME_HAMMER_LIST, tht);

	return 1;
}



int entry_reload_config(context_t *ctx) {
	cycleip_t *remotes = 0;

	// free old config
	if (ctx->cfg) {
		ht_finalize(ctx->cfg);
		free(ctx->cfg);
	}

	ctx->cfg = malloc(sizeof(hashtable_t));
	ht_init(ctx->cfg);

	ht_load_prop(ctx->cfg, ctx->cfg_file, ' ');

	// setup hammer protection.
	entry_hp_config(ctx->cfg);

    // get remotes
	remotes = entry_get_remotes(ctx);
	if (!remotes) {
		entry_log("error, no site ips (entry_host_*) found in '%s'.",
				  ctx->cfg_file);
		return 0;
	}

	ht_put_obj_free(ctx->cfg, PROPERTY_CYCLEIPS, remotes, 0);
    
    return 1;
}


/*
 * Thread run method for a port listener.
 *
 */
void *entry_port_run(void *args) {

	stringtokenizer st;
	context_t ctx;
	int s_size, consock, servsock, rc;
	char *s_ipport, *tmpstr;
	struct sockaddr_in clientname;
	cycleip_t *remotes;
	bouncer_arg_t *ba;
	kv_t *tmp;

	tmp = (kv_t*)args;

	st_initialize(&st, tmp->v, " ");

	// check if all config there.
	if (st_count(&st) != 3) {
		entry_log("invalid portconfig '%s' shutting down port.", tmp);
		st_finalize(&st);
		return;
	}

	s_ipport = st_next(&st);
	ctx.cfg_pass = st_next(&st);
	ctx.cfg_file = st_next(&st);
	ctx.cfg_name = tmp->k;
	ctx.running = 0;
	ctx.cfg = 0;
	pthread_mutex_init(&ctx.mutex, 0);

	pthread_mutex_lock(&_bouncer_mutex);
	// put this context to global contexts.
	ht_put_obj(&_bouncer_context, tmp->k, &ctx);
	pthread_mutex_unlock(&_bouncer_mutex);

	if (!entry_init_hostport(&ctx.bind_to, s_ipport)) {
		entry_log("could not create host from %s", s_ipport);
		return;
	}

	while (1) {

		servsock = entry_bind_socket(&ctx.bind_to, 0);

		if (servsock < 0) {
			entry_log("could not bind to socket %s .. waiting 10..", s_ipport);
			sleep(10);
			continue;
		}

		// start to listen
		if (listen(servsock, LOCALQUEUE) < 0) {
			entry_log("could not listen on socket %s .. waiting 10..", s_ipport);
			sleep(10);
			continue;
		}

		break;
	}

	// try to load the config.
	entry_reload_config(&ctx);

	entry_log("bouncer %s started, cfg: %s", tmp->k, tmp->v);

    // main 'wait for new connection' loop..
    s_size = sizeof(clientname);

	while (1) {

		// get max simul.
		rc = entry_max_sim_get(&ctx);

		consock = accept(servsock, (struct sockaddr*)&clientname, &s_size);

		if (consock <= 0)
			continue;

		// check for hammering
		if (entry_is_hammer(&ctx, &clientname)) {
			tmpstr = ht_get(ctx.cfg, PROPERTY_MSG_HAMMER);
			if (tmpstr)
				sock_data_push(consock, tmpstr, strlen(tmpstr));
			close(consock);
			entry_log("closed for hammering user ..");
			continue;
		}

		
		// init argument for bouncer.
		ba = (bouncer_arg_t*)malloc(sizeof(bouncer_arg_t));
		ba->client_fd = consock;
		ba->server_fd = -1;
		ba->context = &ctx;

		memcpy(&ba->client_name, &clientname, sizeof(clientname));

		remotes = entry_cycleip_next(ctx.cfg);

		if (!remotes) {
			tmpstr = ht_get(ctx.cfg, PROPERTY_MSG_NOIPS);
			if (tmpstr)
				sock_data_push(consock, tmpstr, strlen(tmpstr));
			close(consock);
			free(ba);

			continue;
		}
		
		// set remote host to connect to.
		memcpy(&ba->server_name, &remotes->sockname, sizeof(remotes->sockname));

		// if we are already all threads.
		pthread_mutex_lock(&ctx.mutex);
		if (ctx.running >= rc) {
			entry_log("bouncer is pool is full: %d/%d", ctx.running, rc);
			pthread_mutex_unlock(&ctx.mutex);
			close(consock);
			free(ba);
			continue;
		}
		ctx.running++;
		pthread_mutex_unlock(&ctx.mutex);

		// else start the bouncer thread.
		pthread_create(&ba->thread, 0, entry_setup_handler, ba);

		// detach thread.
		pthread_detach(ba->thread);

    }
    
    close(servsock);

}


int entry_start_ports(hashtable_t *cfg) {

	hashtable_t *bouncers;
	hashtable_item_t *hi;
	kv_t *tmp;

	bouncers = ht_get_tree(cfg, PROPERTY_ADMIN_BOUNCER, '_');

	ht_reset(bouncers);

	// iterate over bouncer_*
	while (hi = ht_next(bouncers)) {

		tmp = malloc(sizeof(kv_t));

		tmp->k = strdup(hi->key);
		tmp->v = strdup(hi->value);

		// kick off the port.
		pthread_create(&tmp->t, 0, entry_port_run, tmp);
	}

	// free bouncers.
	ht_finalize(bouncers);
	free(bouncers);

}


int entry_admin_cmd_config(context_t *ctx, httpd *server) {
	FILE *f;
	int rc;
	httpVar *var;

	var = httpdGetVariableByName(server, "config");

	if (!var)
		return 0;

	// try save the request
	f = fopen(ctx->cfg_file, "w");
	if (!f)
		return 0;

	rc = fwrite(var->value, 1, strlen(var->value), f);
	fclose(f);

	// now reload the config.
	entry_reload_config(ctx);

	return 1;
}



void entry_admin_handler(httpd *server) {

	httpVar *var;
	context_t *ctx;
	char *tmp;
	int rc = 0;

	if (httpdAuthenticate(server, "foobnc") == 0) {
		httpdOutput(server, "Authentication failure.");
		return;
	}

	ctx = (context_t*)ht_get(&_bouncer_context, server->request.authUser);
	
	if (ctx == 0 ||
		strcmp(server->request.authPassword, ctx->cfg_pass) != 0) {
		httpdForceAuthenticate(server, "foobnc");
		httpdOutput(server, "Authentication failure.");
		return;
	}

	var = httpdGetVariableByName(server, "command");

	entry_log("admin request: %s\n", (var?var->value:"?"));

	if (var && !strcmp("configure", var->value))
		rc = entry_admin_cmd_config(ctx, server);

	// send response.
	if (rc)
		httpdPrintf(server, "%s: OK", (var?var->value:"?"));
	else
		httpdPrintf(server, "%s: FAIL", (var?var->value:"?"));
}

/*
 * Main loop.
 */
int main(int argc, char *argv[]) {
    char *tmpstr, *tt;
	struct timeval timeout;
	httpd *server;
	int result;

    printf("## foobnc v%s (c) Tanesha Team <tanesha@tanesha.net> ##\n", VERSION);
    
    if (argc < 2) {
		printf("Syntax -> %s <my.cfg>\n", argv[0]);
		exit(1);
    }

	// load bouncer property.
    ht_init(&cfg);
    ht_load_prop(&cfg, argv[1], ' ');

	// init context globals
	ht_init(&_bouncer_context);

    // get where to bind admin.
    tmpstr = ht_get(&cfg, PROPERTY_ADMIN_HOST);
    
    if (!tmpstr) {
		printf("Error -> no '%s' property in config-file.\n", PROPERTY_ADMIN_HOST);
		exit(1);
    }

	tt = strrchr(tmpstr, ':');
	if (!tt) {
		printf("Error -> bad format for ip:port '%s'\n", tmpstr);
		exit(1);
	}

	*tt = 0;

	server = httpdCreate(tmpstr, atoi(tt + 1));

	if (!server) {
		printf("Error -> could not create admin server\n");
		exit(1);
	}

    printf("Started -> Forking into background .. \n");

    // fork into background.
    if (daemon(1, 0) != 0) {
		printf("Error -> Failed going to background\n");
		exit(1);
    }

	//// now in background ;-)

	// block some signals.
	entry_sig_block(SIGPIPE);
	entry_sig_block(SIGCHLD);

	start_time = time(0);

	pthread_mutex_init(&_bouncer_mutex, 0);

	entry_start_ports(&cfg);

	httpdAddCContent(server, "/", "foobnc.html", HTTP_TRUE, 0, entry_admin_handler);

	entry_log("admin port startup ok, waiting for commands ..");

	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	while (1) {

		result = httpdGetConnection(server, &timeout);

		if (result == 0)
			continue;
		else if (result < 0) {
			entry_log("error reading request on admin port.\n");
			continue;
		}

		if (httpdReadRequest(server) < 0) {
			httpdEndRequest(server);
			entry_log("error reading request on admin port.\n");
			continue;
		}

		httpdProcessRequest(server);
		httpdEndRequest(server);

	}

}
