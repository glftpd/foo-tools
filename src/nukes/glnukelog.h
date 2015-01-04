/*
 * Tanesha Team! $Id: glnukelog.h,v 1.3 2002/03/15 23:34:36 flower Exp $
 */

#ifndef _glnukelog_h
#define _glnukelog_h

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <collection/sortedlist.h>



struct glnukelog {
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

typedef struct glnukelog glnukelog_t;

/**
 * Universan nukee log structure.
 */
struct nukeelog {
	char *nukee;
	float bytes;
};

typedef struct nukeelog nukeelog_t;

/**
 * Universal nuke structure.
 */
struct nukelog {
	int status;
	char *dirname;
	float bytes;
	int mult;
	char *nuker;

	char *reason;
	time_t nuketime;

	sortedlist_t nukees;
};

typedef struct nukelog nukelog_t;

/*
 * Will load nukelog into a hashtable, hashtable has 
 * dirname as key, and an iterator should give the
 * nuked releases in chronological order.
 */
int nukelog_load(sortedlist_t *collection, char *filename);

#endif
