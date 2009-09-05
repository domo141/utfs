/*
 * $Id; utfs.h $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *	Copyright (c) 2007 Tomi Ollila
 *	    All rights reserved
 *
 * Created: Tue Aug 28 21:49:35 EEST 2007 too
 * Last modified: Fri 04 Sep 2009 19:56:30 EEST too
 */

#ifndef UTFS_H
#define UTFS_H

#if !UTFS_CLIENT && !UTFS_SERVER && !UTFS_COMMON
#error
#endif

#ifndef UTIL_H
#include "util.h"
#endif
#ifndef SOCKFD_H
#include "sockfd.h"
#endif

#if WIN32
#define DETACHOPT
#else
#define DETACHOPT "[--detach] "
#endif

struct _G {
    const char * prgname;
#if WIN32
    sockfd _sd;
#else
    int _fd_in;
    int _fd_out;
#endif
    int verbose;
};

#if WIN32
#define G_fd_in G._sd
#define G_fd_out G._sd
#else
#define G_fd_in G._fd_in
#define G_fd_out G._fd_out
#endif

#define xverbose(x) \
    do { if (G.verbose) { warn x; }} while (0)

/* in utfs.c */

extern struct _G G;
void init_G(const char * prgname);

void utfs_initial_opts(int * argcp, char *** argvp, void (*usage)(void),
		       int * doconnp, char ** hostp, bool * detachp);
const unsigned char * get_secrets(unsigned char * bufspace);

void make_connection(char * host, int doconn, bool dtach);
void use_fds(const char * host, bool dtach);

int getport(const char * s);
void gethostport(const char * s, char * host, int * port);
bool isdir(const char * path);


#if UTFS_CLIENT


/* in utfs-client.c */


/* in utfs-client-fuse.c */

void utfs_client_fuse_usage(void) GCCATTR_NORETURN;
void utfs_client_handle_args(int * argcp, char *** argvp);
void utfs_client_fuse_main(int argc, char ** argv) GCCATTR_NORETURN;

#elif UTFS_SERVER

void utfs_server(void) GCCATTR_NORETURN;

#endif

#if UTFS_PROTOCOL

enum {
    UTFS__OFFSET = 0x7a5120ff,
    UTFS__GETATTR,
    UTFS__READLINK,
    UTFS__GETDIR,
    UTFS__MKNOD,
    UTFS__MKDIR,
    UTFS__UNLINK,
    UTFS__RMDIR,
    UTFS__SYMLINK,
    UTFS__RENAME,
    UTFS__LINK,
    UTFS__CHMOD,
    UTFS__CHOWN,
    UTFS__TRUNCATE,
    UTFS__UTIME,
    UTFS__OPEN,
    UTFS__READ,
    UTFS__WRITE,
    UTFS__STATFS,
    UTFS__FLUSH,
    UTFS__RELEASE,
    UTFS__FSYNC,
    UTFS__SETXATTR,
    UTFS__GETXATTR,
    UTFS__LISTXATTR,
    UTFS__REMOVEXATTR,
    /* 2.3 */
    UTFS__OPENDIR,
    UTFS__READDIR,
    UTFS__RELEASEDIR,
    UTFS__FSYNCDIR,
    UTFS__INIT,
    UTFS__DESTROY,
    /* 2.5 */
    UTFS__ACCESS,
    UTFS__CREATE,
    UTFS__FTRUNCATE,
    UTFS__FGETATTR,
    /* 2.6 */
    UTFS__LOCK,
    UTFS__UTIMENS,
    UTFS__BMAP
};

#endif

#endif /* UTFS_H */

/*
 * Local variables:
 * mode: c
 * c-file-style: "stroustrup"
 * tab-width: 8
 * End:
 */
