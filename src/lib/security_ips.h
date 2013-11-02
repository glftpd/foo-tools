/*
 * $Id: security_ips.h,v 1.1.1.1 2001/04/30 10:49:37 sd Exp $
 */

/*
 * The list of local ips.
 *
 * If you need to get the number for some ip, then compile the module with DEBUG=1
 * and it will output the number of the ip that its trying to match.
 */
struct ip_t {
	unsigned long ip;
} ips [] = {
	// 10.0.0.3 (flowers local lan box).
	50331658,
	0
};

