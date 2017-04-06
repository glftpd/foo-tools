/*
    slv 02082012 - mp3 genre output added to PRE (instead of as a module) 
                   reads last 128 bytes of mp3 and extracts genre tag
                   thanks to movEAX_444 (devshed.com) for his code example
*/

#include <stdio.h>

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
char *id3_genre;
int id3_genre_num;

int main(int argc, char *argv[]) {
	const char* filename=argv[1];
	get_mp3_genre(filename);
	//printf("%s (%d)\n",id3_genre,id3_genre_num);
	printf("(%d)\n",id3_genre_num);
}

int get_mp3_genre(const char* filename) {
	FILE *fp;
	char mp3_fbuf[128];
	if (!(fp=fopen(filename,"rb"))) {
		return 1;
	}
	if (fseek(fp,-128,SEEK_END)) {
		return 1;
	}
	fread(mp3_fbuf,sizeof(char),sizeof(mp3_fbuf),fp); mp3_fbuf[3] = '\0';
	if (strcmp((const char *)"TAG" , (const char *) mp3_fbuf)) {
		return 1;
	}
	id3_genre_num = mp3_fbuf[127]; //if(!(id3_genre_num > 0 && id3_genre_num < genre_count)) { id3_genre_num = 148; }
	//id3_genre = genre_s[id3_genre_num];
}
