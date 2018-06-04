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
 * Module to extract mp3info from a pred release
 * Author, Soren.
 * $Id: mod_idmp3.c,v 1.5 2003/06/23 14:32:18 sorend Exp $
 */

#include <string.h>

// project includes
#include "mod_idmp3.h"
#include "../foo-pre.h"

// include mp3info.h in special way to get globals registered.
#define __MAIN
#include "mp3info/mp3info.h"
#undef __MAIN

// footools includes
#include <collection/hashtable.h>
#include <lib/gllogs.h>

// in foo-pre
hashtable_t *_mod_idmp3_cfg = 0;

void set_config(hashtable_t *cfg) {
	_mod_idmp3_cfg = cfg;
}
hashtable_t *get_config() {
	return _mod_idmp3_cfg;
}

// prototype for file handling function.
int mod_idmp3_file_func(char *filepath, char *argv[]);

module_list_t mod_idmp3_info = {
	// module name
	"mp3 id extractor",

	// module dir func
	0,

	// module file func
	mod_idmp3_file_func,

	// module rel func
	0,

	// struct module_list entry
	0
};

// layer informatino
char *mod_idmp3_layer_text[] = {
	"I", "II", "III"
};


// module global, checks number of mp3s checked (only handle first).
int mod_idmp3_count = 0;

// function to return module info of this module.
module_list_t *module_loader() {
	return &mod_idmp3_info;
}

// replace function
int mod_idmp3_replace(char *b, char *n, char *r) {
	char *t, *save;
	int i=0;

	while (t=strstr(b, n)) {
		save=(char*)malloc(strlen(t)-strlen(n)+1);
		strcpy(save, t+strlen(n));
		*t=0;
		strcat(b, r);
		strcat(b, save);
		free(save);
		i++;
	}
}

// get genre as text from id
void mod_idmp3_text_genre(unsigned char *genre,char *buffer) {
   int genre_num = (int) genre[0];

   if(genre_num <= MAXGENRE) {
	sprintf(buffer,"%s",typegenre[genre_num]);
   } else if(genre_num < 255) {
	sprintf(buffer,"(UNKNOWN) [%d]",genre_num);
   } else {
	buffer[0]='\0';
   }
}



// format output using mp3info.
void mod_idmp3_format_output(char *format_string, mp3info *mp3, int vbr_report, char *relname) {

	char genre[40]="";
	char mod[1000],*percent,*pos,*code;
	char *format=format_string;
	int modlen;

	strcpy(mod, format_string);

	mod_idmp3_replace(mod, "%a", mp3->id3.artist);
	mod_idmp3_replace(mod, "%l", mp3->id3.album);
	mod_idmp3_replace(mod, "%y", mp3->id3.year);
	mod_idmp3_text_genre(mp3->id3.genre, genre);
	mod_idmp3_replace(mod, "%g", genre);
	sprintf(genre, "%d", mp3->id3.genre[0]);
	mod_idmp3_replace(mod, "%G", genre);

	if (mp3->vbr && (vbr_report == VBR_VARIABLE))
		sprintf(genre, "Variable");
	else if (vbr_report == VBR_AVERAGE)
		sprintf(genre, "%.1f", mp3->vbr_average);
	else
		sprintf(genre, "%d", header_bitrate(&mp3->header));
	mod_idmp3_replace(mod, "%r", genre);

	sprintf(genre, "%d", header_frequency(&mp3->header));
	mod_idmp3_replace(mod, "%Q", genre);

	mod_idmp3_replace(mod, "%L", mod_idmp3_layer_text[header_layer(&mp3->header)-1]);

	mod_idmp3_replace(mod, "%o", header_mode(&mp3->header));

	mod_idmp3_replace(mod, "%R", relname);

	mod_idmp3_replace(mod, "%S", "NYI");
	mod_idmp3_replace(mod, "%m", "NYI");
	mod_idmp3_replace(mod, "%s", "NYI");

	gl_gllog_add(mod);

	printf("  .. idmp3 module says: Logged information to glftpd.log\n   .. %s\n", mod);

	// printf("%s\n", mod);
}


// file func.
int mod_idmp3_file_func(char *filepath, char *argv[]) {

	mp3info mp3;
	char *tmp;
	FILE *fh;

	if (mod_idmp3_count > 0) {
		// printf("already got the mp3, break\n");
		return 0;
	}
	
	tmp = strrchr(filepath, '.');

	if (tmp)
		tmp++;
	else
		tmp = filepath;

	if (strcasecmp(tmp, "mp3")) {
		// printf(" .. %s -> not mp3, continue\n", tmp);
		return 1;
	}

	fh = fopen(filepath, "r");

	// could not open file
	if (!fh)
		return 1;

	bzero(&mp3, sizeof(mp3info));
	mp3.filename = filepath;
	mp3.file = fh;

	// call into mp3tech's get_mp3_info to get info
	get_mp3_info(&mp3, SCAN_QUICK, 0);

	fclose(fh);

	// if headers or id are not valid then continue
	if (!mp3.header_isvalid || !mp3.id3_isvalid)
		return 1;

	mod_idmp3_count++;

	tmp = ht_get(get_config(), PROPERTY_MOD_IDMP3_OUTPUT);

	if (!tmp)
		tmp = strdup(DEFAULT_OUTPUT);
	else
		tmp = strdup(tmp);

	mod_idmp3_format_output(tmp, &mp3, 0, argv[1]);

	return 1;
}


