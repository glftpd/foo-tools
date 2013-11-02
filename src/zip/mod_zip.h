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

#ifndef _mod_zip_h
#define _mod_zip_h

#include <collection/hashtable.h>

// properties this module wants.
#define PROPERTY_ZIP_EXTRACT "zip_extract"
#define PROPERTY_ZIP_SHOWFILE "zip_showfile"

#define PROPERTY_ZIP_AD_FILE "zip_adfilename"
#define PROPERTY_ZIP_AD "zip_ad"
#define PROPERTY_ZIP_AD_COMMENT "zip_adcomment"

#define PROPERTY_ZIP_SUCCESS "zip_success"

#define PROPERTY_ZIP_UNWANTEDDIR "zip_unwanteddir"

#define ZIP_LAST_ZF_VARS_PROP "tmp_zip_last"

#define ZIP_STATUS_EXTRACTED "Unpacked"
#define ZIP_STATUS_TESTED "Tested"
#define ZIP_STATUS_DELETED "Deleted"
#define ZIP_STATUS_EXISTS "Exists"

#define ZIP_TIME_FORMAT "%Y-%m-%d %H:%M"

int zip_check(hashtable_t *conf, char *file, char *dir, long crc);

#endif
