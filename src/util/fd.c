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
#include <unistd.h>
#include "fd.h"

int fd_copy(int to, int from) {
	if (from == to)
		return 0;

	if (fcntl(from, F_GETFL, 0) == -1)
		return -1;

	close(to);

	if (fcntl(from, F_DUPFD, to) == -1)
		return -1;

	return 0;
}

int fd_move(int to, int from) {
	if (from == to)
		return 0;

	if (fd_copy(to, from) == -1)
		return -1;

	close(from);

	return 0;
}
