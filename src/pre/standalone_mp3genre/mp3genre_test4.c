#include <stdio.h>

typedef struct _id3tag
{
	unsigned char genre;
} id3tag;

int ReadID3(const char* Filename, id3tag *ID3Tag);//0==error, 1==sucess

int main(int argc, char **argv)
{
	id3tag ID3;
	if(!ReadID3("test.mp3",&ID3))
		printf("error!\n");
	printf("Genre %d\n",ID3.genre);
	return 0;
}


int ReadID3(const char* Filename, id3tag *ID3Tag)
{
	FILE *fp=fopen(Filename,"rb");
	char buffer[128];
	int x;
	fseek(fp,-128,SEEK_END);
	fread(buffer,sizeof(char),sizeof(buffer),fp);
	if(!(buffer[0]=='T' && buffer[1] == 'A' && buffer[2] == 'G'))
	{
		return 0;
	}
	if(buffer[127] > 0 && buffer[127] < 256)
                //if its between 1-255, put it
		ID3Tag->genre = buffer[127];
	else
		//255 means unused
		ID3Tag->genre = 255;
	return 1;
}
