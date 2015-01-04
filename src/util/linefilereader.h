/*
 * Reads a file on line-oriented basis.
 */

struct linefilereader {
	char *file_content;
	long len, pos;
};

typedef struct linefilereader linefilereader_t;

/*
 * Initializes a line-filereader from the given filename (fn).
 *
 * returns 0 on success and -1 on error.
 */
int lfr_open(linefilereader_t *lfr, char *fn);

/*
 * Returns next line in buf upto 'len' of size.
 *
 * Returns length of next line, or 0 on eof/error.
 */
int lfr_getline(linefilereader_t *lfr, char *buf, int len);

/*
 * Closes a lfr context.
 *
 * Always returns 0 (success).
 */
int lfr_close(linefilereader_t *lfr);

