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

#ifndef _mpegtool_h
#define _mpegtool_h



struct video {
	short height;
	short width;
	char *fps;
	unsigned char aspect_ratio;
};

typedef struct video video_t;

struct audio {
	char   id3_artist      [31];
	char   id3_title       [31];
	char   id3_album       [31];
	char   id3_year        [5];
	char   bitrate         [5];
	char   samplingrate    [6];
	char   *id3_genre;
	char   *layer;
	char   *codec;
	char   *channelmode;
};

typedef struct audio audio_t;


// get video info from mpeg file
void mpeg_video(int fd, video_t *video);

// get video info of avi file
void avi_video(int fd, video_t *video);

// get audio info from mpeg
void mpeg_audio_info(int fd, audio_t *audio);



#endif
