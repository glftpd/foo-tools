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


#ifndef _foobnc_h
#define _foobnc_h

#include <collection/hashtable.h>
#include <netinet/in.h>

#define DEFAULT_TIMEOUT 30

#define DEFAULT_ERRORTMPL "421 Error: %s\n"

#define LOCALQUEUE 10

// buffer to use for reading client requests.
#define BUFSIZE 4096

struct con_allow {
	int con;
	int sec;
};

typedef struct con_allow con_allow_t;

struct cycleip {
    struct sockaddr_in sockname;
    long is_available;

    struct cycleip *next;
};

typedef struct cycleip cycleip_t;


struct context {

	hashtable_t *cfg;

	char *cfg_name;
	char *cfg_file;
	char *cfg_pass;

	cycleip_t *ips;

	int running;
    pthread_mutex_t mutex;

	struct sockaddr_in bind_to;
};

typedef struct context context_t;


struct bouncer_arg {
	int client_fd;
	int server_fd;

    struct sockaddr_in client_name;
    struct sockaddr_in server_name;

    int accesses;

	context_t *context;

    unsigned long bytes_in, bytes_out;
	pthread_t thread;
};

typedef struct bouncer_arg bouncer_arg_t;

#define MSG_ERROR_CONNECT_REMOTE "Cannot connect to remote"


#define PROPERTY_MSG_WELCOME "entry_msg_welcome"
#define PROPERTY_MSG_BUSY "entry_msg_busy"
#define PROPERTY_MSG_ERROR "entry_msg_error"
#define PROPERTY_MSG_NOIPS "entry_msg_noips"
#define PROPERTY_MSG_DENY "entry_msg_deny"
#define PROPERTY_MSG_HAMMER "entry_msg_hammer"

// admin properties.
#define PROPERTY_ADMIN_HOST "admin_host"
#define PROPERTY_ADMIN_BOUNCER "bouncer"

#define PROPERTY_ENTRY_HOST_MAX "entry_host_max"
#define PROPERTY_ENTRY_HOST "entry_host_%d"
#define PROPERTY_ENTRY_PORT "entry_port"
#define PROPERTY_ENTRY_SIMUL_MAX "entry_simul_max"
#define PROPERTY_IDENT_ENABLE "ident_enable"
#define PROPERTY_CONNECT_ALLOW "connect_allow"

#define PROPERTY_DATA_TIMEOUT "data_timeout"
#define PROPERTY_CONNECT_TIMEOUT "connect_timeout"

#define PROPERTY_CYCLEIPS "tmp_cycle_ips"
#define PROPERTY_LOGFILE "logfile"

#define ADMIN_MAGIC "foobnc-v3"

#endif
