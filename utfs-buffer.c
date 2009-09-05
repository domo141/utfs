/*
 * $Id; utfs-buffer.c $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *	Copyright (c) 2007 Tomi Ollila
 *	    All rights reserved
 *
 * Created: Thu Dec 27 17:55:12 EET 2007 too
 * Last modified: Fri 04 Sep 2009 18:12:22 EEST too
 */

#include <unistd.h>
#include <string.h> /* for memmove() */
#if WIN32
#include "w32x.h"
#else
#include <sys/uio.h>
#endif
#include "util.h"

#include "utfs-buffer.h"

#if UTFS_SERVER
#define UTFS_DATABUFSIZE 36892 /* 32768 + 4096 + 12 + 16 */
#else
#define UTFS_DATABUFSIZE 4116 /* 4096 + 16 + 4 */
#endif

#if WIN32 // this modules reads *only* from (tcp) socket
#define read(s, b, l) recv(s, b, l, 0)
#endif

struct {
    sockfd fd;
    uint8_t * p;
#if DEBUG
    uint8_t canary0[16];
#endif
    uint8_t b[UTFS_DATABUFSIZE];
#if DEBUG
    uint8_t canary1[16];
#endif
} BUFFER;

uint8_t * utfs_buffer_init(sockfd fd)
{
    BUFFER.fd = fd;
    BUFFER.p = BUFFER.b;
    return BUFFER.b;
}

#if 0 /* travis2utfs is used instead... */
int read_wrapper(int fd, char * data, int len);
{
    if (len > 3) len = 3;
    return read(fd, data, len);
}
#define read read_wrapper
#endif

#if DEBUG
static void utfs_buffer_chkcanary(void)
{
    static char xbuf[16];

    if (memcmp(BUFFER.canary0, xbuf, 16) != 0)
	die("Canary 0 XXX");
    if (memcmp(BUFFER.canary1, xbuf, 16) != 0)
	die("Canary 1 XXX");
}
#endif

uint8_t * utfs_buffer_reserve(uint8_t * p, int needbytes)
{
    int havebytes = BUFFER.p - p;

    d1(("utfs_buffer_reserve(p offset %d, needbytes %d), havebytes = %d",
	p - BUFFER.b, needbytes, havebytes));

    if (havebytes >= needbytes)
	return p;

    if (havebytes <= 0)
	p = BUFFER.p = BUFFER.b;
    else if (BUFFER.b + UTFS_DATABUFSIZE - p < needbytes) {
	memmove(BUFFER.b, p, havebytes); /* expect optimized version */
	BUFFER.p = BUFFER.b + havebytes;
	p = BUFFER.b;
    }

    do {
	int s = UTFS_DATABUFSIZE - (BUFFER.p - BUFFER.b);
	int l = read(BUFFER.fd, BUFFER.p, s);
	d1(("%d = read(%d, %d, %d)", l, BUFFER.fd, BUFFER.p - BUFFER.b, s));
	if (l <= 0)
	    return null;
	BUFFER.p += l;
	havebytes += l;
    } while (havebytes < needbytes);

    d1x( utfs_buffer_chkcanary() );
    return p;
}

uint8_t * utfs_buffer_scatter(uint8_t * base, int done, int all)
{
    int len = all - done;
    int pad = all & 3;
    int i, tlen;
    struct iovec iov[3];
    uint8_t pad000[3];

    d1(("utfs_buffer_scatter(%p, %d, %d) len %d", base, done, all, len));

    iov[0].iov_base = base + done;
    iov[0].iov_len = len;

    if (pad) {
	pad = 4 - pad;
	iov[1].iov_base = pad000;
	iov[1].iov_len = pad;
	i = 2;
	tlen = len + pad;
    }
    else {
	i = 1;
	tlen = len;
    }
    iov[i].iov_base = BUFFER.b;
    iov[i++].iov_len = UTFS_DATABUFSIZE;

    while (1) {
	int l = readv(BUFFER.fd, iov, i);
	d1(("%d = readv(%d, %p, %i) tlen %d", l, BUFFER.fd, iov, i, tlen));

	if (l <= 0)
	    return null;
	tlen -= l;
	if (tlen <= 0)
	    break;

	if (S2U(l, int,,) < iov[0].iov_len) {
	    iov[0].iov_base = (uint8_t *)iov[0].iov_base + l;
	    iov[0].iov_len -= l;
	}
	else {
	    /* there is still pad to be cleared out */
	    l -= iov[0].iov_len;
	    do {
		/* (uint8_t *)iov[1].iov_base += l; */
		iov[1].iov_len -= l;
		l = readv(BUFFER.fd, iov + 1, i - 1);
		d1(("%d = readv(%d, %p, %i) tlen %d",
		    l, BUFFER.fd, iov + 1, i - 1, tlen));

		if (l <= 0)
		    return null;
		tlen -= l;
	    } while (tlen > 0);
	    break; /* while (1) */
	}
    }
    BUFFER.p = BUFFER.b - tlen;
    d1(("_scatter() exit. tlen %d", tlen));
    return BUFFER.b;
}

int utfs_buffer_rest(uint8_t * p)
{
    d1(("%d = utfs_buffer_rest(p)", BUFFER.p - p));

    return BUFFER.p - p;
}

uint8_t * utfs_buffer_discard(uint8_t * p, int len)
{
    d1(("utfs_buffer_discard(len %d)", len));
    do {
	int rest = utfs_buffer_rest(p);
	if (rest >= len)
	    return p + len;
	rest &= ~3;
	p += rest;
	len -= rest;
	p = utfs_buffer_reserve(p, 4);
    } while (p);

    return null;
}


/*
 * Local variables:
 * mode: c
 * c-file-style: "stroustrup"
 * tab-width: 8
 * End:
 */
