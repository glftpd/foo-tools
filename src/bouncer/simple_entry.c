/*
 * This is the very basic version of the entry bouncer.  All datanode
 * and ssl stuff has been removed.
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
#include <thread/threadpool.h>

#include "simple_entry.h"

#ifdef ADMINPORT
#include "admin_httpd.h"
#endif

#define VERSION "3.o"
#define CVSVERSION "$Id: simple_entry.c,v 1.6 2002/03/18 15:56:50 flower Exp $"

#define MAX_BUFSIZE 4096

// use this to dump error.
#define OBJDUMP_CMD "objdump -l -d --start-address=%p --stop-address=%p %s"

#define S_SERVER 1
#define S_CLIENT 2

hashtable_t cfg;
time_t start_time;

extern int errno;

char *prog_name;

void entry_sig_block(int sig) {
	sigset_t ss;
	sigemptyset(&ss);
	sigaddset(&ss,sig);
	sigprocmask(SIG_BLOCK,&ss,(sigset_t *) 0);
}

void entry_log(char *fmt, ...) {
	FILE *f;
	va_list ap;

	f = (FILE*)ht_get(&cfg, "logfile");

	if (!f) {
		f = fopen("simple_entry.log", "a");

		ht_put_obj(&cfg, "logfile", f);
	}

	va_start(ap, fmt);
	vfprintf(f, fmt, ap);
	va_end(ap);
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


cycleip_t *entry_get_remotes(hashtable_t *c) {
    int port, max, i;
    char buf[30], *tmp, tbuf[100];
    cycleip_t *r = 0, *tr;
	char *mstr;

	mstr = ht_get(c, PROPERTY_ENTRY_HOST_MAX);

	if (!mstr) {
		printf("Error: no '%s' in configfile.\n", PROPERTY_ENTRY_HOST_MAX);

		exit(1);
	}

	max = atoi(mstr);

    for (i = 0; i < max + 1; i++) {
		sprintf(buf, PROPERTY_ENTRY_HOST, i);

		tmp = ht_get(c, buf);

		if (!tmp)
			continue;

		strcpy(tbuf, tmp);

		tmp = strchr(tbuf, ':');

		if (!tmp)
			continue;

		*tmp = 0;

		port = atoi(tmp + 1);

		tr = (cycleip_t*)malloc(sizeof(cycleip_t));

		if (init_sockaddr(&tr->sockname, tbuf, port)) {
			printf("  --> Cycle-ip: %s/%d\n", tbuf, port);
			tr->next = r;
			r = tr;
		} else
			free(tr);
	}
    
    // make the list cyclic.
    tr = r;
    while (tr && tr->next)
		tr = tr->next;
    tr->next = r;
    
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

int entry_make_socket(char *ip, unsigned short int port) {
    int sock;
    struct sockaddr_in name;
    int on = 1;
    
    // create socket.
    sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
		printf("Error -> Cannot create socket in make_socket .. \n");
		exit(1);
    }
    
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0) {
		printf("Error -> Cannot setsockopt() in make_socket.. \n");
		exit(1);
    }
    
    name.sin_family = AF_INET;
    name.sin_port = htons (port);

	if (ip) {
		inet_aton(ip, &name.sin_addr);
	} else
		name.sin_addr.s_addr = htonl (INADDR_ANY);
    
    // bind socket.
    if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0) {
		printf("Error -> Cannot bind socket in make_socket (%d).. \n", port);
		exit(1);
    }
    
    return sock;
}

int entry_find_ident(struct sockaddr_in *client, int servport, long timeout, char *ident) {
    char *tmp, *ttmp, buf_hostname[500], buf_ip[500], buf_ident[300];
    int ident_fd, on = 1, rc;
    struct sockaddr_in identname;
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
    
    ident_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (ident_fd < 0)
	return 0;
    
    if (setsockopt(ident_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0) {
		close(ident_fd);
		return 0;
    }
    
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

	entry_log("+ Ident Reply: %s\n", buf_ident);
    
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

	entry_log("+ Found ident: %s\n", ttmp);

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
}

void entry_bouncer_error(bouncer_arg_t *a, char *msg) {
	char *tmp;

	tmp = ht_get(&cfg, PROPERTY_MSG_ERROR);

	if (tmp)
		entry_msg_send(a, a->client_fd, tmp, msg);
	else
		entry_msg_send(a, a->client_fd, DEFAULT_ERRORTMPL, msg);

	entry_close(a);
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
	tmp = ht_get(&cfg, PROPERTY_DATA_TIMEOUT);
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
				if (!valid_connection)
					entry_msg_send(bnc, bnc->client_fd, "Hello Kitty!\r\n");

				sock_fd_close(fds);
				return;
			}

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

int entry_property_true(char *p) {
	char *tmp;

	tmp = ht_get(&cfg, p);

	if (!tmp)
		return 0;

	if (!strcasecmp(tmp, "yes"))
		return 1;

	if (!strncmp(tmp, "1", 1))
		return 1;

	return 0;
}

void entry_setup_handler(void *arg) {
	bouncer_arg_t *a;
	char *tmp, ident[MAX_BUFSIZE], buf[MAX_BUFSIZE];
	int rc, on = 1, port;
	long connect_timeout;

	a = (bouncer_arg_t*)arg;

	// get the port.
	tmp = ht_get(&cfg, PROPERTY_ENTRY_PORT);
	port = atoi(tmp);

	// get timeout
	tmp = ht_get(&cfg, PROPERTY_CONNECT_TIMEOUT);
	connect_timeout = atol(tmp);

#ifdef DEBUG
	printf("ConnectTimeout = %d\n", connect_timeout);
#endif

	// send welcome msg to client.
	tmp = ht_get(&cfg, PROPERTY_MSG_WELCOME);
	if (tmp) {
		rc = entry_data_push(a->client_fd, tmp, strlen(tmp), a);

		if (rc < 0) {
			entry_close(a);

			return;
		}
	}

	// create connection to remote.
	a->server_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (a->server_fd < 0) {
		entry_bouncer_error(a, MSG_ERROR_CONNECT_REMOTE);
		return;
	}

    if (setsockopt(a->server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0) {
		entry_bouncer_error(a, MSG_ERROR_CONNECT_REMOTE);
		return;
	}	

    if (sock_connect(a->server_fd, (struct sockaddr*)&a->server_name, sizeof(a->server_name), connect_timeout/1000) < 0) {
		entry_bouncer_error(a, MSG_ERROR_CONNECT_REMOTE);
		return;
	}

	// lookup+send ident if thats what we need.
	if (entry_property_true(PROPERTY_IDENT_ENABLE)) {
		rc = entry_find_ident(&a->client_name, port, connect_timeout, ident);

		sprintf(buf, "IDNT %s\n", ident);
		rc = sock_data_push(a->server_fd, buf, strlen(buf));

		if (rc <= 0) {
			entry_close(a);
			return;
		}
	}

	entry_bounce_handler(a);

	entry_close(a);

	return;
}

void entry_debug_handler(int sig) {
  /*  char addr[20], cmd[160], line[160], src_line[160];
    struct sigcontext *sc;
    FILE *fp;
    char *p;

    sc = (struct sigcontext *)(&sig + 1);

    sprintf(addr, "%lx", sc->eip);

    sprintf(cmd, OBJDUMP_CMD, sc->eip, sc->eip + 0x10, prog_name);
    fp = popen(cmd, "r");

    while(fgets(line, sizeof(line), fp) != NULL) {
        if(*line == '/') {
            break;
        }

        if(*line != ' ') {
            strcpy(src_line, line);
            continue;
        }

        for(p=line; *p && *p!=':'; p++);

        if(*p == ':') {
            *p = 0;

            if(!strcmp(addr, line+1)) {
                break;
            }
        }
	}

	fprintf(stderr, "thread #%u: ", pthread_self());

    if(line[0] == '/') {
        fprintf(stderr, "segmentation violation at %s", line);
	} else {
		src_line[strlen(src_line) - 2] = 0;
        fprintf(stderr, "segmentation violation in function %s\n",
				src_line);
        fprintf(stderr, "compile with -g to get filename and line number\n");
	}

	pclose(fp);

*/    exit(1);
}                                         



