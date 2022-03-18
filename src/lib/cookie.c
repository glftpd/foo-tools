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
 * $Id: cookie.c,v 1.2 2003/01/22 14:31:29 sorend Exp $
 */
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fnmatch.h>
#include "cookie.h"

void c_free(void **v)
{
	if (*v)
	{
		free(*v);
		*v = NULL;
	}
}

int cv_create(cookie_value_s **cv)
{
	*cv = (cookie_value_s*) malloc(sizeof(cookie_value_s));
	if (!*cv)
		return 0;

	(*cv)->cv_val = NULL;
	(*cv)->cv_next = NULL;

	return 1;
}

int c_create(cookie_s **c)
{
	*c = (cookie_s*) malloc(sizeof(cookie_s));
	if (!*c)
		return 0;

	(*c)->c_tok = NULL;
	(*c)->c_val = NULL;
	(*c)->c_next = NULL;

	return 1;
}

void cv_destroy(cookie_value_s **cv)
{
	cookie_value_s *cr = *cv,*t;

	while (cr)
	{
		t = cr->cv_next;
		c_free((void*)&cr->cv_val);
		c_free((void*)&cr);
		cr = t;
	}

	*cv = NULL;
}

void c_destroy(cookie_s **c)
{
	cookie_s		*cr,*t;

	cr = *c;
	while (cr)
	{
		t = cr->c_next;
		c_free((void*)&cr->c_tok);
		cv_destroy(&cr->c_val);
		c_free((void*)&cr);
		cr = t;
	}

	*c = NULL;
}

cookie_s *c_gettoken(cookie_s *c,char *tok)
{
	cookie_s		*cr = c;

	while (c)
	{
		if (!strcmp(c->c_tok,tok))
			return c;
		c = c->c_next;
	}

	return NULL;
}

int c_addvalue(cookie_value_s **cv,void *val,int type)
{
	cookie_value_s		*n,*cr;

	if (!cv_create(&n))
		return 0;

	switch (type)
	{
		case C_STRING	:
			n->cv_val = malloc(strlen(val));
			if (!n->cv_val)
			{
				cv_destroy(&n);
				return 0;
			}
			strncpy(n->cv_val,val,strlen(val));
			break;
		case C_INTEGER	:
			n->cv_val = malloc(sizeof(int));
			if (!n->cv_val)
			{
				cv_destroy(&n);
				return 0;
			}
			*((int*)n->cv_val) = *((int*)val);
			break;
		case C_FLOAT	:
			n->cv_val = malloc(sizeof(double));
			if (!n->cv_val)
			{
				cv_destroy(&n);
				return 0;
			}
			*((double*)n->cv_val) = *((double*)val);
			break;

		default		:
			cv_destroy(&n);
			return 0;
	}

	if (!*cv)
		*cv = n;
	else
	{
		cr = *cv;
		while (cr->cv_next)
			cr = cr->cv_next;

		cr->cv_next = n;
	}

	return 1;
}

int c_addtoken(cookie_s **c,char *tok,void *val,int type)
{
	cookie_s		*n,*cr;

	cr = c_gettoken(*c,tok);
	if (cr)
		return c_addvalue(&cr->c_val,val,type);

	if (!c_create(&n))
		return 0;

	n->c_tok = strdup(tok);
	if (!n->c_tok)
	{
		c_destroy(&n);
		return 0;
	}

	if (!c_addvalue(&n->c_val,val,type))
	{
		c_destroy(&n);
		return 0;
	}

	n->c_type = type;

	if (!*c)
		*c = n;
	else
	{
		cr = *c;
		while (cr->c_next)
			cr = cr->c_next;

		cr->c_next = n;
	}

	return 1;
}

cookie_value_s *c_getvalue(cookie_s *c,int i)
{
	cookie_value_s	*cv;
	int		x;
	
	for (cv = c->c_val,x = 1; cv && x < i; x++)
		cv = cv->cv_next;

	return x == i ? cv : NULL;
}

char *c_strndup(char *buf,size_t size)
{
	char *s = malloc(size+1);
	if (!s) return NULL;
	strncpy(s,buf,size);
	s[size] = '\0';
	return s;
}

char *c_readto(char **i,char c)
{
	char *s,*p,*ip = *i;

	for (p = ip; *p && *p != c; p++);
	if (*p != c) return NULL;
	if (!(s = c_strndup(ip,p - ip)))
		return NULL;
	if (!(ip += strlen(s) + 1))
	{
		c_free((void*)&s);
		return NULL;
	}
	*i = ip;
	return s;
}

char *c_insert(cookie_s *c,char *ibuf,char *obuf,size_t size)
{
	cookie_s		*cr;
	cookie_value_s		*cv;
	char			*o,*i,*p,*fmt,*idx,*tok;
	int			len;

	bzero(obuf,size);
	fmt = idx = tok = NULL;

	for (i = ibuf,o = obuf; *i && (len = size - strlen(obuf));)
	{
		if (*i == '%')
		{
			if (!*++i)
				continue;

			if (*i == '%')
			{
				*o++ = *i++;
				continue;
			}

			c_free((void*)&fmt);
			c_free((void*)&idx);
			c_free((void*)&tok);

			while (1)
			{
				if (*i == '[' && !fmt)
				{
					if (!*i++) continue;
					fmt = c_readto(&i,']');
				}
				else
				if (*i == '(' && !idx)
				{
					if (!*i++) continue;
					idx = c_readto(&i,')');
				} 
				else
					break;
			}

			for (p = i; *p && !isspace(*p) && *p != '%'; p++);
			if (!isspace(*p) && *p != '%') continue;
			if (!(tok = c_strndup(i,p - i)))
				continue;
			i = p;
			if (*i == '%') *i++;

			if (!(cr = c_gettoken(c,tok)))
				continue;

			if (cv = c_getvalue(cr,idx ? atoi(idx) > 0 ? atoi(idx) : 1 : 1))
			{
				switch (cr->c_type)
				{
				case C_STRING	:
					snprintf(o,len,fmt ? fmt : "%s",(char*)cv->cv_val);
					break;
				case C_INTEGER	:
					snprintf(o,len,fmt ? fmt : "%li",*((long*)cv->cv_val));
					break;
				case C_FLOAT	:
					snprintf(o,len,fmt ? fmt : "%f",*((float*)cv->cv_val));
					break;
				}
			}
			else
			{
				char	*temp = malloc(len);
				int 	x;

				if (!temp) continue;
				switch (cr->c_type)
				{
				case C_STRING	:
					snprintf(temp,len,fmt ? fmt : "%s","");
					break;
				case C_INTEGER	:
					snprintf(temp,len,fmt ? fmt : "%li",0);
					break;
				case C_FLOAT	:
					snprintf(temp,len,fmt ? fmt : "%f",0);
					break;
				}
				for (x = 0; x < strlen(temp); x++)
					*o++ = ' ';
				c_free((void*)&temp);
			}
		}
		else
			*o = *i++;
		while (*o)
			o++;
	}

	obuf[strlen(obuf)] = '\0';

	c_free((void*)&fmt);
	c_free((void*)&idx);
	c_free((void*)&tok);

	return o;
}

int c_addint(cookie_s **c,char *tok,long i)
{
	return c_addtoken(c,tok,&i,C_INTEGER);
}

int c_addfloat(cookie_s **c,char *tok,double d)
{
	return c_addtoken(c,tok,&d,C_FLOAT);
}

int c_addstring(cookie_s **c,char *tok,char *s)
{
	return c_addtoken(c,tok,s,C_STRING);
}
