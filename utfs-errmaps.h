/*
 * $Id; utfs-errmaps.h $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *	Copyright (c) 2008 Tomi Ollila
 *	    All rights reserved
 *
 * Created: Mon Jan 07 21:11:48 EET 2008 too
 * Last modified: Fri 04 Sep 2009 18:12:22 EEST too
 */

#ifndef UTFS_ERRMAPS_H
#define UTFS_ERRMAPS_H

#include <errno.h>
#if WIN32
#include <winsock2.h>
#define EDEFAULT EINVAL
#else
#define EDEFAULT EPROTO
#endif
extern const unsigned char utfs_server_errmap[];
extern const unsigned int  utfs_server_errmap_size;

extern const unsigned char utfs_client_errmap[];
extern const unsigned int  utfs_client_errmap_size;

#endif /* UTFS_ERRMAPS_H */

/*
 * Local variables:
 * mode: c
 * c-file-style: "stroustrup"
 * tab-width: 8
 * End:
 */
