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

#include "linereaderbuffer.h"

#include <stdlib.h>

int lrb_initialize(linereaderbuffer_t *lrb) {
	lrb->data = 0;
	lrb->offset = lrb->len = 0;
}

int lrb_add_data(linereaderbuffer_t *lrb, char *data, int len) {
	char *tmp;

	if (lrb->data == 0) {
		lrb->len = len;
		lrb->data = malloc(len);
		memcpy(lrb->data, data, len);

		return 0;
	}

	tmp = malloc(lrb->len + len);
	memcpy(tmp, lrb->data, lrb->len);
	memcpy(tmp + lrb->len, data, len);
	free(lrb->data);
	lrb->data = tmp;
	lrb->len += len;

	return 0;
}

int lrb_add_eof(linereaderbuffer_t *lrb) {
	lrb_add_data(lrb, "", 1);

	return 0;
}

int lrb_getline(linereaderbuffer_t *lrb, char *buf, int len) {
	char *tmp, *nbuf;
	int outlen;

	tmp = lrb->data;

	if (!tmp)
		return LRB_NO_DATA;

	if (*tmp == 0)
		return LRB_EOF;

	while (tmp < (lrb->data + lrb->len)) {
		if (*tmp == '\n' || *tmp == '\r' || *tmp == 0)
			break;

		tmp++;
	}

	if ((*tmp != '\n') && (*tmp != '\r') && (*tmp != 0))
		return LRB_NO_DATA;

	outlen = (tmp - lrb->data > len) ? len : tmp - lrb->data;
	strncpy(buf, lrb->data, outlen);
	buf[outlen] = 0;

	// move past the crlf.
	if (*tmp == '\r' && *(tmp + 1) == '\n')
		tmp += 2;
	else if (*tmp != 0)
		tmp++;

	outlen = lrb->len - (tmp - lrb->data);

	if (outlen == 0) {
		free(lrb->data);
		lrb->data = 0;
		lrb->len = 0;
	} else {
		nbuf = malloc(outlen);
		memcpy(nbuf, tmp, outlen);
		free(lrb->data);
		lrb->data = nbuf;
		lrb->len = outlen;
	}

	return strlen(buf);
}

int lrb_finalize(linereaderbuffer_t *lrb) {
	if (lrb->data)
		free(lrb->data);
	lrb->len = 0;
	lrb->data = 0;
}


/**
 * Test of the main functionality.

int main(int argc, char *argv[]) {

	linereaderbuffer_t lrb;
	char buf[300];
	int rc;

	lrb_initialize(&lrb);
	lrb_add_data(&lrb, "hello\n", strlen("hello\n"));

	rc = lrb_getline(&lrb, buf, 300);
	printf("rc = %d, buf = %s\n", rc, buf);

	rc = lrb_getline(&lrb, buf, 300);
	printf("rc = %d, buf = %s\n", rc, buf);

	lrb_add_data(&lrb, "sup today?\n maye nothing?\n", strlen("sup today?\n maye nothing?\n"));

	rc = lrb_getline(&lrb, buf, 300);
	printf("rc = %d, buf = %s\n", rc, buf);

	rc = lrb_getline(&lrb, buf, 300);
	printf("rc = %d, buf = %s\n", rc, buf);

	rc = lrb_getline(&lrb, buf, 300);
	printf("rc = %d, buf = %s\n", rc, buf);

	rc = lrb_getline(&lrb, buf, 300);
	printf("rc = %d, buf = %s\n", rc, buf);

	lrb_add_data(&lrb, "sup today?\n maye nothing?\n", strlen("sup today?\n maye nothing?\n"));
	lrb_add_data(&lrb, "sup today?\n maye nothing?\n", strlen("sup today?\n maye nothing?\n"));
	lrb_add_data(&lrb, "sup today?\n maye nothing?\r\n", strlen("sup today?\n maye nothing?\r\n"));

	lrb_add_eof(&lrb);

	while (1) {
		rc = lrb_getline(&lrb, buf, 300);

		printf("rc = %d\n", rc);

		if (rc == LRB_EOF)
			break;

		printf("Line: %s\n", buf);
	}

}

*/
