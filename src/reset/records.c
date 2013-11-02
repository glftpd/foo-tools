/*
 * foo-tools, a collection of utilities for glftpd users.
 * Copyright (C) 2003  Tanesha FTPD Project, www.tanesha.net
 *
 * This file is part of foo-tools.
 *
 * foo-tools is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * foo-tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with foo-tools; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 * $Id: records.c,v 1.2 2003/02/12 13:03:11 sorend Exp $
 */

/*
 * Do NOT use this library in applications which must run over longer time
 * yet. It contains memory leaks :-).
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include <collection/hashtable.h>

#include "records.h"

#define REC_DEL_CHAR '='

int rec_get_recfile(char *buf, char *ftpdatadir, int isgroup, char *period, int isupload) {

	sprintf(buf, "%s/%s/%s%s%s",
			ftpdatadir,
			REC_DIR,
			(isgroup ? REC_GROUP_PREFIX : REC_USER_PREFIX),
			period, (isupload ? REC_UPLOAD : REC_DOWNLOAD));

	return 1;
}



stat_record_t * rec_find_record(char *recfile) {

	hashtable_t ht;
	long files;
	double kbytes;
	char *tmp;
	stat_record_t *rec;

	// load file
	ht_init(&ht);
	ht_load_prop(&ht, recfile, REC_DEL_CHAR);

	rec = malloc(sizeof(stat_record_t));

	// validate/parse contents
	tmp = ht_get(&ht, REC_RECORDHOLDER);
	if (!tmp)
		return 0;

	strcpy(rec->recordholder, tmp);

	tmp = ht_get(&ht, REC_FILES);

	if (!tmp)
		return 0;

	rec->files = atol(tmp);

	tmp = ht_get(&ht, REC_SECONDS);

	if (!tmp)
		return 0;

	rec->seconds = atol(tmp);

	tmp = ht_get(&ht, REC_DATESET);

	if (!tmp)
		return 0;

	rec->dateset = date_parse_unix(tmp);

	if (rec->dateset)
		return 0;

	tmp = ht_get(&ht, REC_USERS);

	if (!tmp)
		return 0;

	rec->users = atoi(tmp);

	tmp = ht_get(&ht, REC_KBYTES);

	if (!tmp)
		return 0;

	sscanf(tmp, "%.0f", &rec->kbytes);

	ht_finalize(&ht);

	return rec;
}


int rec_set_record(char *recfile, stat_record_t *rec) {

	FILE *f;
	char *tmp;

	f = fopen(recfile, "w");

	if (!f)
		return 0;

	fprintf(f, "# file created by foo-reset, EDIT ON OWN RISK!\n");

	fprintf(f, "%s%c%s\n", REC_RECORDHOLDER, REC_DEL_CHAR, rec->recordholder);
	fprintf(f, "%s%c%.0f\n", REC_KBYTES, REC_DEL_CHAR, rec->kbytes);
	fprintf(f, "%s%c%d\n", REC_FILES, REC_DEL_CHAR, rec->files);
	fprintf(f, "%s%c%d\n", REC_SECONDS, REC_DEL_CHAR, rec->seconds);
	fprintf(f, "%s%c%d\n", REC_USERS, REC_DEL_CHAR, rec->users);

	// date
	tmp = date_tostring(rec->dateset, 0);
	fprintf(f, "%s%c%s\n", REC_DATESET, REC_DEL_CHAR, tmp);

	free(tmp);

	fclose(f);

	return 1;
}
