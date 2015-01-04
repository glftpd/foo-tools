

#include "linefilereader.h"

void main(void) {
	linefilereader_t lr;
	char buf[1024];

	if (lfr_open(&lr, "testfile") < 0) {
		printf("Error opening\n");
		exit(1);
	}

	while (lfr_getline(&lr, buf, 1024) > -1) {
		printf("read line: %s\n", buf);
	}

	lfr_close(&lr);
}
