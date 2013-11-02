/*
 * Routines to move around with filedescriptors.
 *
 * $Id: fd.h,v 1.1 2001/11/29 10:02:19 sd Exp $
 */

#ifndef _fd_h
#define _fd_h

int fd_copy(int to, int from);

int fd_move(int to, int from);

#endif

