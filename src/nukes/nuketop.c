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
 * NOT FINISHED, DO NOT USE
 */

// #include "nukes.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <collection/sortedlist.h>

#define NUKELOG "/ftp-data/logs/nukelog"
#define DEFAULT_NUM 15

struct nukelog {
        ushort status;
        time_t nuketime;
        char   nuker[12];
        char   unnuker[12];
        char   nukee[12];
        ushort mult;
        float  bytes;
        char   reason[60];
        char   dirname[255];
        struct nukelog *nxt;
        struct nukelog *prv;
};

typedef struct nukelog nukelog_t;

struct nuke_top {
	char *user;

	int nuke_num;
	int nuke_mul_sum;
	float nuke_bytes_sum;
	float nuke_bytes_cred;

	struct nuke_top *next;
};

typedef struct nuke_top nuke_top_t;


nuke_top_t * nuke_top_add(nuke_top_t *l, nukelog_t *e) {
	nuke_top_t *tmp;

	// find user if any.
	for (tmp = l; tmp; tmp = tmp->next)
		if (!strcmp(tmp->user, e->nukee))
			break;

	// add stats if we got one.
	if (tmp) {
		tmp->nuke_num++;
		tmp->nuke_mul_sum += e->mult;
		tmp->nuke_bytes_sum += e->bytes;
		tmp->nuke_bytes_cred += (e->mult * e->bytes);
	}
	// create new entry.
	else {
		tmp = (nuke_top_t*)malloc(sizeof(*tmp));
		tmp->user = strdup(e->nukee);

		tmp->nuke_num = 1;
		tmp->nuke_mul_sum = e->mult;
		tmp->nuke_bytes_sum = e->bytes;
		tmp->nuke_bytes_cred = (e->mult * e->bytes);

		tmp->next = l;
		l = tmp;
	}

	return l;
}


nuke_top_t * find_nukes(time_t from, time_t to) {
	nuke_top_t *l = 0;
	FILE *nl;
	nukelog_t buf;

	nl = fopen(NUKELOG, "rb");

	if (!nl)
		return l;

	while (fread(&buf, sizeof(buf), 1, nl)) {
		if ((from != 0) && (buf.nuketime < from))
			continue;

		if ((to != 0) && (buf.nuketime > to))
			continue;

		// add sum of this entry.
		l = nuke_top_add(l, &buf);
	}
	
	fclose(nl);

	return l;
}

/*
 * Comparator for on bytes.
 */
int nuke_top_sort_bytes(void *p, void *q) {
	nuke_top_t *a, *b;

	a = (nuke_top_t*)p;
	b = (nuke_top_t*)q;

	if (a->nuke_bytes_sum > b->nuke_bytes_sum)
		return 1;

	return 0;
}

int nuke_top_sort_num(void *p, void *q) {
	nuke_top_t *a, *b;

	a = (nuke_top_t*)p;
	b = (nuke_top_t*)q;

	if (a->nuke_num > b->nuke_num)
		return 1;

	return 0;
}

int nuke_top_sort_mul(void *p, void *q) {
	nuke_top_t *a, *b;

	a = (nuke_top_t*)p;
	b = (nuke_top_t*)q;

	if (((float)a->nuke_mul_sum/a->nuke_num) > ((float)b->nuke_mul_sum/b->nuke_num))
		return 1;

	return 0;
}


int nuke_top_sort_cred(void *p, void *q) {
	nuke_top_t *a, *b;

	a = (nuke_top_t*)p;
	b = (nuke_top_t*)q;

	if (a->nuke_bytes_cred > b->nuke_bytes_cred)
		return 1;

	return 0;
}

void print_sorted(nuke_top_t *n, int max, char type) {
	sortedlist_t sorter;
	nuke_top_t *tmp;
	int i;

	sortedlist_init(&sorter);
	for (tmp = n; tmp; tmp = tmp->next)
		sortedlist_add(&sorter, tmp);

	switch (type) {
	case 'n':
		sortedlist_sort(&sorter, nuke_top_sort_num);
		break;

	case 'm':
		sortedlist_sort(&sorter, nuke_top_sort_mul);
		break;

	case 'c':
		sortedlist_sort(&sorter, nuke_top_sort_cred);
		break;

	default:
		sortedlist_sort(&sorter, nuke_top_sort_bytes);
		break;

	}

	sortedlist_reset(&sorter);
	i = 0;
	while (sortedlist_hasnext(&sorter) && (i < max)) {
		nuke_top_t *tmp = (nuke_top_t*)sortedlist_next(&sorter);

		printf(" %3d. | %-12.12s | %5d | %9.1fM | x%4.2f | %9.1fM\n",
			   i+1, tmp->user, tmp->nuke_num,
			   tmp->nuke_bytes_sum,
			   (float)tmp->nuke_mul_sum/tmp->nuke_num,
			   tmp->nuke_bytes_cred);

		i++;
  	}
}


time_t time_start(char p) {
	time_t tmp;
	struct tm *lt;
	int wday;

	tmp = time(0);
	lt = localtime(&tmp);

	switch (p) {

	case 'm':
		lt->tm_mday = 0;
		tmp = mktime(lt);
		break;

	case 'a':
		tmp = 0;
		break;

	default:
		// default is week.
	  //wday = lt->tm_wday;
		lt->tm_wday = 0;
		tmp = mktime(lt);
		// tmp = tmp/(3600*24);
	}

	return tmp;
}


int main(int argc, char *argv[]) {
	nuke_top_t *nukes;
	int num = DEFAULT_NUM;
	time_t end, start;

	if (argc < 3) {
		printf("bad install, syntax; foo-nukes <sorting> <period> [num]\n");
		printf("
sorting: n - number-of-nukes, b - bytes, m - multiplier, c - credits
period : w - week, m - month, a - alltime
num    : some number

Example: site nuketop b w 20
");
		exit(1);
	}

	end = 0;
	start = time_start(argv[2][0]);

	nukes = find_nukes(start, end);

	if (argc > 3)
		num = atoi(argv[3]);

	print_sorted(nukes, num, argv[1][0]);

	return 0;
}
