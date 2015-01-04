/*
 * Library for reading passwd files.
 */

/*
 * passwd structure.
 */
struct pwdfile_t {
	char name[30];
	char pass[30];

	int uid;
	int gid;

	char longname[50];
	char homedir[300];
	char shell[300];

	struct pwdfile_t *next;
};

typedef struct pwdfile_t pwdfile;

struct grpfile {
	char group[30];
	char pass[30];

	int gid;

	char *users;

	struct grpfile *next;
};

typedef struct grpfile grpfile_t;

/*
 * Location of passwd file.
 */
#define PASSWDFILE "/etc/passwd"
#define GROUPFILE "/etc/group"

/*
 * getpwnam method.
 */
pwdfile *pwd_getpwnam(char *u);

/*
 * getpwuid method.
 */
pwdfile *pwd_getpwuid(int uid);


grpfile_t *pwd_getgpnam(char *g);

grpfile_t *pwd_getgpgid(int gid);
