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


#include <ctype.h>
#include "strmatch.h"

int _strmatch_patmat(const char *pat, const char *str, int options) {
	int nocase = 0;

	if ((options & STRMATCH_IGNORECASE) == STRMATCH_IGNORECASE)
		nocase = 1;

	switch (*pat) {
	case 0:
		return !*str;
		
	case '*' :
		return _strmatch_patmat(pat+1, str, options) ||
			*str && _strmatch_patmat(pat, str+1, options);
		
	case '?' :
		return *str && _strmatch_patmat(pat+1, str+1, options);
		
	default  :
		return ((nocase?tolower(*pat):*pat) == (nocase?tolower(*str):*str)) &&
			_strmatch_patmat(pat+1, str+1, options);
	}
}


int strmatch_filename(char *pattern, char *string, int options) {
	if (string == 0 || pattern == 0)
		return 0;
	
	return _strmatch_patmat(pattern, string, options);
}

/*
int main(int argc, char *argv[]) {

	printf("1: %d = 1\n", strmatch_filename("hello???mew", "hello123mew", 0));
	printf("2: %d = 0\n", strmatch_filename("hello??mew", "hello123mew", 0));
	printf("3: %d = 1\n", strmatch_filename("hello*mew", "hello123mew", 0));
	printf("4: %d = 1\n", strmatch_filename("hello*mew", "hellomew", 0));
	printf("5: %d = 0\n", strmatch_filename("hello*mew", "hellmew", 0));
	printf("6: %d = 1\n", strmatch_filename("hello123mew", "hello123mew", 0));
	printf("7: %d = 0\n", strmatch_filename("hello12mew", "hello123mew", 0));

	printf("8: %d = 0\n", strmatch_filename("Hello123mew", "hello123mew", 0));
	printf("9: %d = 1\n", strmatch_filename("Hello123mew", "hello123mew", STRMATCH_IGNORECASE));

}
*/
