
#include <dirent.h>
#include "race.h"

typedef struct race_count_t = {
	char pattern[30];
} race_count[] = {
	"*.[r0123456789][a0123456789][r0123456789]",
	"*.mp3",
	"*.nfo",
	"*.sfv",
	"*.mpg",
	"*.avi",
	0
};


int race_is_countable(char *file, struct stat *st) {
	int i;

	if (st && !S_ISREG(st->st_mode))
		return 0;

	if (file)
		for (i = 0; race_count[i].pattern != 0; i++)
			if (!fnmatch(race_count[i].pattern, file, 0))
				return 1;

	return 0;
}

int race_init(racelist *race) {
	race->user = 0;
	race->group = 0;
	race->speeds = 0;

	return 1;
}

race_speedlist * _race_load_speedlist(char *dir) {
	FILE *f;
	char *buf;
	race_speedlist *l = 0;

	buf = (char*)malloc(strlen(dir) + strlen(RACEFILE) + 2);
	sprintf(buf, "%s/%s", dir, RACEFILE);

	f = fopen(buf, "r");

	free(buf);

	if (!f)
		return 0;

	while (fread(&tmp, sizeof(tmp), 1, f)) {


	}

}

int race_load_dir(char *dir, racelist *race) {
	DIR *dh;
	struct dirent *dent;

	race->speeds = 

	dh = opendir(dir);

	if (!dh)
		return 0;

	while (dent = readdir(dh)) {


	}

	return 1;
}
