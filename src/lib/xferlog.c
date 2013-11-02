

#include "xferlog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lib/stringtokenizer.h>

#define BUFSIZE 1024

int _xferlog_parse(char *buf, xferlog_t *log) {
	stringtokenizer st;
	char *tmp;

	if (strlen(buf) < 25)
		return 0;

	log->xfer_date = date_parse_unix(buf);

	if (!log->xfer_date)
		return 0;

	st_initialize(&st, buf + 25, " ");

	if (st_count(&st) < 12) {
		free(log->xfer_date);
		return 0;
	}

	log->xfer_duration = atoi(st_next(&st));
	log->xfer_host = strdup(st_next(&st));
	log->xfer_size = atol(st_next(&st));
	log->xfer_file = strdup(st_next(&st));
	st_next(&st);
	st_next(&st);
	tmp = st_next(&st);
	log->xfer_direction = *tmp;
	st_next(&st);
	log->xfer_user = strdup(st_next(&st));
	log->xfer_group = strdup(st_next(&st));
	st_next(&st);
	log->xfer_ident = strdup(st_next(&st));

	return 1;
}

void xferlog_free(xferlog_t *log) {
	free(log->xfer_host);
	free(log->xfer_file);
	free(log->xfer_user);
	free(log->xfer_group);
	free(log->xfer_ident);
}

xferlog_t * xferlog_clone(xferlog_t *log) {
	xferlog_t *n = malloc(sizeof(xferlog_t));

	n->xfer_date = malloc(sizeof(date_t));
	memcpy(n->xfer_date, log->xfer_date, sizeof(date_t));

	n->xfer_duration = log->xfer_duration;
	n->xfer_host = strdup(log->xfer_host);
	n->xfer_file = strdup(log->xfer_file);
	n->xfer_user = strdup(log->xfer_user);
	n->xfer_group = strdup(log->xfer_group);
	n->xfer_ident = strdup(log->xfer_ident);

	n->xfer_size = log->xfer_size;
	n->xfer_direction = log->xfer_direction;

	return n;
}

long xferlog_read(char *file, int (*handler)(xferlog_t *item)) {
	FILE *f;
	char buf[BUFSIZE];
	int rc;
	long entries = 0;
	xferlog_t tmplog;

	f = fopen(file, "r");

	if (!f)
		return 0;

	while (fgets(buf, BUFSIZE, f)) {

		if (!_xferlog_parse(buf, &tmplog))
			continue;

		rc = handler(&tmplog);

		xferlog_free(&tmplog);

		if (rc == 0)
			break;

		entries++;
	}
	fclose(f);

	return entries;
}
