/*
 * $Id; protocol.c $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *	Copyright (c) 2007 Tomi Ollila
 *	    All rights reserved
 *
 * Created: Sun Aug 26 19:08:59 EEST 2007 too
 * Last modified: Sat 02 Oct 2010 14:03:58 EEST too
 */

#include <unistd.h>
#if !WIN32
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <errno.h>
#endif

#include <string.h>

#define UTFS_COMMON 1
#include "utfs.h" /* for xverbose()... */

#include "sockfd.h"

#include "util.h"

#include "version.h"

static void pollread(sockfd fd, int timeout)
{
#if WIN32
    fd_set readfds;
    struct timeval tval;
    int i;

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    tval.tv_sec = timeout / 1000;
    tval.tv_usec = (timeout % 1000) * 1000;

    switch (select(1, &readfds, 0, 0, &tval)) {
    case -1:
	die("%C: select failed:");
    case 0:
	die("%C: timeout");
    }
#else
    struct pollfd pfd;
    int i;
    pfd.fd = fd;
    pfd.events = POLLIN;

    switch (poll(&pfd, 1, timeout)) {
    case -1:
	die("%C: poll failed:");
    case 0:
	die("%C: timeout");
    }
#endif
}

/* In case eof (or read error) happens before initial secret read
 * return 0 (so that in case of connect, retry). pollread() may
 * still make this to fail (as it should be).
 */

int readwt(sockfd fd, unsigned char * buf, int len, int timeout)
{
    int i;
    pollread(fd, timeout);
    i = sockread(fd, buf, len);
    if (i <= 0) {
#if 0
	die("%C: eof or read error");
#else
	return 0; }

#if 0 /* d1(()) */
    {int j; for (j = 0; j < i; j++) printf("%02x ", buf[j]); puts("");}
#endif

    return i;
}

static int tsocket(bool dobind)
{
    int one = 1;
    sockfd sd = socket(AF_INET, SOCK_STREAM, 0);

#if WIN32
    if (sd == INVALID_SOCKET)
	die("%C: socket failed:");
#else
    if (sd < 0)
	die("%C: socket failed:");
#endif
    setsockopt(sd, SOL_SOCKET,
	       dobind? SO_REUSEADDR: SO_KEEPALIVE, &one, sizeof one);
    setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);

    return sd;
}

int doweaksecretexchange(const unsigned char * secrets,
			  sockfd sd_in, sockfd sd_out)
{
    unsigned char * p;
    int l, i;
    unsigned char buf[4];

#if 0
    for (i = 0; i < 8; i++) {
	unsigned char c = secrets[i];
	printf("%d %02x '%c'\n", i, c, isprint(c)? c: '.');
    }
    printf("\n");
#endif

    (void)sockwrite(sd_out, secrets, 4);

    for (p = buf, i = sizeof buf; i; p += l, i -= l)
	if ((l = readwt(sd_in, p, i, 10 * 1000)) == 0)
	    return 0;

    if (memcmp(buf, secrets + 4, 4) != 0)
	die("%C: Secret mismatch");

    (void)sockwrite(sd_out, "", 1);
    readwt(sd_in, buf, 1, 10 * 1000);
    if (buf[0] != 0)
	die("%C: Peer did not accept our secret -- actually junk in stream");
    return 1;
}

/* XXX add timeout handling */
sockfd doconnect(const unsigned char * secrets, const char * host, int port)
{
    struct hostent * remote;
    struct sockaddr_in addr;
    sockfd sd;
    int i;

    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    remote = gethostbyname(host);
    if (remote == null)
	die("%C: gethostbyname on '%s' returned no value", host);

    memcpy(&addr.sin_addr, remote->h_addr, sizeof addr.sin_addr);

    xverbose(("%C: connect '%s' (%s)", host, inet_ntoa(addr.sin_addr)));

    for (i = 0;; i++) {
	sd = tsocket(false);
	if (connect(sd, (struct sockaddr *)&addr, sizeof addr) < 0) {
#if WIN32
	    if (i >= 5 || WSAGetLastError() != WSAECONNREFUSED)
		die("%C: connect failed:");
#define sleep(s) Sleep(s * 1000)
#else
	    if (i >= 5 || errno != ECONNREFUSED)
		die("%C: connect failed:");
#endif
	    close(sd);
	    sleep(1);
	    continue;
	}
	if (doweaksecretexchange(secrets, sd, sd))
	    break;
	close(sd);
    }
#endif
    return sd;
}

#if NO_BIND_LISTEN_ACCEPT /* for some virus scanners... :( */

sockfd dobindandlisten(const char * ip, int port, bool fatal)
{
    die("%C: listening socket not supported in this version");
}
inline sockfd doaccept(const unsigned char * secrets, sockfd ssd)
{
#if WIN32
    return INVALID_SOCKET;
#else
    return -1;
#endif
}

#else /* not NO_BIND_LISTEN_ACCEPT */

sockfd dobindandlisten(const char * ip, int port, bool fatal)
{
    struct sockaddr_in addr;
    sockfd ssd = tsocket(true);

    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    (void)ip; /* XXX fiz */

    if (bind(ssd, (struct sockaddr *)&addr, sizeof addr) < 0) {
	if (fatal)
	    die("%C: bind failed:");
	else
	    return -1;
    }
    listen(ssd, 1);

    return ssd;
}

sockfd doaccept(const unsigned char * secrets, sockfd ssd)
{
    struct sockaddr_in addr;
#if W32_HAXES
    int addrlen = sizeof addr;
#else
    socklen_t addrlen = sizeof addr;
#endif
    sockfd sd;
    int one = 1;

    memset(&addr, 0, sizeof addr);
    pollread(ssd, 60 * 1000);
    sd = accept(ssd, (struct sockaddr *)&addr, &addrlen);

#if WIN32
    if (sd == INVALID_SOCKET)
	die("%C: accept failed:");
#else
    if (sd < 0)
	die("%C: accept failed:");
#endif
    setsockopt(sd, SOL_SOCKET,SO_KEEPALIVE, &one, sizeof one);
    setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);

    sockclose(ssd);
    if (doweaksecretexchange(secrets, sd, sd))
	return sd;
    else
	die("%C: eof or read error");
}
#endif /* not NO_BIND_LISTEN_ACCEPT */

/*
 * Local variables:
 * mode: c
 * c-file-style: "stroustrup"
 * tab-width: 8
 * End:
 */