/*
 * Main loop.
 */
int main(int argc, char *argv[]) {
    bouncer_arg_t *ba;
	threadpool_t tp;
    cycleip_t *remotes;
    char *tmpstr;
    struct sockaddr_in clientname;
    char buf[300];
    int consock, s_size, pid, timeout = 30, servsock, poolsize, rc;
    FILE *pidfile;

#ifdef ADMINPORT
	admin_httpd_t adminport;
#endif
    
    printf("## 'simple_entry' of foo.Bnc v%s (c) Tanesha Team <tanesha@tanesha.net> ##\n", VERSION);
    
    if (argc < 2) {
		printf("Syntax -> %s <my.properties>\n", argv[0]);
		exit(1);
    }

    ht_init(&cfg);
    
    ht_load_prop(&cfg, argv[1], ' ');
    
    remotes = entry_get_remotes(&cfg);

	// initialize threadpool.
	tmpstr = ht_get(&cfg, PROPERTY_ENTRY_SIMUL_MAX);
	if (!tmpstr) {
		printf("Error -> No '%s' property in config-file.\n", PROPERTY_ENTRY_SIMUL_MAX);
		exit(1);
	}

	poolsize = atoi(tmpstr);

    // get serversocket.
    tmpstr = ht_get(&cfg, PROPERTY_ENTRY_PORT);
    
    if (!tmpstr) {
		printf("Error -> no '%s' property in config-file.\n", PROPERTY_ENTRY_PORT);
		exit(1);
    }
	
	// create socket to listen on.
	rc = atoi(tmpstr);
	tmpstr = ht_get(&cfg, PROPERTY_ENTRY_IP);
    servsock = entry_make_socket(tmpstr, rc);
    
    if (listen(servsock, LOCALQUEUE) < 0) {
		printf("Error -> Cannot listen on entry socket!\n");
		exit(1);
    }
    
#ifndef DEBUG
    printf("Started -> Forking into background (threads: %d).. \n", poolsize);

    // fork into background.
    if (daemon(1, 0) != 0) {
		printf("Error -> Failed going to background\n");
		exit(1);
    }

#endif

	// setup dump handler for segfaults.
	prog_name = argv[0];
	signal(SIGSEGV, entry_debug_handler);

	// block some signals.
	entry_sig_block(SIGPIPE);
	entry_sig_block(SIGCHLD);

	// startup pool
	threadpool_initialize(&tp, poolsize, poolsize);
	threadpool_set_proc(&tp, entry_setup_handler);

	start_time = time(0);
    
#ifdef ADMINPORT

	// start up the admin port.
	adminport.activeips = remotes;
	adminport.threads = &tp;
	adminport.config = &cfg;
	adminport.start_time = start_time;

	pthread_create(&adminport.thread, 0, admin_run, &adminport);
#endif

    // main 'wait for new connection' loop..
    s_size = sizeof(clientname);

	while (1) {

		consock = accept(servsock, (struct sockaddr*)&clientname, &s_size);

		if (consock <= 0)
			continue;
		
		// init argument for bouncer.
		ba = (bouncer_arg_t*)malloc(sizeof(bouncer_arg_t));
		ba->pool = &tp;
		ba->client_fd = consock;
		ba->server_fd = -1;
		memcpy(&ba->client_name, &clientname, sizeof(clientname));
		
		// set remote host to connect to.
		memcpy(&ba->server_name, &remotes->sockname, sizeof(remotes->sockname));

		// cycle onto next ip in list.
		remotes = remotes->next;
		
#ifdef DEBUG
		printf("Connection -> %s -> ", inet_ntoa(clientname.sin_addr));
		printf("%s\n", inet_ntoa(ba->server_name.sin_addr));
#endif
		
		// put work to threadpool.
		rc = threadpool_set_work(&tp, ba);

		// close up if the threadpool was full.
		if (rc <= 0) {
			tmpstr = ht_get(&cfg, PROPERTY_MSG_BUSY);
			if (tmpstr)
				sock_data_push(consock, tmpstr, strlen(tmpstr));

			close(consock);
			free(ba);
		}
    }
    
    close(servsock);
    
    return 0;
}

