

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
