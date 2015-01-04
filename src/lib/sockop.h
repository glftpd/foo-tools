/*
 * Threadsafe timeout'ing socket operations  /flower.
 *
 * These are the same as the ones without timeout in the manpages.  Timeout values
 * are all in seconds.
 */

#ifndef _sockop_h
#define _sockop_h

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

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
