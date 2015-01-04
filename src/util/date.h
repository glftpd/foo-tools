

#ifndef _date_h
#define _date_h

#include <time.h>

struct date {
	int mon;
	int mday;
	int year;

	int wday;

	int hour;
	int min;
	int sec;
};

typedef struct date date_t;


date_t * date_parse_unix(char *d);

char * date_tostring(date_t *d, char *fmt);

int date_equals(date_t *d, date_t *q);
int date_before(date_t *d, date_t *q);
int date_after(date_t *d, date_t *q);


/*
 * static method used in alot of places.
 */
void date_makeage(time_t t, time_t age, char *buf);


#endif
