/*
 * Cookie parsing lib by Biohazard
 * $Id: cookie.h,v 1.1.1.1 2001/04/30 10:49:37 sd Exp $
 */
struct _cookie_value_s
{
	void			*cv_val;
	struct _cookie_value_s	*cv_next;
} _cookie_value_s;

typedef struct _cookie_value_s cookie_value_s;

struct _cookie_s
{
	char			*c_tok;
	cookie_value_s		*c_val;
	int			c_type;
	struct _cookie_s	*c_next;
} _cookie_s;

typedef struct _cookie_s cookie_s;

#define C_STRING	0
#define C_INTEGER	1
#define C_FLOAT		2

int c_addtoken(cookie_s **c,char *tok,void *val,int type);
char *c_insert(cookie_s *c,char *ibuf,char *obuf,size_t size);
int c_addint(cookie_s **c,char *tok,long i);
int c_addfloat(cookie_s **c,char *tok,double d);
int c_addstring(cookie_s **c,char *tok,char *s);
