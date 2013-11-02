/*

Version 3.1 
   - add support for bigger files

Version 3.0 modified by mcr 2004-01-06

   - Command line parsing with getopts()
     (ie: this version requires GNU getopts or a compatible implementation)
   - You can now specify an unlimited number of dirs to skip with -i <dir>
     by default, ".", ".." and "groups" are skipped.
   - Strip leading slash from dir name.
   - Fixed alot of old crummy code :) it now compiles with "-pedantic"

Version 2.2 modified by Bloody_A 06/05/02

   - Made it a lil but more dumb-user proof :)

Version 2.1 modified by Usurper 12/11/01

   - Changed the 'bytes' variable in dirlog sturct to unsigned long, just like
     glftpd has it

Version 2.0 modified by Usurper 11/12/99

   1. Will now accept the -r switch if you're not using the standard
      /etc/glftpd.conf
   2. Fully recursive, will count subdirectories when counting the size of
      directories that it's updating
   3. Will not strip the leading '/' if your rootpath is set to "/"

Note: glupdate does not recursively add directories. It only adds
directories that are in the path you specify; directories under that are
ignored. Example: glupdate /glftpd/site/incoming will add
/glftpd/site/incoming/blah, along with other directories under incoming,
but not /glftpd/site/incoming/blah/bleh

*/

#include <sys/file.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include "glconf.h"

static char rootpath[MAXPATHLEN+1];
static char datapath[MAXPATHLEN+1];
struct ignore *START = NULL;

struct ignore
{
	char *string;
	struct ignore *next;
};

void usage (void);
void load_sysconfig (char *);
void update_log (struct dirlog);
void get_dir_size (char *, int *, unsigned long long *);
char *trim (char *);
void add_ignore(char *);


int
main (int argc, char **argv)
{
	DIR *dirf;
	struct dirent *dn;
	struct stat st;
	struct dirlog log;
	char temppath[MAXPATHLEN];
	char nambuf[MAXPATHLEN];
	unsigned long long bytes;
	int files;
	int i, c;
	char *config_file = GLCONF;
	struct ignore *ignore = START;
	int skip;


	/* These entrys are always ingored. */
	add_ignore(".");
	add_ignore("..");
	add_ignore("groups");


	/* Parse command line options */
	while((c = getopt(argc, argv, "hi:r:")) != EOF) {
		switch(c) {
			case 'h':
				usage();
			case 'i':
				add_ignore(strdup(optarg));
				break;
			case 'r':
				config_file = strdup(optarg);
				break;
			default:
				usage();
			}
	}

	argc -= optind;
	argv += optind;

	if (argc == 0)
		usage();

	load_sysconfig(config_file);

	strncpy(nambuf, argv[0], sizeof(nambuf));

	i = strlen(nambuf) - 1;
	if (nambuf[i] == '/')
		nambuf[i] = '\0';


	dirf = opendir(nambuf);
	if (dirf == NULL) {
		printf ("Unable to read from directory: %s\n", nambuf);
		exit (1);
	}


	for (;;) {
		dn = readdir(dirf);
		if (dn == NULL)
			break;

		if (strstr(dn->d_name, "NUKED") != NULL) {
			printf("Skipping NUKED dir: %s\n", dn->d_name);
			continue;
		}
		ignore = START;
		skip = 0;
		for (;;) {
			if (!strcmp(dn->d_name, ignore->string)) {
				printf("Skipping dir: %s\n", dn->d_name);
				skip=1;
				break;
			}
			if(ignore->next == NULL)
				break;
				ignore = ignore->next;
		}
		if (skip==1) continue;

		snprintf(temppath, MAXPATHLEN, "%s/%s", nambuf, dn->d_name);
		stat(temppath, &st);
		if (S_ISDIR (st.st_mode)) {
			    log.status = 0;
			    log.uptime = (time_t) st.st_mtime;
			    log.uploader = (unsigned short int) st.st_uid;
			    log.group = (unsigned short int) st.st_gid;
			    strncpy (log.dirname, temppath, sizeof(log.dirname) - 1);
			    bytes = 0ULL;
			    files = 0;
			    get_dir_size(temppath, &files, &bytes);
			    log.files = files;
			    log.bytes = bytes;

			    update_log(log);
		}
	}

	closedir(dirf);
	exit(0);
}


void
get_dir_size (char *dirname, int *files, unsigned long long *bytes)
{
	DIR *dirg;
	struct dirent *dp;
	struct stat st;
	char temppath[MAXPATHLEN];

	dirg = opendir (dirname);
	if (dirg) {
		while (1) {
			dp = readdir (dirg);
			if (!dp)
				break;
			if (!strcmp (dp->d_name, ".") || !strcmp (dp->d_name, "..") || !strcasecmp (dp->d_name, "file_id.diz") || !strcmp (dp->d_name, ".message"))
				continue;
			if (strlen (dp->d_name) > 3)
				if (!strcasecmp(dp->d_name + strlen (dp->d_name) - 4,".nfo"))
					continue;

			snprintf (temppath, MAXPATHLEN, "%s/%s", dirname, dp->d_name);
			stat (temppath, &st);
			if (S_ISREG (st.st_mode)) {
				*(files) += 1;
				*(bytes) += (unsigned long long)st.st_size;
			} else if (S_ISDIR (st.st_mode)) {
				get_dir_size (temppath, files, bytes);
			}
		}
		closedir(dirg);
	  }
}


