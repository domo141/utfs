/*
 * $Id; utfs.c $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *	Copyright (c) 2009 Tomi Ollila
 *	    All rights reserved
 *
 * Created: Thu 07 May 2009 09:10:23 EEST too
 * Last modified: Sun 13 Sep 2009 17:04:53 EEST too
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#if ! WIN32
#include <unistd.h>
#include <fcntl.h>
#endif

#include "defs.h"
#include "pretocol.h"
#define UTFS_COMMON 1
#include "utfs.h"

#include "version.h" /* for PROTVER */

#define d2(x) do { warn x; } while (0)

const char * prgname0;
const char G_pad000[3] = { 0, 0, 0 };
struct _G G;
void init_G(const char * prgname)
{
    memset(&G, 0, sizeof G);
    G.prgname = prgname;
    if ( (prgname0 = strchr(prgname, '/')) != null)
	prgname0++;
    else if ( (prgname0 = strchr(prgname, '\\')) != null)
	prgname0++;
    else
	prgname0 = prgname;
    G_fd_in = -1;
#if !WIN32
    G_fd_out = -1;
#endif

    // XXX
    setvbuf(stderr, null, _IOLBF, 0); /* setlinebuf(stderr); */
}

static char * get_next_arg(int * c, char *** v, void (*usage)(void))
{
    if ((*v)[1] == null)
	usage();

    (*v)++; (*c)--;

    d0(("nxt arg: %s, left %d", **v, *c));

    return **v;
}

/* XXX add protocol version (UTFS_PROTOCOL_VERSION or command line) */
/* XXX protver used in hash now -- needed elsewhere */

void utfs_initial_opts(int * argcp, char *** argvp, void (*usage)(void),
		       int * doconnp, char ** strp, bool * detachp)
{
    /* store progname so it can be returned to the beginning
       of remaining argument line */
    char * argv0 = *((*argvp)++);

    while ((*argvp)[0] && (*argvp)[0][0] == '-'
	   && (*argvp)[0][1] != '\0'
	   && ((*argvp)[0][1] == '-' || (*argvp)[0][2] == '\0'))
    {
	char * opt;

	d0(("arg: %s", *argvp[0]));

	switch ((*argvp)[0][1])
	{
	case 'v':
	    G.verbose = 1;
	    break;
	case 'b':
	case 'u':
	    *doconnp = 2;
	    *strp = get_next_arg(argcp, argvp, usage);
	    break;

	case 'c':
	    *doconnp = 1;
	    *strp = get_next_arg(argcp, argvp, usage);
	    break;

	case 'l':
	    opt = get_next_arg(argcp, argvp, usage);
	    setenv("UTFS_LOCAL_SECRET", opt, 1);
	    break;

	case 'r':
	    opt = get_next_arg(argcp, argvp, usage);
	    setenv("UTFS_REMOTE_SECRET", opt, 1);
	    break;

	case 'm':
	    opt = get_next_arg(argcp, argvp, usage);
	    setenv("UTFS_MOUNTDIR", opt, 1);
	    break;
	case '-':
#if ! WIN32
	    if (strcmp((*argvp)[0], "--detach") == 0) {
		*detachp = true;
		break;
	    }
#endif /* ! WIN32 */
	    /* fall through */
	default:
	    usage();
	}
	(*argcp)--; (*argvp)++;
    }
    *(--(*argvp)) = argv0;
}

#if ! WIN32
void detach(void)
{
    int fd = open("/dev/null", O_RDWR);

    if (fd < 0)
	die("Opening '/dev/null' failed:");

    switch (fork())
    {
    case -1: die("fork failed:");
    case 0:  break; /* child */
    default: exit(0); /* parent */
    }

#if DEBUG && 0
    fd2 = 2;
#endif

    /* child continues */
    dup2(fd, 0); dup2(fd, 1); dup2(/*fd2*/fd, 2); setsid();
    if (fd > 2) close(fd);
    if (G.verbose)
	G.verbose = 0;
}
#endif /* ! WIN32  */

static unsigned long sdbm_hash(const unsigned char * str)
{
    unsigned long hash = PROTVER;
    int c;

    while ((c = *str++) != '\0')
	hash = c + (hash << 6) + (hash << 16) - hash;

    return hash;
}

