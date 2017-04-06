#include <stdio.h>

#include "mp3tech.h"

int get_id3 (mp3info *mp3) {
   int retcode=0;
   char fbuf[4];
   if(mp3->datasize >= 128) {
        if(fseek(mp3->file, -128, SEEK_END )) {
           fprintf(stderr,"ERROR: Couldn't read last 128 bytes of %s!!\n",mp3->filename);
           retcode |= 4;
        } else {
           fread(fbuf,1,3,mp3->file); fbuf[3] = '\0';
           mp3->id3.genre[0]=255;
           if (!strcmp((const char *)"TAG",(const char *)fbuf)) {
              mp3->id3_isvalid=1;
              mp3->datasize -= 128;
              fseek(mp3->file, -125, SEEK_END);
              fread(mp3->id3.genre,1,1,mp3->file);
           }
        }
   }
   return retcode;
}
