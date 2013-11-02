/*
 * A fixed-size threadpool.
 *
 * <flower@tanesha.net>
 */

#ifndef _threadpool_h
#define _threadpool_h

#include <pthread.h>

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
