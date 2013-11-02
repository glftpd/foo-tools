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


#include "threadpool.h"
#include "../bouncer/datanode_msg.h"

struct tmp_t {
	int blub;
};

/*
 * our function to get executed.
 */
void do_something(void *args) {
	struct tmp_t *tmp;

	tmp = (struct tmp_t*)args;

	printf("doing something, blub = %d\n", tmp->blub);

	return;
}



int main(int argc, char *argv[]) {
	int i;
	threadpool_t p;
	struct tmp_t *tmp;
	
	threadpool_initialize(&p, 5, 10);
	threadpool_set_proc(&p, do_something);

	for (i = 0; i < 12; i++) {
		tmp = (struct tmp_t*)malloc(sizeof(struct tmp_t));
		tmp->blub = 2^i;

		if (threadpool_set_work(&p, tmp) < 0)
			printf("Error adding to pool .. %d\n", i);
	}

	sleep(10);
}
