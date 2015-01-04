

#include "xferlog.h"


int xfers_print(xferlog_t *tmp) {
	// noop.
	return 1;
}



int main(int argc, char *argv) {
	printf("read %d entries.\n", xferlog_read("xferlog", xfers_print));
}
