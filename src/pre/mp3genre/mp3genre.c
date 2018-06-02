/*
 * slv 20180502 - mp3genre.c:
 *
 *	reads last 128 bytes of mp3 and extracts genre tag
 *	ph33r my ugly "code" ;o
 *
 *      to compile as standalone binary:
 *        gcc -DSTANDALONE -o mp3genre mp3genre.c
 *        use -DDEBUG to enable debug logging to
 *        foo-pre log or mp3genre.log (standalone)
 *
 * this file is based in part on:
 *
 *	* MP3Info 0.5 by Ricardo Cerqueira <rmc@rccn.net>
 * 	* MP3Stat 0.9 by Ed Sweetman <safemode@voicenet.com> and
 *                       Johannes Overmann <overmann@iname.com>
 *
*/

#include <time.h>
#include "mp3genre.h"

#ifdef STANDALONE
int main(int argc, char *argv[]) {
	const char* filename=argv[1];
	char *get_mp3_genre(const char* filename);
	get_mp3_genre(filename);
}
#endif

char *get_mp3_genre(const char* filename) {

	unsigned char id3_genre_num[1];
        char *id3_genre = "Unknown";
	FILE *f, *mp3file;
	char mp3_fbuf[4];

	if (!(mp3file=fopen(filename,"rb"))) {
		return NULL;
	}
	if (fseek(mp3file,-128,SEEK_END)) {
		return NULL;
	} else {
		fread(mp3_fbuf,1,3,mp3file); mp3_fbuf[3] = '\0';
		id3_genre_num[0]=255;
		if (!strcmp((const char *)"TAG",(const char *)mp3_fbuf)) {
			fseek(mp3file, -1, SEEK_END);
			fread(id3_genre_num,1,1,mp3file);
			if (id3_genre_num[0] != '\0' && id3_genre_num[0] > 0 && id3_genre_num[0] < genre_count) {
				id3_genre = genre_s[id3_genre_num[0]];
			}
		}
	}
	fclose(mp3file);
#ifdef STANDALONE
	printf("%s (%d)\n",id3_genre,id3_genre_num[0]);
#endif
#ifdef DEBUG
	f = fopen("/ftp-data/logs/foo-pre.log", "a");
	if (!f)
		f = fopen("mp3genre.log", "a");
	char fdate[12], ftime[10];
	time_t now;
	struct tm *tm_now;
	now = time(0);
	tm_now = localtime(&now);
	strftime(fdate, 1024, "%Y-%m-%d", tm_now);
	strftime(ftime, 1024, "%H:%M:%S", tm_now);
	fprintf(f, "%s %s MP3GENRE: \"%s\" \"%s\"\n", fdate, ftime, filename, id3_genre);
	fclose(f);
#endif
	return id3_genre;
}
/* vim: set noai tabstop=8 shiftwidth=8 softtabstop=8 noexpandtab: */
