

#include "date.h"

#include <lib/stringtokenizer.h>
#include <stdlib.h>
#include <time.h>

char DATE_MDAY[][4] = {
	{ "Sun" },
	{ "Mon" },
	{ "Tue" },
	{ "Wed" },
	{ "Thu" },
	{ "Fri" },
	{ "Sat" }
};

char DATE_MON[][4] = {
	{ "Jan" },
	{ "Feb" },
	{ "Mar" },
	{ "Apr" },
	{ "May" },
	{ "Jun" },
	{ "Jul" },
	{ "Aug" },
	{ "Sep" },
	{ "Oct" },
	{ "Nov" },
	{ "Dec" }
};

char DATE_OUT[1024];

// Tue Jan  1 16:06:20 2002
date_t * date_parse_unix(char *d) {
	stringtokenizer st, mst;
	char *buf, *tmp, *off;
	int i;
	date_t *da;
	
	// make string tokenizable
	buf = malloc(strlen(d) + 1);
	off = buf;

	*buf = 0;
	tmp = d;

	while (*tmp) {
		if (*tmp == ' ') {
			*(buf++) = ' ';
			tmp++;

			while (*tmp == ' ')
				tmp++;
		} else
			*(buf++) = *(tmp++);
	}
	*buf = 0;


	st_initialize(&st, off, " ");
	free(off);

	if (st_count(&st) < 5) {
		st_finalize(&st);
		return 0;
	}

	da = malloc(sizeof(date_t));

	// wday.
	tmp = st_next(&st);
	da->wday = -1;
	for (i = 0; i < 7; i++) {
		if (!strcmp(tmp, DATE_MDAY[i]))
			da->wday = i;
	}

	// mon
	tmp = st_next(&st);
	da->mon = -1;
	for (i = 0; i < 12; i++) {
		if (!strcmp(tmp, DATE_MON[i]))
			da->mon = i;
	}

	// sanity check
	if ((da->mon == -1) || (da->wday == -1)) {
		free(da);
		st_finalize(&st);

		return 0;
	}
		
	// mday.
	tmp = st_next(&st);
	da->mday = atoi(tmp);

	// hour:min:sec
	tmp = st_next(&st);
	st_initialize(&mst, tmp, ":");

	if (st_count(&mst) != 3) {
		st_finalize(&mst);
		st_finalize(&st);
		free(da);

		return 0;
	}

	da->hour = atoi(st_next(&mst));
	da->min = atoi(st_next(&mst));
	da->sec = atoi(st_next(&mst));

	st_finalize(&mst);

	// year
	tmp = st_next(&st);
	da->year = atoi(tmp);

	st_finalize(&st);

	return da;
}

char * date_tostring(date_t *d, char *fmt) {
	char *smday, *smon, *buf;

	buf = malloc(50);

	sprintf(buf, "%s %s %2d %02d:%02d:%02d %04d",
			DATE_MDAY[d->wday],
			DATE_MON[d->mon],
			d->mday,
			d->hour,
			d->min,
			d->sec,
			d->year);

	return buf;
}

int date_equals(date_t *d, date_t *q) {
	if ((d == 0) || (q == 0))
		return 0;

	if ((d->wday == q->wday) && 
		(d->mon == q->mon) &&
		(d->mday == q->mday) &&
		(d->min == q->min) &&
		(d->hour == q->hour) &&
		(d->sec == q->sec) &&
		(d->year == q->year))
		return 1;

	return 0;
}

int date_before(date_t *d, date_t *q) {
	if ((d == 0) || (q == 0))
		return 0;

	// check year.
	if (d->year < q->year)
		return 1;
	else if (d->year > q->year)
		return 0;

	// now we know that year must be the same, so we dont need to
	// check that anymore.
	if (d->mon < q->mon)
		return 1;
	else if (d->mon > q->mon)
		return 0;

	if (d->mday < q->mday)
		return 1;
	else if (d->mday > q->mday)
		return 0;

	if (d->hour < q->hour)
		return 1;
	else if (d->hour > q->hour)
		return 0;

	if (d->min < q->min)
		return 1;
	else if (d->min > q->min)
		return 0;

	if (d->sec < q->sec)
		return 1;
	else if (d->min > q->min)
		return 0;

	return 0;
}

int date_after(date_t *d, date_t *q) {
	if ((d == 0) || (q == 0))
		return 0;


	// check year.
	if (d->year > q->year)
		return 1;
	else if (d->year < q->year)
		return 0;

	// now we know that year must be the same, so we dont need to
	// check that anymore.
	if (d->mon > q->mon)
		return 1;
	else if (d->mon < q->mon)
		return 0;

	if (d->mday > q->mday)
		return 1;
	else if (d->mday < q->mday)
		return 0;

	if (d->hour > q->hour)
		return 1;
	else if (d->hour < q->hour)
		return 0;

	if (d->min > q->min)
		return 1;
	else if (d->min < q->min)
		return 0;

	if (d->sec > q->sec)
		return 1;
	else if (d->min < q->min)
		return 0;


	return 0;
}


/*
 *
 */
void date_makeage(time_t t, time_t age, char *buf) {
	time_t days=0,hours=0,mins=0,secs=0;
	
	if (t>=age)
		sprintf(buf,"0m 0s");
	else {
		age-=t;
		days=age/(3600*24);
		age-=days*3600*24;
		hours=age/3600;
		age-=hours*3600;
		mins=age/60;
		secs=age-(mins*60);
		
		if (days)
			sprintf(buf,"%dd %dh",days,hours);
		else if (hours)
			sprintf(buf,"%dh %dm",hours,mins);
		else
			sprintf(buf,"%dm %ds",mins,secs);
	}
}

