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
 * A fixed-size threadpool.
 *
 * <sorend@tanesha.net>
 */

#ifndef _threadpool_h
#define _threadpool_h

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define TP_DEF_MIN 5
#define TP_DEF_MAX 10

#define TP_ERROR_FULL -1

struct threadpool_runner {
	// ptr to the pool where thread belongs.
	struct threadpool *pool;

	pthread_t thread;
	pthread_mutex_t mutex;
	pthread_cond_t cond;

	void *request;

	struct threadpool_runner *next;
};

typedef struct threadpool_runner threadpool_runner_t;

struct threadpool {
	// min and max threads to have.
	int min, max;

	// proc to run.
	void (*thread_proc)(void *args);

	pthread_mutex_t mutex;

	threadpool_runner_t *runners;
};

typedef struct threadpool threadpool_t;


/*
 *
 */
int threadpool_initialize(threadpool_t *pool, int min, int max);

/*
 * Sets the thread-proc to use for new requests to pool
 */
int threadpool_set_proc(threadpool_t *pool, void (*proc)(void *args));

/*
 * Gives work to a runner thread.
 */
int threadpool_set_work(threadpool_t *pool, void *arg);


#endif
