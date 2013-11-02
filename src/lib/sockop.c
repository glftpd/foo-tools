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

#include <fcntl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "sockop.h"

#include <errno.h>

#ifndef _errno
#define _errno

extern int errno;

#endif

int init_sockaddr(struct sockaddr_in *name,
				  const char *hostname,
				  unsigned short int port) {

     struct hostent *hostinfo;

     name->sin_family = AF_INET;
     name->sin_port = htons(port);
	 
	 hostinfo = gethostbyname(hostname);

	 if (!hostinfo && inet_aton(hostname, &name->sin_addr))
		 return 1;

	 name->sin_addr = *(struct in_addr*)hostinfo->h_addr;
	 
     return 1;
}

int sock_connect(int sfd, struct sockaddr *serv_addr, socklen_t addrlen, int timeout) {
	int res, slen, flags;
	struct timeval tv;
	struct sockaddr_in addr;
	fd_set rdf, wrf;

	fcntl(sfd, F_SETFL, O_NONBLOCK);

	res = connect(sfd, serv_addr, addrlen);

	if (res >= 0)
		return res;

	FD_ZERO(&rdf);
	FD_ZERO(&wrf);

	FD_SET(sfd, &rdf);
	FD_SET(sfd, &wrf);

	bzero(&tv, sizeof(tv));
	tv.tv_sec = timeout;

	if (select(sfd + 1, &rdf, &wrf, 0, &tv) <= 0)
		return -1;

	if (FD_ISSET(sfd, &wrf) || FD_ISSET(sfd, &rdf)) {
		slen = sizeof(addr);

		if (getpeername(sfd, (struct sockaddr*)&addr, &slen) == -1)
			return -1;

		// reset non-blocking.
		flags = fcntl(sfd, F_GETFL, NULL);
		fcntl(sfd, F_SETFL, flags & ~O_NONBLOCK);

		return 0;
	}

	return -1;
}

int sock_accept(int sfd, struct sockaddr *serv_addr, socklen_t *addrlen, int timeout) {
	int res, slen, flags;
	struct timeval tv;
	struct sockaddr_in addr;
	fd_set rdf, wrf;
	
	fcntl(sfd, F_SETFL, O_NONBLOCK);
	
	FD_ZERO(&rdf);
	FD_ZERO(&wrf);
	
	FD_SET(sfd, &rdf);
	FD_SET(sfd, &wrf);

	bzero(&tv, sizeof(tv));
	tv.tv_sec = timeout;

	if (select(sfd + 1, &rdf, &wrf, 0, &tv) <= 0)
		return -1;
	
	if (FD_ISSET(sfd, &wrf) || FD_ISSET(sfd, &rdf)) {
		slen = sizeof(addr);

		res = accept(sfd, serv_addr, addrlen);

		// reset non-blocking.
		flags = fcntl(sfd, F_GETFL, NULL);
		fcntl(sfd, F_SETFL, flags & ~O_NONBLOCK);

		return res;
	}

	return -1;
}

/*
 * For weird backward compatiblities.
 */
int sock_send(int fd, char *str, int len) {

	return sock_data_push(fd, str, len);
}

int sock_read(int fd, char *str, int len) {
	int nbytes;
	
	nbytes = read(fd, str, len);
	
	return nbytes;
}


int sock_fd_select(selectfds_t *fds, long msec) {
	fd_set rfds;
	selectfds_t *tmp;
	int fd_max = -1, sel, num = 0;
	struct timeval tv;

 restart_select:

	bzero(&tv, sizeof(tv));
	tv.tv_sec = msec/1000;
	tv.tv_usec = (msec%1000)*1000;

	FD_ZERO(&rfds);

	// add the fds to the set.
	for (tmp = fds; tmp; tmp = tmp->next) {
		FD_SET(tmp->rfd, &rfds);

		if (tmp->rfd > fd_max)
			fd_max = tmp->rfd;
	}

	// wait for some input.
	sel = select(fd_max + 1, &rfds, 0, 0, &tv);

	if (sel <= 0) {
#ifdef BSD
		// threaded environment on fbsd -> all threads gets notified on select.
		if ((sel < 0) && (errno == EINTR))
			goto restart_select;
#endif

		return sel;
	}

	sel = -1;

	// set flags in struct.
	for (tmp = fds; tmp; tmp = tmp->next) {
		if (FD_ISSET(tmp->rfd, &rfds)) {
			tmp->has_data = 1;
			sel = 1;
		} else
			tmp->has_data = 0;
	}
	
	return sel;
}


selectfds_t * sock_fd_add(selectfds_t *l, int p, int q) {
	selectfds_t *tmp = (selectfds_t*)malloc(sizeof(selectfds_t));

	tmp->rfd = p;
	tmp->wfd = q;
	tmp->has_data = 0;

	tmp->next = l;

	return tmp;
}



int sock_data_push(int fd, char *str, int len) {
	int nleft, nwritten;

	nleft = len;

	while (nleft > 0) {
		nwritten = write(fd, str, nleft);

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

 int sock_fd_close(selectfds_t *l) {
	 selectfds_t *tmp;

	 while (l) {
		 tmp = l;

		 l = l->next;

		 free(tmp);
	 }

 }
