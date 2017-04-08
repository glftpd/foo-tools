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


#ifndef _date_h
#define _date_h

#include <time.h>
#include <stdio.h>
#include <string.h>

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
date_t * date_parse_timet(time_t d);

char * date_tostring(date_t *d, char *fmt);

int date_equals(date_t *obj, date_t *q);
int date_before(date_t *obj, date_t *q);
int date_after(date_t *obj, date_t *q);


/*
 * static method used in alot of places.
 */
void date_makeage(time_t t, time_t age, char *buf);

void date_makeage_delim(time_t t, time_t age, char *buf, char *delim);


#endif
