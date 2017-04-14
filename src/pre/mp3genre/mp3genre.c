/*
 * slv 20170414 - mp3genre.c:
 *
 *	reads last 128 bytes of mp3 and extracts genre tag
 *	ph33r my ugly "code" ;o
 *
 * this file is based in part on:
 *
 *	* MP3Info 0.5 by Ricardo Cerqueira <rmc@rccn.net>
 * 	* MP3Stat 0.9 by Ed Sweetman <safemode@voicenet.com> and
 *                       Johannes Overmann <overmann@iname.com>
 *
*/

#include "mp3genre.h"

char *get_mp3_genre(const char* filename) {
	FILE *fp;
	unsigned char id3_genre_num[1];
	char *id3_genre = "Unknown";
	char mp3_fbuf[2];
	if (!(fp=fopen(filename,"rb"))) {
		return NULL;
	}
	if (fseek(fp,-128,SEEK_END)) {
		return NULL;
	} else {
	        fread(mp3_fbuf,1,3,fp); mp3_fbuf[3] = '\0';
	        id3_genre_num[0]=255;
		if (!strcmp((const char *)"TAG",(const char *)mp3_fbuf)) {
		        fseek(fp, -1, SEEK_END);
			fread(id3_genre_num,1,1,fp);
			if(id3_genre_num[0] != '\0' && id3_genre_num[0] > 0 && id3_genre_num[0] < genre_count) {
				id3_genre = genre_s[id3_genre_num[0]];
			}
		}
	}
	return id3_genre;
}
