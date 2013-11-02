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
 * $Id: security_ips.h,v 1.2 2003/01/22 14:31:29 sorend Exp $
 */

/*
 * The list of local ips.
 *
 * If you need to get the number for some ip, then compile the module with DEBUG=1
 * and it will output the number of the ip that its trying to match.
 */
struct ip_t {
	unsigned long ip;
} ips [] = {
	// 10.0.0.3 (sorends local lan box).
	50331658,
	0
};

