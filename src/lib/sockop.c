
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
		 return 0;

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
