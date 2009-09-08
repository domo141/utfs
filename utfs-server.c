/*
 * $Id; utfs-server.c $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *	Copyright (c) 2007 tomi Ollila
 *	    All rights reserved
 *
 * Created: Sun 26 Aug 2007 08:18:39 AM EEST too
 * Last modified: Tue 08 Sep 2009 22:45:07 EEST too
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include "sockfd.h"

#include "util.h"
#include "pretocol.h"

#define UTFS_SERVER 1
#include "utfs.h"

#include "version.h"

#define _S(x) #x
#define __S(x) _S(x)

static void usage(void)
{
    warn("\nUsage: utfs-server " DETACHOPT "[-l local_secret] [-r remote_secret] \\\n"
	 "             -(b|c|u) [host:]port mountdir\n\n"
	 "  -l and -r options override UTFS_LOCAL_SECRET and\n"
	 "  UTFS_REMOTE_SECRET environment variables.\n"
	 "  -c: connect, -b: bind, -u: use (format <ifd>:<ofd>).\n");
    die("utfs version: " VERSION " protocol " __S(PROTVER) "\n");

}


/* currently lot of duplicate with utfs client implementation */
static void init(int * argcp, char *** argvp)
{
    int doconn = -1;
    bool detach_after_connection = false;
    char * host, *mountdir;

    utfs_initial_opts(argcp, argvp, &usage, &doconn, &host,
		      &detach_after_connection);
    if (doconn < 0)
	usage(); /* FIXME */

    mountdir = (*argvp)[1];

    if (mountdir == null)
	die("Mount directory missing");

    if (chdir(mountdir) < 0)
	die("chdir('%s')\n", mountdir);

    if (doconn < 2)
	make_connection(host, doconn, detach_after_connection);
    else
	use_fds(host, detach_after_connection);
}

#if WIN32
void w32_init(void)
{
    static WSADATA wsaData;
    int iResult;

    /* Initialize Winsock */
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0)
	die ("WSAStartup failed: %d\n", iResult);
}
#endif


int main(int argc, char ** argv)
{
    init_G(argv[0]);
#if WIN32
    void w32_init(void);
    w32_init();
#endif
    init(&argc, &argv);
    utfs_server();
    return 0;
}

/*
 * Local variables:
 * mode: c
 * c-file-style: "stroustrup"
 * tab-width: 8
 * End:
 */
