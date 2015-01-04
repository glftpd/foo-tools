

#ifndef _simple_entry_h
#define _simple_entry_h

#include <collection/hashtable.h>
#include <thread/threadpool.h>
#include <netinet/in.h>

#define DEFAULT_TIMEOUT 30

#define DEFAULT_ERRORTMPL "421 Error: %s\n"

#define LOCALQUEUE 10

// buffer to use for reading client requests.
#define BUFSIZE 4096

struct cycleip {
    struct sockaddr_in sockname;
    long is_available;

    struct cycleip *next;
};

typedef struct cycleip cycleip_t;


struct bouncer_arg {
	int client_fd;
	int server_fd;

    struct sockaddr_in client_name;
    struct sockaddr_in server_name;
    
    int accesses;

    unsigned long bytes_in, bytes_out;
    
	threadpool_t *pool;
};

typedef struct bouncer_arg bouncer_arg_t;

#define MSG_ERROR_CONNECT_REMOTE "Cannot connect to remote"


#define PROPERTY_MSG_WELCOME "entry_msg_welcome"
#define PROPERTY_MSG_BUSY "entry_msg_busy"
#define PROPERTY_MSG_ERROR "entry_msg_error"

#define PROPERTY_ENTRY_HOST_MAX "entry_host_max"
#define PROPERTY_ENTRY_HOST "entry_host_%d"
#define PROPERTY_ENTRY_PORT "entry_port"
#define PROPERTY_ENTRY_IP "entry_ip"
#define PROPERTY_ENTRY_SIMUL_MAX "entry_simul_max"
#define PROPERTY_DATA_ENABLE "data_enable"
#define PROPERTY_IDENT_ENABLE "ident_enable"

#define PROPERTY_DATA_TIMEOUT "data_timeout"
#define PROPERTY_CONNECT_TIMEOUT "connect_timeout"

#define PROPERTY_TLS_ENABLE "tls_enable"

#endif
