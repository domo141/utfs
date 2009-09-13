/*
 * $Id; utfs-client.c $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *	Copyright (c) 2007 Tomi Ollila
 *	    All rights reserved
 *
 * Created: Sun 26 Aug 2007 08:14:57 AM EEST too
 * Last modified: Sun 13 Sep 2009 21:53:56 EEST too
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "util.h"
#include "pretocol.h"
#include "version.h"

#define UTFS_CLIENT 1
#include "utfs.h"

#if 0 // XXX take text //
void usage(char * progname)
{
    fprintf(stderr, "\nUsage: %s ... mountpoint [fuse options]\n",
	    progname);

    fprintf(stderr, "\n    Options:  FIXME FIXME --\n\n"
	    "\t -c host:port     -- connect to given server port\n"
	    "\t -b [host:]port   -- bind to port for server to connect\n"
#if 0
	    "\t -u  <ifd>:<ofd>  -- use fd:s as i/o connection fds\n"
#endif
	    "\n\t The above options are mutually exclusive.\n\n"
	    "\t -v               -- utfs version information\n"
	);
    fprintf(stderr, "\n    Example: %s too@remote:Mail remoteMail\n\n",
	    progname);

}
#endif

#define _S(x) #x
#define __S(x) _S(x)

static void usage(void)
{
    warn("\nUsage: utfs-client " DETACHOPT "[-l local_secret] [-r remote_secret] \\\n"
	 "             -(b|c|u) [host:]port mountpoint [fuse-options]\n\n"
	"  -l and -r options override UTFS_LOCAL_SECRET and\n"
	"  UTFS_REMOTE_SECRET environment variables.\n"
	"  -c: connect, -b: bind, -u: use (format <ifd>:<ofd>).\n");
    warn("utfs version: " VERSION " protocol " __S(PROTVER) "\n");

    utfs_client_fuse_usage();
}

/* currently lot of duplicate with utfs server implementation */
static void init(int * argcp, char *** argvp)
{
    int doconn = -1;
    bool detach_after_connection = false;
    char * host;

    utfs_initial_opts(argcp, argvp, &usage, &doconn, &host,
			&detach_after_connection);
    if (doconn < 0)
	usage(); /* FIXME */

    if ((*argvp)[1] == null)
	die("Mount point missing");

    d0(("argv[0] %s\n", *argvp[0]));
    utfs_client_handle_args(argcp, argvp);

    if (doconn < 2)
	make_connection(host, doconn, detach_after_connection);
    else
	use_fds(host, detach_after_connection);
    warn("utfs client is ready"); // XXX move deeper
}

int main(int argc, char ** argv)
{
    init_G(argv[0]);
    init(&argc, &argv);
    utfs_client_fuse_main(argc, argv);
    exit(0);
}

/*
 * Local variables:
 * mode: c
 * c-file-style: "stroustrup"
 * tab-width: 8
 * End:
 */
