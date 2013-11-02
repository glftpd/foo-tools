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
/**
 * Library to take care of accessing glftpd's online-list.
 *
 **
 * $Id: who.h,v 1.3 2003/03/28 18:01:20 sorend Exp $
 * Author: Flower
 */

#ifndef _who_h
#define _who_h

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <strings.h>
#include <stdio.h>

#define TRANS_NONE 0
#define TRANS_UP 1
#define TRANS_DOWN 2

typedef struct online_t {
  int is_good;
  struct ONLINE *online;

  int shmid;
  struct shmid_ds ipcbuf;
} online_t;

typedef int32_t time32_t;

/* Force structure alignment to 4 bytes (for 64bit support). */
#pragma pack(push, 4)

/* 32-bit timeval data structure (for 64bit support). */
typedef struct {
    int32_t tv_sec;
    int32_t tv_usec;
} timeval32_t;

struct ONLINE {
    char        tagline[64];     /* The users tagline */
    char        username[24];    /* The username of the user */
    char        status[256];     /* The status of the user, idle, RETR, etc */
//    int16_t     ssl_flag;        /* 0 = no ssl, 1 = ssl on control, 2 = ssl on control and data */
    int16_t ssl_flag;
    char        host[256];       /* The host the user is comming from (with ident) */
    char        currentdir[256]; /* The users current dir (fullpath) */
    int32_t     groupid;         /* The groupid of the users primary group */
    int32_t     login_time;      /* The login time since the epoch (man 2 time) */
    timeval32_t tstart;          /* replacement for last_update. */
    timeval32_t txfer;           /* The time of the last succesfull transfer. */
    uint64_t    bytes_xfer;      /* bytes transferred so far. */
    uint64_t    bytes_txfer;     /* bytes transferred in the last loop (speed limiting) */
    int32_t     procid;          /* The processor id of the process */
};

/*
 * Con-/Deconstructors for who structures. Remember to call who_deinit(.. )
 */
int who_init(online_t *c, key_t ipckey);
int who_deinit(online_t *c);

/*
 * Returns ONLINE structure for a specific node. Returns NULL if 'i' is too large
 * or noone is online on the node.
 */
struct ONLINE *who_getnode(online_t *c, int i);

/*
 * Returns pointer to entry for the ONLINE structures.
 */
struct ONLINE *who_getonline(online_t *c);

/*
 * Returns number of ONLINE structures in the shared mem.
 */
int who_online_max(online_t *c);

/*
 * Returns number of online users.
 */
int who_online_count(online_t *c);

/*
 * Returns one of either TRANS_NONE, TRANS_UP or TRANS_DOWN
 */
int who_transfer_direction(struct ONLINE *o);

/*
 * Fills 'b' with the file user is transfering (full path to it).
 *
 * returns 1 on success, and 0 on error.
 */
int who_transfer_file(struct ONLINE *o, char *b);

/*
 * Returns the speed of the user's transfer. -1 if there is no transfer.
 */
double who_transfer_speed(struct ONLINE *o);


#endif
