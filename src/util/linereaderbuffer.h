
struct linereaderbuffer {
	char *data;
	int offset;
	int len;
};

typedef struct linereaderbuffer linereaderbuffer_t;

#define LRB_NO_DATA -1
#define LRB_EOF -2

int lrb_initialize(linereaderbuffer_t *lrb);

/*
 * Return 0 on success.
 */
int lrb_add_data(linereaderbuffer_t *lrb, char *data, int len);

int lrb_add_eof(linereaderbuffer_t *lrb);

/*
 * Return -1 on error, otherwise length of new line.
 */
int lrb_getline(linereaderbuffer_t *lrb, char *buf, int len);

int lrb_finalize(linereaderbuffer_t *lrb);

