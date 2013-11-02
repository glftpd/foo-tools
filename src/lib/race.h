/*
 * Standard methods for fetching race-lists from a dir.
 */

#include <sys/stat.h>

#define RACEFILE ".tanesha"
#define MAXFILELEN 256
#define MAXDIRLEN 300

struct racelist_file_t {
	char file[MAXFILELEN];
	long speed; // -1 if the race is over.

	struct racelist_file_t *next;
};

typedef struct racelist_file_t racelist_file;

struct racelist_item_t {
	char name[30];

	racelist_file *files;

	struct racelist_item_t *next;
	struct racelist_item_t *prev;
};

struct race_speedlist_t {
	char name[30];
	char file[MAXFILELEN];
	char dir[MAXDIRLEN];
	long speed;

	struct race_speedlist_t *next;
}

typedef struct race_speedlist_t race_speedlist;
typedef struct racelist_item_t racelist_item;

struct racelist_t {
	racelist_item *user;
	racelist_item *group;

	race_speedlist *speeds;
};

typedef struct racelist_t racelist;

/*
 * Figure if a file is countable in races.
 */
int race_is_countable(char *file, struct stat *st);

/*
 * Load race-info for a dir.
 */
int race_load_dir(char *dir, racelist *race);

/*
 * Use this before using a race-struct.
 */
int race_init(racelist *race);
