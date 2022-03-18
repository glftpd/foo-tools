/*
 * slv 20170414 - headers for mp3genre.c
 *
 * From pzs-ng zipscript/src/multimedia.c
 * Copyright (c) 2007, project-zs-ng team
 * All rights reserved.
 * http://www.pzs-ng.com
 *
*/
#include <string.h>
#include <stdio.h>

#ifndef _mp3genre_h_
#define _mp3genre_h_

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
	"Native American", "Cabaret", "New Wave", "Psychedelic",
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
	"Abstract", "Art Rock", "Baroque", "Bhangra",
	"Big Beat", "Breakbeat", "Chillout", "Downtempo",
	"Dub", "EBM", "Eclectic", "Electro", "Electroclash",
	"Emo", "Experimental", "Garage", "Global",
	"IDM", "Illbient", "Industro-Goth", "Jam Band",
	"Krautrock", "Leftfield", "Lounge", "Math Rock",
	"New Romantic", "Nu-Breakz", "Post-Punk", "Post-Rock",
	"Psytrance", "Shoegaze", "Space Rock", "Trop Rock",
	"World Music", "Neoclassical", "Audiobook", "Audio Theatre",
	"Neue Deutsche Welle", "Podcast", "Indie Rock", "G-Funk",
	"Dubstep", "Garage Rock", "Psybient", "Unknown"
};
unsigned char genre_count=193;
#endif
/* vim: set noai tabstop=8 shiftwidth=8 softtabstop=8 noexpandtab: */
