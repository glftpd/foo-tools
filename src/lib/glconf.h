#ifndef GL_GLOBALS_H
#define GL_GLOBALS_H

#include <stdint.h>
#include <inttypes.h>
#include <sys/param.h>

#define GLCONF	"/home/ftp/etc/glftpd.conf"	/* Default config path */

typedef int32_t time32_t;

/* Force structure alignment to 4 bytes (for 64bit support). */
#pragma pack(push, 4)

/* 32-bit timeval data structure (for 64bit support). */
typedef struct {
    int32_t tv_sec;
    int32_t tv_usec;
} timeval32_t;

struct dirlog {
    uint16_t status;        /* 0 = NEWDIR, 1 = NUKE, 2 = UNNUKE, 3 = DELETED */
    int32_t  uptime;        /* Creation time since epoch (man 2 time) */
    uint16_t uploader;      /* The userid of the creator */
    uint16_t group;         /* The groupid of the primary group of the creator */
    uint16_t files;         /* The number of files inside the dir */
    uint64_t bytes;         /* The number of bytes in the dir */
    char     dirname[255];  /* The name of the dir (fullpath) */
    char     dummy[8];      /* Unused, kept for compatibility reasons */
};

struct lastonlog {
    char     uname[24];     /* username */
    char     gname[24];     /* users primary group */
    char     tagline[64];   /* users tagline */
    int32_t  logon;         /* users logon time */
    int32_t  logoff;        /* users logoff time */
    uint32_t upload;        /* bytes uploaded */
    uint32_t download;      /* bytes downloaded */
    char     stats[6];      /* what did the user do ? */
};

struct nukelog {
    uint16_t status;        /* 0 = NUKED, 1 = UNNUKED */
    int32_t  nuketime;      /* The nuke time since epoch (man 2 time) */
    char     nuker[12];     /* The name of the nuker */
    char     unnuker[12];   /* The name of the unnuker */
    char     nukee[12];     /* The name of the nukee */
    uint16_t mult;          /* The nuke multiplier */
    float    bytes;         /* The number of bytes nuked */
    char     reason[60];    /* The nuke reason */
    char     dirname[255];  /* The dirname (fullpath) */
    char     dummy[8];      /* Unused, kept for compatibility reasons */
};

struct oneliner {
    char    uname[24];      /* The user that added the oneliner */
    char    gname[24];      /* The primary group of the user who added the oneliner */
    char    tagline[64];    /* The tagline of the user who added the oneliner */
    int32_t timestamp;      /* The time the message was added (epoch) */
    char    message[100];   /* The message (oneliner) */
};

struct dupefile {
    char    filename[256];
    int32_t timeup;
    char    uploader[25];
};

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

struct dirlog_cache_entry {
    char name[MAXPATHLEN];
    int32_t fpos;
    int32_t status;                 //0-unused, -1-not found in dirlog, 1-found in dirlog
};

/* Restore default structure alignment for non-critical structures. */
#pragma pack(pop)

static inline void tmFROM32(struct timeval *to,timeval32_t *from) {
 to->tv_sec=from->tv_sec;
 to->tv_usec=from->tv_usec;
}

static inline void tmTO32(timeval32_t *to,struct timeval *from) {
 to->tv_sec=from->tv_sec;
 to->tv_usec=from->tv_usec;
}


#endif




