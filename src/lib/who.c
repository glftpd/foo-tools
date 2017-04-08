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
 * $Id: who.c,v 1.2 2003/01/22 14:31:29 sorend Exp $
 * Maintained By: Flower
 */

#include "who.h"
#include <string.h>

int who_init(online_t *c, key_t key) {
  c->is_good = 0;

  c->shmid = shmget(key, 0, 0);

  if (c->shmid == -1)
    return 0;

  c->online = (struct ONLINE*)shmat(c->shmid, NULL, SHM_RDONLY);

  if (c->online == (struct ONLINE*)-1)
    return 0;

  shmctl(c->shmid, IPC_STAT, &c->ipcbuf);

  c->is_good = 1;

  return 1;
}

int who_deinit(online_t *c) {
  shmctl(c->shmid, IPC_STAT, &c->ipcbuf);
  if (c->ipcbuf.shm_nattch <= 1)
    shmctl(c->shmid, IPC_RMID, 0);

  shmdt(c->online);

  c->is_good = 0;

  return 1;
}

int who_online_max(online_t *c) {
  if (!c->is_good)
    return 0;

  return c->ipcbuf.shm_segsz/sizeof(struct ONLINE);
}

struct ONLINE *who_getonline(online_t *c) {
  if (!c->is_good)
    return 0;

  return (struct ONLINE*)c->online;
}

struct ONLINE *who_getnode(online_t *c, int i) {
  if (!c->is_good)
    return 0;

  if (i >= who_online_max(c))
    return NULL;

  if (c->online[i].procid == 0)
    return NULL;

  return (struct ONLINE*)&c->online[i];
}

int who_online_count(online_t *c) {
  int i, max = 0, online_num = 0;

  max = who_online_max(c);

  for (i = 0; i < max; i++)
    if (c->online[i].procid != 0)
      online_num++;

  return online_num;
}

int who_transfer_direction(struct ONLINE *o) {
  if (!o)
    return TRANS_NONE;

  if (o->bytes_xfer < 1)
    return TRANS_NONE;

  if (!strncasecmp(o->status, "STOR", 4) || !strncasecmp(o->status, "APPE", 4))
    return TRANS_UP;
  else if (!strncasecmp(o->status, "RETR", 4))
    return TRANS_DOWN;
  else
    return TRANS_NONE;
}

int who_transfer_file(struct ONLINE *o, char *b) {
  char *s, *r, buf[300];

  if (who_transfer_direction(o) == TRANS_NONE)
    return 0;

  strcpy(buf, o->status + 5);

  r = strrchr(buf, '/');

  if (r)
    s = r + 1;
  else
    s = (char*) &buf;

  r = s;
  while (*r)
    if ((*r == '\n') || (*r == '\r'))
      *r = 0;
    else
      r++;

  if (strstr(o->currentdir, s))
    sprintf(b, "%s", o->currentdir);
  else
    sprintf(b, "%s/%s", o->currentdir, s);

  return 1;
}

double who_transfer_speed(struct ONLINE *o) {
  struct timeval tstop;
  double delta, rate;
  unsigned long xfer;

  if (!o)
    return -1;

  xfer = o->bytes_xfer;

  if (xfer < 1)
    return -1;

  gettimeofday(&tstop, (struct timezone*) 0);
  delta = ((tstop.tv_sec * 10.) + (tstop.tv_usec / 100000.)) -
    ((o->tstart.tv_sec * 10.) + (o->tstart.tv_usec / 100000.));

  delta = delta / 10.;

  rate = ((xfer / 1024.0) / (delta));

  if (!rate) rate++;

  return (double)(rate);
}

