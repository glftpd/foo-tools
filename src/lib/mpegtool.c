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
 * mpegtool - routines kindly borrowed from d4rk0n3, and then refactored/optimized
 * a bit.
 *
 * $Id: mpegtool.c,v 1.2 2003/01/22 14:31:29 sorend Exp $
 */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include "mpegtool.h"
#include "mpegtoolconsts.h"

void mpeg_video(int fd, video_t *video) {
	unsigned char	header[] = { 0, 0, 1, 179 };
	unsigned char	buf[8];
	short		width = 0;
	short		height = 0;
	unsigned char	aspect_ratio;
	unsigned char	fps = 0;
	short		t = 0;

	lseek(fd, 0, SEEK_SET);

	while (read(fd, buf, 1) == 1) {
		if (*buf == *(header + t)) {

			t++;
			if ( t == sizeof(header) ) {
				read(fd, buf, 8);
				memcpy(&t, buf, 2);

				t = *(buf + 1) >> 4;
				width = (*buf << 4) + t;
				height = ((*(buf + 1) - (t << 4)) << 4) + *(buf + 2);

				aspect_ratio = *(buf + 3) >> 4;
				fps = *(buf + 3) - (aspect_ratio << 4);
				break;
			}
		} else if ( *buf == 0 ) {
			t = (t == 2 ? 2 : 1);
		} else {
			t = 0;
		}
	}

	video->height = height;
	video->width = width;
	video->fps = fps_s[fps > 8 ? 0 : fps];
	video->aspect_ratio = aspect_ratio;
}



void avi_video(int fd, video_t *video) {
	char	tfps[10];
	unsigned char	buf[56];
	int		fps;

	if (lseek(fd, 32, SEEK_SET) != -1 &&
		read(fd, buf, 56) == 56 ) {

		memcpy(&fps, buf, 4);
		if ( fps > 0 ) {
			memcpy(&video->width, buf + 32, 4);
			memcpy(&video->height, buf + 36, 4);
			sprintf(tfps, "%i", 1000000 / fps);
			video->fps = strdup(tfps);
		} else {
			video->height = 0;
			video->width = 0;
			video->fps = fps_s[0];
		}
	}

};


void mpeg_audio_info(int fd, audio_t *audio) {
	int		t_genre;
	int		n;
	int		tag_ok = 0;
	unsigned char	header[4];
	unsigned char	version;
	unsigned char	layer;
	unsigned char	protected = 1;
	unsigned char	t_bitrate;
	unsigned char	t_samplingrate;
	unsigned char	channelmode;
	short		bitrate;
	short		br_v1_l3[]  = { 0, 32, 40, 48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 0 };
	short		br_v1_l2[]  = { 0, 32, 48, 56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, 0 };
	short		br_v1_l1[]  = { 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0 };
	short		br_v2_l1[]  = { 0, 32, 48, 56,  64,  80,  96, 112, 128, 144, 160, 176, 192, 224, 256, 0 };
	short		br_v2_l23[] = { 0,  8, 16, 24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0 };
	unsigned	samplingrate = 0;
	unsigned	sr_v1[] = { 44100, 48000, 32000, 0 };
	unsigned	sr_v2[] = { 22050, 24000, 16000, 0 };
	unsigned	sr_v25[] = { 11025, 12000, 8000, 0 };

	lseek(fd, 0, SEEK_SET);
	
	n = 2;
	while ( read(fd, header + 2 - n, n) == n ) {
		if ( *header == 255 ) {
			n = 2;
			if (*(header + 1) >= 224) {
				n = 0;
				break;
			} else {
				n = 2;
			}
		} else {
			if (*(header + 1) == 255 ) {
				*header = *(header + 1);
				n = 1;
			} else {
				n = 2;
			}
		}
	};
	
	if ( n == 0 ) { /* mp3 header */
		*(header + 1) -= 224;
		
		read(fd, header + 2, 2);
		
		version = *(header + 1) >> 3;
		layer = (*(header + 1) - (version << 3)) >> 1;
		if ( ! *(header + 1) & 1 ) protected = 0;
		t_bitrate = *(header + 2) >> 4;
		t_samplingrate = *(header + 2) - (t_bitrate << 4) >> 2;
		switch ( version ) {
		case 3:
			samplingrate = sr_v1[t_samplingrate];
			switch ( layer ) {
			case 1:
				bitrate = br_v1_l3[t_bitrate];
				break;
			case 2:
				bitrate = br_v1_l2[t_bitrate];
				break;
			case 3:
				bitrate = br_v1_l1[t_bitrate];
				break;
			}
			break;
		case 0:
			samplingrate = sr_v25[t_samplingrate];
		case 2:
			if ( ! samplingrate ) {
				samplingrate = sr_v2[t_samplingrate];
			}
			switch ( layer ) {
			case 3:
				bitrate = br_v2_l1[t_bitrate];
				break;
			case 1:
			case 2:
				bitrate = br_v2_l23[t_bitrate];
				break;
			}
			break;
		}
		channelmode = *(header + 3) >> 6;
		
		sprintf(audio->samplingrate, "%i", samplingrate);
		sprintf(audio->bitrate, "%i", bitrate);
		audio->codec = codec_s[version];
		audio->layer = layer_s[layer];
		audio->channelmode = chanmode_s[channelmode];
		
		/* ID3 TAG */
		
		lseek(fd, -128, SEEK_END);
		read(fd, header, 3);
		if ( memcmp(header, "TAG", 3) == 0 ) { /* id3 tag */
			tag_ok = 1;
			read(fd, audio->id3_title, 30);
			read(fd, audio->id3_artist, 30);
			read(fd, audio->id3_album, 30);
			
			lseek(fd, -35, SEEK_END);
			read(fd, audio->id3_year, 4);
			if ( tolower(audio->id3_year[1]) == 'k' ) {
				memcpy(header, audio->id3_year, 3);
				sprintf(audio->id3_year, "%c00%c", *header, *(header + 2));
			}
			
			lseek(fd, -1, SEEK_END);
			read(fd, header, 1);
			t_genre = (int)*header;
			if ( t_genre < 0 ) t_genre += 256;
			if ( t_genre > 148 ) t_genre = 148;
			
			audio->id3_genre = genre_s[t_genre];
			audio->id3_year[4] =
				audio->id3_artist[30] =
				audio->id3_title[30] =
				audio->id3_album[30] = 0;
 		}
	} else { /* header is broken, shouldnt crc fail? */
		strcpy(audio->samplingrate, "0");
		strcpy(audio->bitrate, "0");
		audio->codec = codec_s[1];
		audio->layer = layer_s[0];
		audio->channelmode = chanmode_s[4];
	}
	
	if ( tag_ok == 0 ) {
		strcpy(audio->id3_year, "0000");
		strcpy(audio->id3_title, "Unknown");
		strcpy(audio->id3_artist, "Unknown");
		strcpy(audio->id3_album, "Unknown");
		audio->id3_genre = genre_s[148];
	}

}