void
usage (void)
{
	printf ("glftpd DIRLOG update utility v3.1\n\n");
        printf ("glupdate [-r /pathto/glftpd.conf] [-i ...] [-h] <full directory path>\n Options:\n"
               " -r /path/file : alternative path to config file\n"
               " -i ...        : unlimited number of dirs to skip\n"
               "                 \".\", \"..\" and \"groups\" are skipped by default\n"
               " -h            : this screen\n");
	exit (0);
}


void
update_log (struct dirlog log)
{
	struct dirlog newlog;
	char work_buf[MAXPATHLEN+1];
	char actname[MAXPATHLEN+1];
	FILE *file;
	int x, y;

	if (strrchr (log.dirname, '/') == NULL)
		strncpy (actname, log.dirname, sizeof (actname));
	else
		strncpy (actname, strrchr (log.dirname, '/') + sizeof (char), sizeof (actname));

	snprintf (work_buf, MAXPATHLEN, "%s/%s/logs/dirlog", rootpath, datapath);
	file = fopen (work_buf, "r+b");
	if (file == NULL) {
		printf ("Cannot open %s\n", work_buf);
		exit (1);
	}

	if (strcmp (rootpath, "/") != 0) {
		memset(work_buf, '\0', sizeof(work_buf));
		y = 0;
		for (x = 0; x < (int)strlen (log.dirname); x++) {
			if (rootpath[x] == log.dirname[x])
				continue;
			work_buf[y++] = log.dirname[x];
		}
		work_buf[y] = '\0';
		strncpy (log.dirname, work_buf, sizeof (log.dirname));
	}
	printf ("Updating: %s\n", actname);

	for (;;) {
		fread (&newlog, sizeof (struct dirlog), 1, file);

		if (feof (file)) {
			fwrite (&log, sizeof (struct dirlog), 1, file);
			fclose (file);
			return;
		}

		if (strcmp(newlog.dirname, log.dirname) == 0) {
			fseek (file, -(sizeof (struct dirlog)), SEEK_CUR);
			fwrite (&log, sizeof (struct dirlog), 1, file);
			fclose (file);
			return;
		    }
	  }
}


/* load_sysconfig
   Loads data from system configuration file.
*/
void
load_sysconfig (char *config_file)
{
	FILE *configfile;
	char lvalue[64];
	char rvalue[MAXPATHLEN];
	int x, y;
	char work_buff[MAXPATHLEN];

	strncpy (work_buff, config_file, MAXPATHLEN);

	if ((configfile = fopen (work_buff, "r")) == NULL) {
		fprintf(stderr, "Bad or missing config file (%s), using defaults\n", config_file);
		strcpy(datapath, "ftp-data");
		return;
	}

	for (;;) {
		if (fgets(work_buff, sizeof(work_buff), configfile) == NULL) {
			fclose(configfile);
			return;
		}

		/* Clip out comments */
		for (x = 0; x < (int)strlen (work_buff); x++)
			if (work_buff[x] == '#')
				work_buff[x] = '\0';

		/* Trim */
		trim(work_buff);

		/* Clear out old values */
		memset (lvalue, '\0', sizeof (lvalue));
		memset (rvalue, '\0', sizeof (rvalue));

		/* Parse lvalue */
		y = 0;
		for (x = 0; x < (int)strlen (work_buff) && work_buff[x] != ' '; x++)
			if (isprint (work_buff[x]))
				lvalue[y++] = work_buff[x];

		/* Parse rvalue */
		y = 0;
		x++;
		for (; x < (int)strlen (work_buff); x++)
			if (isprint (work_buff[x]))
				rvalue[y++] = work_buff[x];

		if (strcasecmp (lvalue, "datapath") == 0)
			strncpy (datapath, rvalue, sizeof (datapath));
		if (strcasecmp (lvalue, "rootpath") == 0)
			strncpy (rootpath, rvalue, sizeof (rootpath));
	}
	return;
}


/* trim
   Trims a string. ("    TEST    BBOY    " becomes "TEST BBOY")
*/
char *
trim (char *str)
{
	char *ibuf;
	char *obuf;

	if (str)
	  {
		  for (ibuf = obuf = str; *ibuf;)
		    {
			    while (*ibuf && (isspace (*ibuf)))
				    ibuf++;
			    if (*ibuf && (obuf != str))
				    *(obuf++) = ' ';
			    while (*ibuf && (!isspace (*ibuf)))
				    *(obuf++) = *(ibuf++);
		    }
		  *obuf = '\0';
	  }
	return (str);
}


/* add_ignore
   Add an entry to the linked list of dirs to ignore
*/
void
add_ignore (char *string)
{
	struct ignore *ignore = START;
	struct ignore *new;

	while (ignore != NULL && ignore->next != NULL)
		ignore = ignore->next;

	new = malloc(sizeof(struct ignore));
	if (new == NULL) {
		fprintf(stderr, "unable to allocate %d bytes of memmory in add_ignore()\n", (int)sizeof(struct ignore));
		exit(1);
	}

	new->next = NULL;
	new->string = string;

	if (START == NULL) {
		START = new;
	} else {
		ignore->next = new;
	}

	return;
}
