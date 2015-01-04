

#include "date.h"



int main(int argc, char *argv[]) {

	date_t *d1, *d2;

	d1 = date_parse_unix("Wed Jan  9 10:22:43 2002");
	d2 = date_parse_unix("Wed Jan  9 10:22:53 2002");

	if (date_before(d1, d2))
		printf("%s before\n", date_tostring(d1, ""));
}
