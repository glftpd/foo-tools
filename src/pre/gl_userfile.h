/*
 * Some functions to manipulate stats in glftpd's userfiles.
 *
 * <tanesha@tanesha.net>
 */

#ifndef _gl_userfile_h
#define _gl_userfile_h

/*
 * Gets ratio from a userfile.
 */
int gl_userfile_get_ratio(char *userfile, int section);

/*
 * Updates stats.
 */
int gl_userfile_add_stats(char *userfile, int files, long kbytes, int seconds, long credits, int stat_section, int cred_section) {


#endif
