

#ifndef _records_h
#define _records_h

#include <util/date.h>

// dir for storing stats in ftp-data folder
#define REC_DIR "records"

// for constructing filename
#define REC_GROUP_PREFIX "group_"
#define REC_USER_PREFIX "user_"
#define REC_WEEK "week_"
#define REC_MONTH "month_"
#define REC_DAY "day_"
#define REC_UPLOAD "upload"
#define REC_DOWNLOAD "download"

// for properties of the file
#define REC_KBYTES "kbytes"
#define REC_FILES "files"
#define REC_SECONDS "seconds"
#define REC_USERS "users"
#define REC_RECORDHOLDER "recordholder"
#define REC_DATESET "dateset"




struct stat_record {

	// stats
	double kbytes;
	long files;
	long seconds;

	// the number of users (if group)
	int users;

	// the name of the record holder
	char recordholder[30];

	date_t *dateset;
};

typedef struct stat_record stat_record_t;

/*
 * Fills buf a path to a record file.
 */
int rec_get_recfile(char *buf, char *ftpdatadir, int isgroup, char *period, int isupload);

/*
 * Reads record structure from recfile and returns it.  returns 0 if there was an error.
 */
stat_record_t * rec_find_record(char *recfile);

/*
 * Saves a record structure to recfile
 */
int rec_set_record(char *recfile, stat_record_t *rec);



#endif
