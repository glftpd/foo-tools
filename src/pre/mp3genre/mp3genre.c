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

#include <string.h>

#include "mp3genre.h"

char *genre_s[] = {
        "Blues", "Classic Rock", "Country", "Dance",
        "Disco", "Funk", "Grunge", "Hip-Hop",
        "Jazz", "Metal", "New Age", "Oldies",
        "Other", "Pop", "R&B", "Rap",
        "Reggae", "Rock", "Techno", "Industrial",
        "Alternative", "Ska", "Death Metal", "Pranks",
        "Soundtrack", "Euro-Techno", "Ambient", "Trip-Hop",
        "Vocal", "Jazz+Funk", "Fusion", "Trance",
        "Classical", "Instrumental", "Acid", "House",
        "Game", "Sound Clip", "Gospel", "Noise",
        "AlternRock", "Bass", "Soul", "Punk",
        "Space", "Meditative", "Instrumental Pop", "Instrumental Rock",
        "Ethnic", "Gothic", "Darkwave", "Techno-Industrial",
        "Electronic", "Pop-Folk", "Eurodance", "Dream",
        "Southern Rock", "Comedy", "Cult", "Gangsta",
        "Top 40", "Christian Rap", "Pop_Funk", "Jungle",
        "Native American", "Cabaret", "New Wave", "Psychadelic",
        "Rave", "Showtunes", "Trailer", "Lo-Fi",
        "Tribal", "Acid Punk", "Acid Jazz", "Polka",
        "Retro", "Musical", "Rock & Roll", "Hard Rock",
        "Folk", "Folk-Rock", "National Folk", "Swing",
        "Fast Fusion", "Bebob", "Latin", "Revival",
        "Celtic", "Bluegrass", "Avantgarde", "Gothic Rock",
        "Progressive Rock", "Psychedelic Rock", "Symphonic Rock", "Slow Rock",
        "Big Band", "Chorus", "Easy Listening", "Acoustic",
        "Humour", "Speech", "Chanson", "Opera",
        "Chamber Music", "Sonata", "Symphony", "Booty Bass",
        "Primus", "Porn Groove", "Satire", "Slow Jam",
        "Club", "Tango", "Samba", "Folklore",
        "Ballad", "Power Ballad", "Rhythmic Soul", "Freestyle",
        "Duet", "Punk Rock", "Drum Solo", "A cappella",
        "Euro-House", "Dance Hall", "Goa", "Drum & Bass",
        "Club House", "Hardcore", "Terror", "Indie",
        "BritPop", "Negerpunk", "Polsk Punk", "Beat",
        "Christian Gangsta Rap", "Heavy Metal", "Black Metal", "Crossover",
        "Contemporary Christian", "Christian Rock", "Merengue", "Salsa",
        "Thrash Metal", "Anime", "JPop", "Synthpop",
        "Unknown"
};
unsigned char genre_count=149;

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
