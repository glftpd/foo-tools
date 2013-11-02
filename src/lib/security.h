/*
 * Some lib to do smth about security  /flower
 *
 * $Id: security.h,v 1.1.1.1 2001/04/30 10:49:37 sd Exp $
 */

/*
 * The network interface which the network ip must be on.
 */
#define NETINTERFACE "eth0"

/*
 * Fills ip with the local ip of a box.
 *
 * Returns 1 if local ip was found, and 0 on error. 
 */
int get_local_ip(char *ip);

/*
 * Matches some ip against list of allowed ips.
 *
 * Returns 1 if the ip is allowed, and 0 if not.
 */
int is_secure_ip(char *ip);