/* pretocol does not know env vars... */
const unsigned char * get_secrets(unsigned char * bufspace)
{
    const char * lsec = getenv("UTFS_LOCAL_SECRET");
    const char * rsec = getenv("UTFS_REMOTE_SECRET");
    unsigned long hash;

    if (lsec == null) die("local \"secret\" missing\n");
    if (rsec == null) die("remote \"secret\" missing\n");

    hash = sdbm_hash(S2U(lsec, const char, *, *));
    xverbose(("%C: local hash: %08x", hash));
    bufspace[0] = hash >> 24;
    bufspace[1] = hash >> 16;
    bufspace[2] = hash >> 8;
    bufspace[3] = hash >> 0;
    hash = sdbm_hash(S2U(rsec, const char, *, *));
    xverbose(("%C: remote hash: %08x", hash));
    bufspace[4] = hash >> 24;
    bufspace[5] = hash >> 16;
    bufspace[6] = hash >> 8;
    bufspace[7] = hash >> 0;

    unsetenv("UTFS_LOCAL_SECRET");
    unsetenv("UTFS_REMOTE_SECRET");

    return bufspace;
}


int getport(const char * s)
{
    int i;
    i = atoi(s);
    if (i > 0 && i < 65536)
    {
	const char * p = s;
	do {
	    if (*p < '0' || *p > '9')
		break;
	} while (*++p);
	if (*p == '\0')
	    return i;
    }
    die("Illegal port %s", s);
}

void make_connection(char * host, int doconn, bool dtach)
{
    char * p;
    sockfd fd;
    int port;
    unsigned char buf[256]; /* XXX */

    /*gethostport! ??*/
    p = strchr(host, ':'); /* XXX not ipv6_safe */
    if (p)
	*p++ = '\0';
    else {
	p = host;
	host = null;
    }
    port = getport(p);

    if (doconn) {
	if (host == null)
	    die ("Host null\n");
	fd = doconnect(get_secrets(buf), host, port);
    }
    else
	/* XXX host currently ignored below */
	fd = dobindandlisten(host, port, true);

#if ! WIN32
    if (dtach)
	detach();
#endif
    if (! doconn)
	fd = doaccept(get_secrets(buf), fd);

    G_fd_in = fd;
#if !WIN32
    G_fd_out = fd;
#endif
}

void use_fds(const char * host, bool dtach)
{
    unsigned char buf[256]; /* XXX */
    int fd1, fd2;
    const char * p;

    fd1 = strtol(host, &p, 10);
    if (p[0] != ':')
	die("fd arg '%s' not in format 'n:n'\n", host);
    p++;
    fd2 = strtol(p, &p, 10);
    if (p[0] != '\0')
	die("fd arg '%s' not in format 'n:n'\n", host);

#if WIN32
    die("fd usage not supported in windows\n");
#else
    doweaksecretexchange(get_secrets(buf), fd1, fd2);
#endif
    G_fd_in = fd1;
#if !WIN32
    G_fd_out = fd2;
#endif
#if ! WIN32
    if (dtach)
	detach();
#endif

}

#if 000
void gethostport(const char * s, char * host, int * port)
{
    char * p;
#if 0
    if (port != 0)
	die("Port %d not 0, reused / mutually exclusive argument ?", port);
#endif
    p = strrchr(s, ':');
    if (p) {
	/* XXX bounds check */
	memcpy(host, s, p - s);
	host[p - s] = '\0';
	*port = getport(p + 1);
    }
    else {
	*port = getport(s);
	host[0] = '\0';
    }
}
#endif

#if W32_HAXES
#define lstat stat /* no symlinks in windows (that we care of) */
#endif

bool isdir(const char * path)
{
    struct stat st;
    if (lstat(path, &st) < 0) /* stat -> lstat: symlink does not do it ! */
	return false;

    return S_ISDIR(st.st_mode);
}

/*
 * Local variables:
 * mode: c
 * c-file-style: "stroustrup"
 * tab-width: 8
 * End:
 * vi: set sw=8 ts=8:
 */
