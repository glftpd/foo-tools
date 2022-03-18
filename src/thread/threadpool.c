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

#include <unistd.h>
#include "threadpool.h"

/*
 * A sample proc that does absolutely nothing :)
 */
void threadpool_donothing_proc(void *args) {
	printf("Doing nothing..\n");

	sleep(1);

	printf("Done doing nothing ..\n");

	return;
}


static void _threadpool_cleanup(void *m) {

}

/*
 * gets number of runners in pool.
 */
int _threadpool_runner_count(threadpool_runner_t *r) {
	if (r)
		return 1 + _threadpool_runner_count(r->next);
	else
		return 0;
}

/*
 * gets a free runner.
 */
threadpool_runner_t * _threadpool_get_free(threadpool_t *pool) {
	threadpool_runner_t *tmp;

	for (tmp = pool->runners; tmp; tmp = tmp->next)
		if (tmp->request == 0)
			break;

	return tmp;
}

/*
 * waiting loop.
 */ 
void *_threadpool_wait_loop(void *args) {
	threadpool_runner_t *ctx;

	ctx = (threadpool_runner_t*)args;

	// set some thread params.
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	// free mutex if smth happens.
	pthread_cleanup_push(_threadpool_cleanup, (void*)&ctx->mutex);

	pthread_mutex_lock(&ctx->mutex);

	while (1) {

		if (ctx->request) {

			pthread_mutex_unlock(&ctx->mutex);

			// call the pool threadproc with the request.
			ctx->pool->thread_proc(ctx->request);

			// remove the request from pool again.
			pthread_mutex_lock(&ctx->pool->mutex);
			free(ctx->request);
			ctx->request = 0;
			pthread_mutex_unlock(&ctx->pool->mutex);

			pthread_mutex_lock(&ctx->mutex);

		} else
			// no req yet, wait for notification.
			pthread_cond_wait(&ctx->cond, &ctx->mutex);
	}

	pthread_cleanup_pop(0);

	return 0;
}


threadpool_runner_t * _threadpool_runner_add(threadpool_t *pool) {
	threadpool_runner_t *r = (threadpool_runner_t*)malloc(sizeof(threadpool_runner_t));

#ifdef DEBUG
	printf("Threadpool -> Adding runner [pool-size:%d+1]\n", _threadpool_runner_count(pool->runners));
#endif

	r->pool = pool;
	r->request = 0;

	pthread_mutex_init(&r->mutex, 0);
	pthread_cond_init(&r->cond, 0);

	r->next = pool->runners;

	pool->runners = r;

	pthread_create(&r->thread, 0, _threadpool_wait_loop, r);

	return r;
}


/*
 *
 */
int threadpool_initialize(threadpool_t *pool, int min, int max) {
	int i;

	pool->runners = 0;
	pool->min = min;
	pool->max = max;
	pool->thread_proc = threadpool_donothing_proc;

	pthread_mutex_init(&pool->mutex, 0);

	// start up the min runners.
	for (i = 0; i < min; i++)
		_threadpool_runner_add(pool);
}

/*
 * Sets the thread-proc to use for new requests to pool
 */
int threadpool_set_proc(threadpool_t *pool, void (*proc)(void *args)) {
	pool->thread_proc = proc;
}

/*
 * Gives work to a runner thread.
 */
int threadpool_set_work(threadpool_t *pool, void *arg) {
	threadpool_runner_t *r;

#ifdef DEBUG
	printf("Threadpool -> Adding work ..\n");
#endif

	pthread_mutex_lock(&pool->mutex);

	r = _threadpool_get_free(pool);

	// if no free runner, but pool is not full, then create new runner.
	if (!r) {
		if (_threadpool_runner_count(pool->runners) < pool->max)
			r = _threadpool_runner_add(pool);
		else {
			pthread_mutex_unlock(&pool->mutex);

			return TP_ERROR_FULL;
		}
	}

	r->request = arg;

	pthread_mutex_unlock(&pool->mutex);

#ifdef DEBUG
	printf("Threadpool -> Waking up a sleeping runner [req:%lu]\n", arg);
#endif

	// signal that there is work for this runner.
	pthread_cond_signal(&r->cond);

	return 1;
}

int threadpool_get_busy(threadpool_t *pool) {

	return 0;
}
