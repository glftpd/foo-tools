
#ifndef _xferlog_h
#define _xferlog_h

#include <util/date.h>

struct xferlog {
	date_t *xfer_date;
	int xfer_duration;
	char *xfer_host;
	long xfer_size;
	char *xfer_file;
	// char xfer_mode
	// char ??
	char xfer_direction;
	// char ??
	char *xfer_user;
	char *xfer_group;
	// int ??
	char *xfer_ident;
};

typedef struct xferlog xferlog_t;

long xferlog_read(char *file, int (*handler)(xferlog_t *item));

xferlog_t * xferlog_clone(xferlog_t *log);
void xferlog_free(xferlog_t *log);


#endif
