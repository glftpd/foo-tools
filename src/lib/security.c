/*
 * Check security.h ;)
 *
 * $Id: security.c,v 1.1.1.1 2001/04/30 10:49:37 sd Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "security.h"
#include "security_ips.h"

int get_local_ip(char *ip) {
	int sockfd;
	struct ifreq ifr;
	struct sockaddr_in *sa = (struct sockaddr_in *)&ifr.ifr_addr;
	
	if (0 > (sockfd = socket(AF_INET, SOCK_STREAM, 0)))
		return 0;
	
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, NETINTERFACE, IFNAMSIZ - 1);
	sa->sin_family = AF_INET;
	
	if (!ioctl(sockfd, SIOCGIFADDR, &ifr)) {
		strcpy(ip, inet_ntoa(sa->sin_addr));

		return 1;
	}
	
	return 0;
}

int is_secure_ip(char *ip) {
	int i = 0;
	struct in_addr addr;

	if (!inet_aton(ip, &addr))
		return 0;

#ifdef DEBUG
	printf("ip: %lu\n", addr.s_addr);
#endif

	while (ips[i].ip != 0) {
		if (addr.s_addr == ips[i].ip)
			break;

		i++;
	}

	if (ips[i].ip > 0)
		return 1;

	return 0;
}
