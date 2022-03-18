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
 * Zip module for frame checker.
 *
 * Zip routine usage ideas gotten from  biohazard's ziplib.c
 */

#include <string.h>
#include <time.h>
#include <ctype.h>
#include <fnmatch.h>

#include <checker/frame.h>
#include "mod_zip.h"

#include <unzip/unzip.h>
#include <zip/zip.h>
#include <checker/util.h>
#include <lib/stringtokenizer.h>

#define ZIP_BUFLEN 65536
#define TEMPSUFFIX "-processing"

int zip_dupe(hashtable_t *ht, char *file, char *dir) {

	// not implemented, always returns success.
	return 1;
}

int zip_make_last_zf_vars(hashtable_t *c, unz_file_info *fi, char *fn, char *stat) {
	char key[MAX_FILELEN];
	char val[MAX_FILELEN];

	sprintf(key, "%s%c%s", ZIP_LAST_ZF_VARS_PROP, HT_DEF_DELIM, "file");
	ht_put(c, key, fn);
	
	sprintf(key, "%s%c%s", ZIP_LAST_ZF_VARS_PROP, HT_DEF_DELIM, "size");
	sprintf(val, "%.1f", (float)fi->uncompressed_size/1024);
	ht_put(c, key, val);

	sprintf(key, "%s%c%s", ZIP_LAST_ZF_VARS_PROP, HT_DEF_DELIM, "zsize");
	sprintf(val, "%.1f", (float)fi->compressed_size/1024);
	ht_put(c, key, val);

	sprintf(key, "%s%c%s", ZIP_LAST_ZF_VARS_PROP, HT_DEF_DELIM, "method");
	sprintf(val, "m%lu", fi->compression_method);
	ht_put(c, key, val);

	sprintf(key, "%s%c%s", ZIP_LAST_ZF_VARS_PROP, HT_DEF_DELIM, "status");
	ht_put(c, key, stat);
}

char * zip_must_extract(char *file, stringtokenizer *st) {
	char *extfile, *tmp;
	int i;

	if (!file || !st)
		return 0;

	tmp = strrchr(file, '/');
	if (tmp)
		extfile = strdup(tmp + 1);
	else
		extfile = strdup(file);

	for (i = 0; i < strlen(extfile); i++)
		extfile[i] = tolower(extfile[i]);

	st_reset(st);

	while (tmp = st_next(st)) {
		if (!fnmatch(tmp, extfile, 0))
			return extfile;
	}

	return 0;
}

int zip_convert_fi_to_zi(unz_file_info *fi, zip_fileinfo *zi) {
	zi->tmz_date.tm_sec = fi->tmu_date.tm_sec;
	zi->tmz_date.tm_min = fi->tmu_date.tm_min;
	zi->tmz_date.tm_hour = fi->tmu_date.tm_hour;
	zi->tmz_date.tm_mday = fi->tmu_date.tm_mday;
	zi->tmz_date.tm_mon = fi->tmu_date.tm_mon;
	zi->tmz_date.tm_year = fi->tmu_date.tm_year;
	zi->dosDate = 0;
	zi->internal_fa = 0;
	zi->external_fa = 0;

	return fi->compression_method;
}


int zip_is_propaganda(hashtable_t *ht, char *file) {
	int i, len;
	struct stat st;
	char *tmp, *buf, *lfile;

	tmp = ht_get(ht, PROPERTY_ZIP_UNWANTEDDIR);

	if (!tmp)
		return 0;

	lfile = strdup(file);

	// lowercase.
	len = strlen(lfile);
	for (i = 0; i < len; i++)
		lfile[i] = tolower(lfile[i]);

	buf = (char*)malloc(strlen(lfile) + strlen(tmp) + 2);

	sprintf(buf, "%s/%s", tmp, lfile);

	free(lfile);

	i = stat(buf, &st);

	free(buf);

	return (i != -1);
}

