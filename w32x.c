/*
 *
 * Created: Mon 11 May 2009 12:28:12 EEST
 */

#include "w32x.h"

static int sock_readfully(sockfd fd, char * buf, int len)
{
    int tl = 0;
    do {
	int l = recv(fd, buf, len, 0);
	if (l <= 0)
	    return tl;
	tl += l; buf += l; len -= l;
    } while (len > 0);
    return tl;
}

static int sock_writefully(sockfd fd, char * buf, int len)
{
    int tl = 0;
    do {
	int l = send(fd, buf, len, 0);
	if (l <= 0)
	    return tl;
	tl += l; buf += l; len -= l;
    } while (len > 0);
    return tl;
}



/* recvmsg !!!??+ */

ssize_t readv(sockfd fd, const struct iovec * iov, int iovcnt)
{
    int i, tl = 0;

    for (i = 0; i < iovcnt; i++) {
	int l = sock_readfully(fd, iov[i].iov_base, iov[i].iov_len);
	if (l != iov[i].iov_len)
	    return -1;
	tl += l;
    }
    return tl;
}

/* sendmsg !!!??+ */

ssize_t writev(sockfd fd, const struct iovec * iov, int iovcnt)
{
    int i, tl = 0;

    for (i = 0; i < iovcnt; i++) {
	int l = sock_writefully(fd, iov[i].iov_base, iov[i].iov_len);
	if (l != iov[i].iov_len)
	    return -1;
	tl += l;
    }
    return tl;
}


#define USE_EXTERNAL_GPL_CODE 1
#include "w32x_ext.hc"
