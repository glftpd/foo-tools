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
 * Check security.h ;)
 *
 * $Id: security.c,v 1.2 2003/01/22 14:31:29 sorend Exp $
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