int zip_test_and_extract(hashtable_t *ht, char *file, char *dir, zipFile updzf) {
	unzFile zf;
	unz_global_info gi;
	unz_file_info fi;
	zip_fileinfo zi;
	int i, mustextract, nread, method, propaganda, rc;
	char *tmp, *extract;
	char efile[MAX_FILELEN], zstatus[MAX_FILELEN];
	hashtable_t *tmpenv;
	FILE *outf = 0;
	char zbuf[ZIP_BUFLEN];
	stringtokenizer extrfiles;
	struct stat st;

	extract = ht_get(ht, PROPERTY_ZIP_EXTRACT);
	if (extract)
		st_initialize(&extrfiles, extract, "|");
	else
		st_initialize(&extrfiles, 0, 0);

	if ((zf = unzOpen(file)) == 0) {
		error_printf(ht, ht_get(ht, PROPERTY_TMPL_ERRORMSG), "Opening zipfile");

		return 0;
	}

	if (unzGetGlobalInfo(zf, &gi) < 0) {
		error_printf(ht, ht_get(ht, PROPERTY_TMPL_ERRORMSG), "Reading zipfile header");

		unzClose(zf);

		return 0;
	}

	for (i = 0; i < gi.number_entry; i++) {
		strncpy(zstatus, ZIP_STATUS_TESTED, MAX_FILELEN);

		if (unzGetCurrentFileInfo(zf, &fi, efile, MAX_FILELEN, 0, 0, 0, 0) < 0) {
			error_printf(ht, ht_get(ht, PROPERTY_TMPL_ERRORMSG), "Reading zipfile entry-hdr");

			unzClose(zf);

			return 0;
		}

		propaganda = zip_is_propaganda(ht, efile);

		if (propaganda) {
			strcpy(zstatus, ZIP_STATUS_DELETED);

			goto ZIP_DO_NEXT_FILE;
		}


		if (unzOpenCurrentFile(zf) < 0) {
			error_printf(ht, ht_get(ht, PROPERTY_TMPL_ERRORMSG), "Reading zipfile entry-data");

			unzClose(zf);

			return 0;
		}

		method = zip_convert_fi_to_zi(&fi, &zi);

		if (zipOpenNewFileInZip(updzf, efile, &zi, 0, 0, 0, 0, 0, method, Z_DEFAULT_COMPRESSION) < 0) {
			error_printf(ht, ht_get(ht, PROPERTY_TMPL_ERRORMSG), "Opening entry in temp-zip");
			unzClose(zf);
			return 0;
		}

		extract = zip_must_extract(efile, &extrfiles);

		outf = 0;
		if (extract) {
			tmp = (char*)malloc(strlen(dir) + strlen(extract) + 2);
			sprintf(tmp, "%s/%s", dir, extract);

			rc = stat(tmp, &st);

			if (rc != -1) {
				strncpy(zstatus, ZIP_STATUS_EXISTS, MAX_FILELEN);
			} else {
				strncpy(zstatus, ZIP_STATUS_EXTRACTED, MAX_FILELEN);

				outf = fopen(tmp, "wb");

				free(tmp);

				if (!outf)
					error_printf(ht, ht_get(ht, PROPERTY_TMPL_ERRORMSG), "Opening extracted file for writing");
			}
		}

		do {
			if ((nread = unzReadCurrentFile(zf, &zbuf, ZIP_BUFLEN)) < 0) {
				error_printf(ht, ht_get(ht, PROPERTY_TMPL_ERRORMSG), "Reading zipfile entry-data");

				if (outf)
					fclose(outf);

				unzClose(zf);

				return 0;
			}

			if (nread && outf)
				fwrite(&zbuf, 1, nread, outf);

			if (zipWriteInFileInZip(updzf, &zbuf, nread) < 0) {
				error_printf(ht, ht_get(ht, PROPERTY_TMPL_ERRORMSG), "Writing in temp-zip");
				unzClose(zf);
				return 0;
			}

		} while (nread > 0);

		if (zipCloseFileInZip(updzf) < 0) {
			error_printf(ht, ht_get(ht, PROPERTY_TMPL_ERRORMSG), "Closing in temp-zip");
			unzClose(zf);
			return 0;
		}

		if (outf)
			fclose(outf);

	ZIP_DO_NEXT_FILE:

		tmp = ht_get(ht, PROPERTY_ZIP_SHOWFILE);

		// if we have this property, then display output for each zipfile.
		if (tmp) {
			zip_make_last_zf_vars(ht, &fi, efile, zstatus);

			tmpenv = ht_get_tree(ht, ZIP_LAST_ZF_VARS_PROP, HT_DEF_DELIM);

			tmp = util_replacer_env(tmpenv, tmp);
			ht_finalize(tmpenv);
			free(tmpenv);

			printf(tmp);

			free(tmp);
		}

		if (i + 1 < gi.number_entry)
			if (unzGoToNextFile(zf) < 0) {
				error_printf(ht, ht_get(ht, PROPERTY_TMPL_ERRORMSG), "Finding next file in zip");

				unzClose(zf);

				return 0;
			}
	}

	unzClose(zf);

	return 1;
}

