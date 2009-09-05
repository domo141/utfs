#if 0 /*
 set -e; TRG=`echo $0 | sed 's/.c$//'`; rm -f "$TRG"
 WARN="-Wall -Wstrict-prototypes -pedantic -Wno-long-long"
 WARN="$WARN -Wcast-align -Wpointer-arith " # -Wfloat-equal #-Werror
 WARN="$WARN -W -Wwrite-strings -Wcast-qual -Wshadow" # -Wconversion
 date=`date`; set -x
 #${CC:-gcc} -ggdb $WARN "$@" -o "$TRG" "$0" -DCDATE="\"$date\""
 ${CC:-gcc} -O2 $WARN "$@" -o "$TRG" "$0" -DCDATE="\"$date\""
 exit 0
 */
#endif
/*
 * $Id; travis2utfs.c $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *	Copyright (c) 2008 Tomi Ollila
 *	    All rights reserved
 *
 * Created: Fri Jan 11 16:29:10 EET 2008 too
 * Last modified: Sat 05 Sep 2009 08:04:13 EEST too
 */

/*
 * Traffic visualizer (and splitter) for utfs
 *
 * This intentionally uses no common code with utfs, to notice
 * unintentional changes in traffic...
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

#undef null
#define null ((void*)0)

int waitmsec = 20;

/* all msgs to the same channel (stdout) */
void die(const char * format, ...)
{
    int e = errno;
    va_list ap;
    va_start(ap, format);
    vfprintf(stdout, format, ap);
    va_end(ap);
    if (format[strlen(format) - 1] == ':')
	fprintf(stdout, " %s\n", strerror(e));
    else
	fputs("\n", stdout);
    fflush(stdout);
    sleep(1);
    exit(1);
}

void printf1(const char * format, ...)
{
    char buf[1024];
    unsigned int l;
    va_list ap;
    va_start(ap, format);
    l = (unsigned int)vsnprintf(buf, sizeof buf, format, ap);
    write(1, buf, l > sizeof buf? sizeof buf - 1: l);
}

int _socket(int dobind, struct sockaddr_in *addr, int port)
{
    int one = 1;
    int sd = socket(AF_INET, SOCK_STREAM, 0);

    if (sd < 0)
        die("socket failed:");

    setsockopt(sd, SOL_SOCKET,
               dobind? SO_REUSEADDR: SO_KEEPALIVE, &one, sizeof one);
    setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);

    memset(addr, 0, sizeof *addr);
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);

    return sd;
}

void movefd(int o, int n)
{
    if (o == n)
	return;
    dup2(o, n);
    close(o);
}

