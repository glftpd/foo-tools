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
 * Threadsafe timeout'ing socket operations  /sorend.
 *
 * These are the same as the ones without timeout in the manpages.  Timeout values
 * are all in seconds.
 */

#ifndef _sockop_h
#define _sockop_h

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <stdlib.h>

/*
 * Structure for listening on sockets.
 */
struct selectfds {
	int rfd;
	int wfd;

	int has_data;

	struct selectfds *next;
};

typedef struct selectfds selectfds_t;


/*
 * Initializes a socket.
 */
int init_sockaddr(struct sockaddr_in *name, const char *hostname, unsigned short int port);

/*
 * Connects a socket.
 */
int sock_connect(int sockfd, struct sockaddr *serv_addr, socklen_t addrlen, int timeout);

/*
 * Accepts on a socket.
 */
int sock_accept(int sockfd, struct sockaddr *serv_addr, socklen_t *addrlen, int timeout);

/*
 * Sends 'len' bytes if possible.
 */
int sock_send(int sockfd, char *s, int len);

/*
 * Reads at most 'len' bytes.
 */
int sock_read(int sockfd, char *s, int len);

selectfds_t * sock_fd_add(selectfds_t *l, int p, int q);
int sock_fd_select(selectfds_t *fds, long msec);

int sock_data_push(int fd, char *str, int len);

int sock_fd_close(selectfds_t *l);


#endif