void zip_fill_zi(zip_fileinfo *i) {
	time_t now;
	struct tm *ts;

	now = time(0);

	ts = localtime(&now);

	i->tmz_date.tm_sec = ts->tm_sec;
	i->tmz_date.tm_min = ts->tm_min;
	i->tmz_date.tm_hour = ts->tm_hour;
	i->tmz_date.tm_mday = ts->tm_mday;
	i->tmz_date.tm_mon = ts->tm_mon;
	i->tmz_date.tm_year = ts->tm_year + 1900;

	i->dosDate = 0;
	i->internal_fa = 0;
	i->external_fa = 0;
}


/*
 * ad file:
 *
 * # - random decimal number
 * @ - random character (a-z)
 * % - ramdom binary number
 */
int zip_put_ads(hashtable_t *ht, zipFile zf) {
	char *tmp, *ad, *adf;
	zip_fileinfo zi;

	adf = ht_get(ht, PROPERTY_ZIP_AD_FILE);
	ad = ht_get(ht, PROPERTY_ZIP_AD);

	if (!adf || !ad)
		return 1;

	tmp = adf;

	while (*tmp) {
		if (*tmp == '#')
			*tmp = '1';
		else if (*tmp == '@')
			*tmp = 'a';
		else if (*tmp == '%')
			*tmp = '0';

		tmp++;
	}

	tmp = util_replacer(ht, ad);

	zip_fill_zi(&zi);

	if (zipOpenNewFileInZip(zf, adf, &zi, 0, 0, 0, 0, 0, Z_DEFLATED, Z_DEFAULT_COMPRESSION) < 0) {
		error_printf(ht, ht_get(ht, PROPERTY_TMPL_ERRORMSG), "Adding new zip-entry");

		zipClose(zf, 0);

		return 0;
	}

	if (zipWriteInFileInZip(zf, tmp, strlen(tmp)) < 0) {
		error_printf(ht, ht_get(ht, PROPERTY_TMPL_ERRORMSG), "Writing new zip-entry");

		zipClose(zf, 0);

		return 0;
	}

	if (zipCloseFileInZip(zf) < 0) {
		error_printf(ht, ht_get(ht, PROPERTY_TMPL_ERRORMSG), "Closing new zip-entry");

		zipClose(zf, 0);

		return 0;
	}

	tmp = ht_get(ht, PROPERTY_ZIP_AD_COMMENT);

	tmp = util_replacer(ht, tmp);

	if (zipClose(zf, tmp) < 0) {
		return 0;
	}

	return 1;
}

int zip_check(hashtable_t *conf, char *file, char *dir, long crc) {
	char *path, *tmp;
	zipFile zf;
	int i = 0;

	path = ht_get(conf, PROPERTY_PATH);

	tmp = (char*)malloc(strlen(path) + strlen(TEMPSUFFIX) + 1);
	sprintf(tmp, "%s%s", path, TEMPSUFFIX);
	zf = zipOpen(tmp, 0);

	if (!zip_test_and_extract(conf, path, dir, zf)) {
		if (zf != 0) {
			zipClose(zf, 0);
			unlink(tmp);
		}
		free(tmp);
		return 0;
	}

	if (!zip_put_ads(conf, zf)) {
		unlink(tmp);
		free(tmp);
		return 0;
	}

	// rename the temp zipfile.
	rename(tmp, path);
	free(tmp);

	tmp = ht_get(conf, PROPERTY_ZIP_SUCCESS);

	if (tmp)
		msg_printf(conf, tmp, "Checking zip-file", "Done");

	return 1;
}






