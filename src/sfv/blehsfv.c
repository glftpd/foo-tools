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
#include <strings.h>
#include <lib/sfv.h>

/*
 * example of how to use the sfv lib  /sorend.
 */
int main(int argc, char *argv[]) {
	sfv_list_t *tmp, *l = NULL;
	int rc, checked = 0, good = 0, bad = 0;
	long crc;

	l = sfv_list_load_path(".");

	if (!l) {
		printf("## Cry, no .sfv found in current dir.\n");
		return 1;
	}

	printf("## Sfv loaded with %d files\n", sfv_list_count(l));

	for (tmp = l; tmp; tmp = tmp->next) {
		if (argc > 1) {
			if (strcasecmp(tmp->filename, argv[1]))
				continue;
		}
		printf("-- %-45.45s [%08lx]", tmp->filename, tmp->crc);

		rc = sfv_mmap_calc_crc32(tmp->filename, &crc);

		if (rc != 0) {
			printf(" -->> Error calcing sfv\n");
			continue;
		}

		checked++;
		printf(" [%08lx]", crc);

		if (crc == tmp->crc)
			printf(" -->> Ok\n");
		else
			printf(" -->> FAIL\n");

		crc == tmp->crc ? good++ : bad++;
	}

	printf("## Total %d of %d files checked, %d good, %d bad, %d errors.\n",
		checked, sfv_list_count(l), good, bad, sfv_list_count(l) - checked);

	return 0;
}
