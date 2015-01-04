/*
 * Library to handle adding/updating some glftpd logfiles.
 *
 * $Id: gllogs.h,v 1.1.1.1 2001/04/30 10:49:37 sd Exp $
 * Maintained by: Flower
 */

#include <fcntl.h>
#include <time.h>

// define some locations for where to find glftpd logs.
#define DUPEFILE "/ftp-data/logs/dupefile"
#define DUPELOG "/ftp-data/logs/dupelog"
#define DIRLOG "/ftp-data/logs/dirlog"
#define GLFTPDLOG "/ftp-data/logs/glftpd.log"
#define GLMSGPATH "/ftp-data/msgs"

typedef unsigned short ushort;
struct dupefile {
    char filename[256];
    time_t timeup;
    char uploader[25];
};

struct dirlog {
    ushort status;     // 0 = NEWDIR, 1 = NUKE, 2 = UNNUKE, 3 = DELETED
    time_t uptime;
    ushort uploader;    /* Libc6 systems use ushort in place of uid_t/gid_t */
    ushort group;
    ushort files;
    long bytes;
    char dirname[255];
    struct dirlog *nxt;
    struct dirlog *prv;
};

/*
 * Adds an entry to the dupefile log.
 */
int gl_dupefile_add(char *fn, char *u);

/*
 * Adds an entry to dirlog (faster than dirlog, but only use if no entry in dirlog already).
 */
int gl_dirlog_add(char *dn, ushort uid, ushort gid, ushort files, long bytes);

/*
 * Update an entry in the dirlog (adds if it doesnt exist already).
 *
 * Note: status == -1 -> leave status flag untouched.
 *
 * Example: gl_dirlog_update("/site/games/Hello-OKE", 10, 100, files+1, bytes+100, -1);
 */
int gl_dirlog_update(char *dn, ushort uid, ushort gid, ushort files, long bytes, int status);

/*
 * Adds an entry to the dupelog.
 *
 * Example: gl_dupelog_add("Supa.Release.v1.0-Hello");
 */
int gl_dupelog_add(char *rel);

/*
 * Put something in glftpd.log
 *
 * Example: gl_gllog_add("My program is still alive");
 */
int gl_gllog_add(char *str);

/*
 * Special method to add a string to be announced.
 *
 * Example: gl_gllog_announce("CUSTOM", "Hello world");
 */
int gl_gllog_announce(char *type, char *str);

/*
 * Send a sitemsg to another user.
 */
int gl_site_msg(char *from, char *to, char *msg);
