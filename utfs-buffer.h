/*
 * $Id; utfs-buffer.h $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *	Copyright (c) 2007 Tomi Ollila
 *	    All rights reserved
 *
 * Created: Thu Dec 27 17:51:30 EET 2007 too
 * Last modified: Fri 04 Sep 2009 18:12:22 EEST too
 */

#ifndef UTFS_BUFFER_H
#define UTFS_BUFFER_H

#include <stdint.h>

#include "sockfd.h"

uint8_t * utfs_buffer_init(sockfd fd);
uint8_t * utfs_buffer_reserve(uint8_t * p, int needbytes);
uint8_t * utfs_buffer_scatter(uint8_t * base, int done, int all);
int       utfs_buffer_rest(uint8_t * p);
uint8_t * utfs_buffer_discard(uint8_t * base, int len);

#endif /* UTFS-BUFFER_H */

/*
 * Local variables:
 * mode: c
 * c-file-style: "stroustrup"
 * tab-width: 8
 * End:
 */
