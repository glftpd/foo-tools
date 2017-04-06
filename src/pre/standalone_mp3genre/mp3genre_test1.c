/*
    slv 02082012 - mp3 genre output added to PRE (instead of as a module) 
                   reads last 128 bytes of mp3 and extracts genre tag
                   thanks to movEAX_444 (devshed.com) for his code example
*/

#include <stdio.h>

/*
typedef struct _id3tag
{
	unsigned char genre;
} id3tag;

int main(int argc, char **argv)
{
	id3tag ID3;
	if(!ReadID3("test.mp3",&ID3))
		printf("error!\n");
	printf("Genre %d\n",ID3.genre);
	return 0;
}
*/
//int ReadID3(const char* Filename, id3tag *ID3Tag)

int main()
{
	const char* filename="test.mp3";
	FILE *fp;
//	unsigned char genre;
	char fbuf[128];
	if (!(fp=fopen(filename,"rb"))) {
		printf("DEBUG: doesnt exist %s\n",filename);
		return 1;
	}
	if (fseek(fp,-128,SEEK_END)) {
		printf("DEBUG: >128 %s\n",filename);
		return 1;
	} else {

/*
                fread(fbuf,1,3,fp); fbuf[3] = '\0';
                printf("DEBUG: fbuf 0 1 2 3 4 %c %c %c %c %c\n", fbuf[0], fbuf[1], fbuf[2], fbuf[3], fbuf[4]);
		char genre[0];
		char title[255];
		char artist[255];
		char album[255];
		char year[255];
		char comment[255];
		char track[255];
		if (!strcmp((const char *)"TAG",(const char *)fbuf)) {
			printf("DEBUG: TAG match\n");
			fseek(fp, -125, SEEK_END);
	              fread(title,1,30,fp); title[30] = '\0';
	              fread(artist,1,30,fp); artist[30] = '\0';
	              fread(album,1,30,fp); album[30] = '\0';
	              fread(year,1,4,fp); year[4] = '\0';
	              fread(comment,1,30,fp); comment[30] = '\0';
	              if(comment[28] == '\0') {
	                 track[0] = comment[29];
	              }
			fread(genre,1,1,fp);
		}
                printf("DEBUG: Title %s\n",title);
                printf("DEBUG: Artist %s\n",artist);
                printf("DEBUG: Album %s\n",album);
                printf("DEBUG: Year %s\n",year);
                printf("DEBUG: Comment %s\n",comment);
                printf("DEBUG: Track %s\n",track);
                printf("DEBUG: Genre %s\n",genre);
*/
		unsigned char genre;

		fread(fbuf,sizeof(char),sizeof(fbuf),fp);
		if(!(fbuf[0]=='T' && fbuf[1] == 'A' && fbuf[2] == 'G')) {
			printf("DEBUG: no tag %s\n",filename);
			return 0;
		}
	        printf("DEBUG: %c %c %c\n",fbuf[0], fbuf[1], fbuf[2]);
		if(fbuf[127] > 0 && fbuf[127] < 256) {
			genre = fbuf[127];
			printf("Genre %d\n",genre);
		}
		else {
			genre = 255;
			printf("DEBUG: Genre above 255 %d\n",genre);
		}
		return 1;
	}
}