void _accept(int port)
{
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof addr;
    int sd = _socket(1, &addr, port);

    movefd(sd, 3);

    if (bind(3, (struct sockaddr *)&addr, sizeof addr) < 0) {
	die("bind failed:");
    }
    listen(3, 1);

    while (1) {
	memset(&addr, 0, sizeof addr);
	printf1("Listening connection on port %d.\n", port);
	sd = accept(3, (struct sockaddr *)&addr, &addrlen);
	if (sd < 0)
	    die("accept failed:");

	printf1("Accepted connection from %s:%d.\n",
		inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

	switch (fork()) {
	case 0:
	    /* child */
	    movefd(sd, 3);
	    return;
	case -1:
	    die("fork failed:");
	default:
	    close(sd);
	    wait(&sd);
	    write(1, "\n", 1);
	}
    }
}

void _connect(const char * host, int port)
{
    struct hostent * remote;
    struct sockaddr_in addr;

    int sd = _socket(0, &addr, port);

    remote = gethostbyname(host);
    if (remote == null)
	die("gethostbyname on '%s' returned no value");

    memcpy(&addr.sin_addr, remote->h_addr, sizeof addr.sin_addr);

    printf1("Connecting %s:%d.\n", inet_ntoa(addr.sin_addr), port);

    if (connect(sd, (struct sockaddr *)&addr, sizeof addr) < 0)
	die("connect failed:");
    movefd(sd, 4);
}

void writehx(char ic, unsigned char * ibuf, int len)
{
    char buf[256], *p;
    int i;

    if (ic) {
	p = buf + 1; buf[0] = ic;
    }
    else {
	p = buf + 3; buf[0] = ' '; buf[1] = buf[2] = '.';
    }

    while (len > 0) {
	for (i = 0; i < 80 && len > 0; i++) {
	    unsigned char c = *ibuf++;
	    len--;
	    if (isprint(c)) { sprintf(p, " %c", c); p += 2; }
	    else            { sprintf(p, " %02x", c); p += 3; }
	}
	write(1, buf, p - buf);
	p = buf;
    }
}

#if 0
char fdinfo[][] = { "stdin", "stdout", "stderr", "client", "server" };
#endif

void areadfully(int fd, unsigned char * buf, int len)
{
    int l;
    while ((l = read(fd, buf, len)) != len) {
	if (l <= 0)
	    die("\nEOF/Error when reading fd %d: (%d)", fd, l);
	buf += l;
	len -= l;
    }
}

int preproto(int chrs, char d, int i, int o)
{
    unsigned char buf[4];

    areadfully(i, buf, chrs);
    write(o, buf, chrs);
    writehx(d, buf, chrs);
    write(1, "\n", 1);
    if (chrs == 4)
	return 1;
    return 0;
}

void writedata(int fd, unsigned char * buf, int len)
{
    int l;
    if ((l = write(fd, buf, len)) != len)
	die("write(...%d) != %d:", len, l);
}

#define T_OFFSET 0x7a5120ff

const char * operlist[] = {
    "private ", "getattr ", "readlink", "getdir  ", "mknod   ", "mkdir   ",
    "unlink  ", "rmdir   ", "symlink ", "rename  ", "link    ", "chmod   ",
    "chown   ", "truncate", "utime   ", "open    ", "read    ", "write   ",
    "statfs  ", "flush   ", "release ", "fsync   ", "setxattr", "getxattr",
    "listxattr", "removexattr", "opendir ", "readdir ", "releasedir",
    "fsyncdir"
};

const char * oper(uint32_t type)
{
    uint32_t type0 = type - T_OFFSET;
    static char def[12];

    if (type0 < sizeof operlist / sizeof operlist[0])
	return operlist[type0];
    snprintf(def, sizeof def, "%x", type);
    return def;
}

void clientin(void)
{
    unsigned char buf[1024];  /* keep divisible by 4 */
    uint32_t type, id, len;

    areadfully(3, buf, 12);
    type = ntohl(((uint32_t *)buf)[0]);
    id = ntohl(((uint32_t *)buf)[1]);
    len = ntohl(((uint32_t *)buf)[2]);

    printf1("> %s [%d]  %d:", oper(type), id, len);
    writedata(4, buf, 11);
    poll(0,0,waitmsec);
    writedata(4, buf + 11, 1);

    len = (len + 3) & ~3;

    /* duplicate with below... */
    while (len >= sizeof buf) {
	areadfully(3, buf, sizeof buf);
	writehx(0, buf, 20);
	writedata(4, buf, sizeof buf);
	len -= sizeof buf;
	poll(0,0,waitmsec);
    }
    if (len > 0) {
	int l = (len > 16)? 16: len;
	areadfully(3, buf, len);
	writehx(0, buf + len - l, l);
	writedata(4, buf, len - 1);
	poll(0,0,waitmsec);
	write(4, buf + len - 1, 1);
    }
    write(1, "\n", 1);
}

void serverin(void)
{
    unsigned char buf[1024]; /* keep divisible by 4 */
    uint32_t type, id, eno, len;

    areadfully(4, buf, 16);
    type = ntohl(((uint32_t *)buf)[0]);
    id = ntohl(((uint32_t *)buf)[1]);
    eno = ntohl(((uint32_t *)buf)[2]);
    len = ntohl(((uint32_t *)buf)[3]);

    printf1("< %s [%d] (%d)  %d:", oper(type), id, eno, len);
    writedata(3, buf, 15);
    poll(0,0,waitmsec);
    writedata(3, buf + 15, 1);

    len = (len + 3) & ~3;

    /* duplicate with above... */
    while (len >= sizeof buf) {
	areadfully(4, buf, sizeof buf);
	writehx(0, buf, 20);
	writedata(3, buf, sizeof buf);
	len -= sizeof buf;
	poll(0,0,waitmsec);
    }
    if (len > 0) {
	int l = (len > 16)? 16: len;
	areadfully(4, buf, len);
	writehx(0, buf + len - l, l);
	writedata(3, buf, len - 1);
	poll(0,0,waitmsec);
	write(3, buf + len - 1, 1);
    }
    write(1, "\n", 1);
}

void main_loop(void)
{
    struct pollfd pfds[2];
    int cstate = 4, sstate = 4;

    pfds[0].fd = 3; pfds[1].fd = 4;
    pfds[0].events = pfds[1].events = POLLIN;

    while (poll(pfds, 2, -1) > 0 && (cstate || sstate))
    {
	if (pfds[0].revents & POLLIN) cstate = preproto(cstate, '>', 3, 4);
	if (pfds[1].revents & POLLIN) sstate = preproto(sstate, '<', 4, 3);
    }

    if (poll(pfds, 2, -1) < 0
	&& pfds[1].revents & POLLIN) {
	/* client message from server socket. swap fd:s */
	dup2(3, 5);
	dup2(4, 3);
	dup2(5, 4);
	close(5);
    }

    while (poll(pfds, 2, -1) > 0)
    {
	if (pfds[0].revents & POLLIN) clientin();
	if (pfds[1].revents & POLLIN) serverin();
    }
}

int main(int argc, char * argv[])
{
    (void)argc;
    (void)argv;

    if (argv[1] != 0 && atoi(argv[1]) > 0)
	waitmsec = atoi(argv[1]);

    printf("Client '>', Server '<', waitmsecs %d\n", waitmsec);
    printf("Messages always split before wait to test packetizing\n");
    printf("Always consider this tool may also have bugs\n\n");

    while (1) {
	_accept(2000);
	_connect("127.0.0.1", 2001);

	main_loop();
    }
    return 0;
}

/*
 * Local variables:
 * mode: c
 * c-file-style: "stroustrup"
 * tab-width: 8
 * compile-command: "sh travis2utfs.c"
 * End:
 */
