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
 * Cookie parsing lib by Biohazard
 * $Id: cookie.h,v 1.2 2003/01/22 14:31:29 sorend Exp $
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
