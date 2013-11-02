


#ifndef _spy_h
#define _spy_h

#include "../lib/who.h"

struct spy_list {
	int pos;
	struct ONLINE *user;

	struct spy_list *next, *prev;
};

typedef struct spy_list spy_list_t;

struct spy_status {
	int ul_num, dl_num;

	double ul_speed, dl_speed;
	int online, max;

	int max_ul_num, max_dl_num;
	double max_ul_speed, max_dl_speed;

	int mode;
	char glroot[300];
};

typedef struct spy_status spy_status_t;

#define MODE_USERLIST 1
#define MODE_DIRLIST 2

#endif
