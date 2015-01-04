/*
 * $Id: linefilereader.c,v 1.1 2001/11/26 11:47:45 sd Exp $
 *
 * Author: flower
 */

#include "linefilereader.h"
#include <stdio.h>


int lfr_open(linefilereader_t *lfr, char *fn) {
	FILE *f;

	lfr->pos = lfr->len = -1;
	lfr->file_content = 0;

	f = fopen(fn, "r");
	if (!f)
		return -1;

	// get file len.
	fseek(f, 0, SEEK_END);
	lfr->len = ftell(f);
	fseek(f, 0, SEEK_SET);

	lfr->file_content = (char*)malloc(lfr->len);

	if (fread(lfr->file_content, 1, lfr->len, f) < lfr->len) {
		fclose(f);
		return -1;
	}

	fclose(f);

	lfr->pos = 0;

	return 0;
}

int lfr_getline(linefilereader_t *lfr, char *buf, int len) {
	char *tmp;
	long off, outlen;

	// ptr to offset for new line.
	tmp = lfr->file_content + lfr->pos;
	off = lfr->pos;

	if (lfr->pos >= lfr->len)
		return -1;

	// find next line.
	while (lfr->pos < lfr->len) {
		if ((lfr->file_content[lfr->pos] == '\r') || (lfr->file_content[lfr->pos] == '\n'))
			break;

		lfr->pos++;
	}

	outlen = (lfr->pos - off > len) ? len : lfr->pos - off;

	strncpy(buf, tmp, outlen);
	buf[outlen] = 0;

	off = lfr->pos - off;

	// advance.
	if ((lfr->file_content[lfr->pos] == '\r') && (lfr->file_content[lfr->pos + 1] == '\n'))
		lfr->pos+=2;
	else
		lfr->pos++;

	return outlen;
}

int lfr_close(linefilereader_t *lfr) {
	if (lfr->file_content)
		free(lfr->file_content);

	lfr->pos = -1;

	return 0;
}
