

#include "strmatch.h"

int _strmatch_patmat(const char *pat, const char *str) {
      switch (*pat) {
      case 0:
            return !*str;

      case '*' :
            return _strmatch_patmat(pat+1, str) || *str && _strmatch_patmat(pat, str+1);

      case '?' :
            return *str && _strmatch_patmat(pat+1, str+1);

      default  :
            return (*pat == *str) && _strmatch_patmat(pat+1, str+1);
      }
}


int strmatch_filename(char *pattern, char *string) {
	if (string == 0 || pattern == 0)
		return 0;

	return _strmatch_patmat(pattern, string);
}

/*
int main(int argc, char *argv[]) {

	printf("1: %d = 1\n", strmatch_filename("hello???mew", "hello123mew"));
	printf("2: %d = 0\n", strmatch_filename("hello??mew", "hello123mew"));
	printf("3: %d = 1\n", strmatch_filename("hello*mew", "hello123mew"));
	printf("4: %d = 1\n", strmatch_filename("hello*mew", "hellomew"));
	printf("5: %d = 0\n", strmatch_filename("hello*mew", "hellmew"));
	printf("6: %d = 1\n", strmatch_filename("hello123mew", "hello123mew"));
	printf("7: %d = 0\n", strmatch_filename("hello12mew", "hello123mew"));

}
*/
