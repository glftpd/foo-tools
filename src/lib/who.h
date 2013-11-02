/**
 * Library to take care of accessing glftpd's online-list.
 *
 **
 * $Id: who.h,v 1.3 2001/11/25 09:51:58 sd Exp $
 * Author: Flower
 */

#ifndef _who_h
#define _who_h

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#define TRANS_NONE 0
#define TRANS_UP 1
#define TRANS_DOWN 2

typedef struct online_t {
  int is_good;
  struct ONLINE *online;

  int shmid;
  struct shmid_ds ipcbuf;
} online_t;

struct ONLINE {
  char tagline[64];
  char username[24];
  char status[256];
  char host[256];
  char currentdir[256];
  long groupid;
  time_t login_time;
  struct timeval tstart;
  unsigned long bytes_xfer;
  pid_t procid;
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
