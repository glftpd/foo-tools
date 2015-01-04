
#ifndef _foopre_h
#define _foopre_h

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define PRE_CONFIGFILE "/etc/pre.cfg"
#define PRE_LOGFILE "/ftp-data/logs/foo-pre.log"

#define PROPERTY_USER "user"
#define PROPERTY_TAGLINE "tagline"
#define PROPERTY_USERGROUP "usergroup"
#define PROPERTY_GROUP "group"

#define PROPERTY_GL_USERDIR "userdir"
#define PROPERTY_SITEDIR "sitedir"

#define PROPERTY_TEXT_HEAD "text.head"
#define PROPERTY_TEXT_TAIL "text.tail"

#define PROPERTY_GROUP_DIR "dir"
#define PROPERTY_GROUP_ALLOW "allow"

#define PROPERTY_GROUP_GL_CS "gl_credit_section"
#define PROPERTY_GROUP_GL_SS "gl_stat_section"
#define PROPERTY_GROUP_RATIO "ratio"
#define PROPERTY_GROUP_ANNOUNCE "announce"

#define PROPERTY_SECTION_DIR "dir"
#define PROPERTY_SECTION_NAME "name"

#define PROPERTY_GROUP_CHOWN_USER "chown.user"
#define PROPERTY_GROUP_CHOWN_GROUP "chown.group"

#define PROPERTY_COUNTABLE "countable"
#define PROPERTY_CREDITABLE "creditable"
#define PROPERTY_ADDSUB "addsubdirstodirlog"


/*
 * Holds info about chowns
 */
struct chowninfo {
	int uid;
	int gid;
};

typedef struct chowninfo chowninfo_t;

/*
 * Holds list of relative filenames.
 */
struct filelist {
	char file[300];
	struct stat st;
	char uname[30];

	struct filelist *next;
};

typedef struct filelist filelist_t;

struct creditlist {
	int uid;
	long bytes;
	long files;

	struct creditlist *next;
};

typedef struct creditlist creditlist_t;

struct subdir_list {
	char dir[500];
	ushort files;
	long bytes;

	struct subdir_list *next;
};

typedef struct subdir_list subdir_list_t;



#endif
