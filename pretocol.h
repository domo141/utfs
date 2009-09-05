/*
 * $Id; pretocol.h $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *	Copyright (c) 2007 Tomi Ollila
 *	    All rights reserved
 *
 * Created: Sun Aug 26 22:40:13 EEST 2007 too
 * Last modified: Fri 04 Sep 2009 19:57:15 EEST too
 */

#ifndef PRETOCOL_H
#define PRETOCOL_H

#ifndef SOCKFD_H
#include "sockfd.h"
#endif

int readwt(int fd, char * buf, int len, int timeout);

void doweaksecretexchange(const unsigned char * secrets,
			  sockfd sd1, sockfd sd2);
int dobindandlisten(const char * ip, int port, bool fatal);
int doconnect(const unsigned char * secrets, const char * host, int port);
int doaccept(const unsigned char * secrets, int ssd);

#endif /* PRETOCOL_H */

/*
 * Local variables:
 * mode: c
 * c-file-style: "stroustrup"
 * tab-width: 8
 * End:
 */
