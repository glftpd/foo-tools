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
/**
 * Library to replace items in text files. Note that this one should be replaced
 * by bio's replacer lib, which is fancier.
 *
 **
 * $Id: macro.h,v 1.2 2003/01/22 14:31:29 sorend Exp $
 * $Source: /home/cvs/footools/footools/src/lib/macro.h,v $
 * Author: Soren
 */

#include <stdlib.h>

struct macro_list {
	char *mac_rep;
	char *mac_key;
	int mac_type;

	struct macro_list *next;
};

struct macro_donelist {
	char *str;

	struct macro_donelist *next;
};

#define ML_INT 0
#define ML_FLOAT 1
#define ML_STRING 2
#define ML_CHAR 3

/*
 * Add a float for replacing in macrolist.
 */
struct macro_list *ml_addfloat(struct macro_list *l, char *r, double f);

/*
 * Add a string for replacing in macrolist.
 */
struct macro_list *ml_addstring(struct macro_list *l, char *r, char *s);

/*
 * add an int for replacing in macrolist.
 */
struct macro_list *ml_addint(struct macro_list *l, char *r, int i);

/*
 * add a char for replacing in macrolist.
 */
struct macro_list *ml_addchar(struct macro_list *l, char *r, char c);

/*
 * replace in buf using the macrolist. returns new char buffer which must be
 * properly cleaned up by user.
 */
char *ml_replacebuf(struct macro_list *l, char *buf);

/*
 * destructor for macrolist structure.
 */
void ml_free(struct macro_list *l);

/*

Example usage:

struct macro_list *ml = NULL;

ml = ml_addstring(ml, "USER", "flowje");
ml = ml_addfloat(ml, "BYTES", 10.2);

printf(ml_replacebuf(ml, "| %[%-10.10s]USER% | %[%10.1f]BYTES% |\n"));

ml_free(ml);

 */
