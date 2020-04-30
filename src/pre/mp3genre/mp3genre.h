/*
 * slv 20170414 - headers for mp3genre.c
 *
 * from pzs-ng zipscript/src/multimedia.c
 * http://www.pzs-ng.com
 *
*/
#ifndef _mp3_genre_h
#define _mp3_genre_h

#include <stdio.h>

extern char *genre_s[];
extern unsigned char genre_count;
char *get_mp3_genre(const char* filename);

#endif
