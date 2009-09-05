/*
 * $Id; utfs-client-fuse.c $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *	Copyright (c) 2007 Tomi Ollila
 *	    All rights reserved
 *
 * Created: Tue Aug 28 21:47:30 EEST 2007 too
 * Last modified: Fri 04 Sep 2009 19:45:06 EEST too
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include <errno.h>

#define UTFS_CLIENT 1
#define UTFS_PROTOCOL 1
#include "utfs.h"
#include "utfs-errmaps.h"

#include "utfs-buffer.h"

#define SIMPLEXDR_INLINE
#include "simplexdr.h"

#define AMIGA_LIST_INLINE
#include "amiga_list.h"

#include "version.h"

#ifndef null
#define null ((void *)0)
#endif

/* #define  utfs_getattr  null */
/* #define  utfs_readlink  null */
#define  utfs_getdir  null
/* #define  utfs_mknod  null */
/* #define  utfs_mkdir  null */
/* #define  utfs_unlink  null */
/* #define  utfs_rmdir  null */
/* #define  utfs_symlink  null */
/* #define  utfs_rename  null */
/* #define  utfs_link  null */
/* #define  utfs_chmod  null */
/* #define  utfs_chown  null */
/* #define  utfs_truncate  null */
/* #define  utfs_utime  null */
/* #define  utfs_open  null */
/* #define  utfs_read  null */
/* #define  utfs_write  null */
/* #define  utfs_statfs  null */
#define  utfs_flush  null
/* #define  utfs_release  null */
#define  utfs_fsync  null
#define  utfs_setxattr  null
#define  utfs_getxattr  null
#define  utfs_listxattr  null
#define  utfs_removexattr  null
#define  utfs_opendir  null
/* #define  utfs_readdir  null */
#define  utfs_releasedir  null
#define  utfs_fsyncdir  null
/*#define  utfs_init  null */
#define  utfs_destroy  null
#define  utfs_access  null
#define  utfs_create  null
#define  utfs_ftruncate  null
#define  utfs_fgetattr  null
#define  utfs_lock  null
#define  utfs_utimens  null
#define  utfs_bmap  null


#define CONCURRENT_REQUESTS 64
struct {
    pthread_mutex_t entry_mutex;
    pthread_cond_t entry_cond;
    pthread_mutex_t write_mutex;
    int pending_requests;
    AList freerequests;
    struct RequestNode {
	AListNode listnode;
	uint32_t type;
	uint32_t refno;
	pthread_mutex_t node_mutex;
	pthread_cond_t node_cond;
	uint8_t * (*cbfunc)(void * cbdata, uint32_t rlen, uint8_t * p);
	void * cbdata;
	int32_t errno_;
    } requests[CONCURRENT_REQUESTS];

} C;


static uint8_t *
utfs_invalid(void * cbdata, uint32_t rlen, uint8_t * p)
{
    (void)cbdata;

    d1(("--- %s(%p, %d, %p)", __func__, cbdata, rlen, p));

    return utfs_buffer_discard(p, (rlen + 3) & ~3);
}

/* this is called before fuse_main() */
static void utfs_init_C(void)
{
    int i;
    d1(("%s(): pid %d", __func__, getpid()));

    memset(&C, 0, sizeof C);
    pthread_mutex_init(&C.entry_mutex, NULL);
    pthread_mutex_init(&C.write_mutex, NULL);
    pthread_cond_init(&C.entry_cond, NULL);

    amiga_list_init(&C.freerequests);
    for (i = 0; i < CONCURRENT_REQUESTS; i++) {
	pthread_mutex_init(&C.requests[i].node_mutex, NULL);
	pthread_cond_init(&C.requests[i].node_cond, NULL);
	C.requests[i].refno = i;
	C.requests[i].cbfunc = utfs_invalid;
	C.requests[i].cbdata = null;
	amiga_list_addtail(&C.freerequests, &C.requests[i].listnode);
    }
}

/* inline functions are part of C99 standard. */

static inline char* str_unconst(const char* p) { return p; } /* like GNU mc */

static /* inline */ int
utfs_client_request_handle(int type, struct iovec* iov, int iovlen, int clen,
			   uint8_t * (*cbfunc)(void *, uint32_t, uint8_t *),
			   void * cbdata)
{
    struct RequestNode * rn;
    int error, wl;
    uint8_t * p = iov[0].iov_base;

    pthread_mutex_lock(&C.entry_mutex);
    if (C.pending_requests >= CONCURRENT_REQUESTS) {
	do
	    pthread_cond_wait(&C.entry_cond, &C.entry_mutex);
	while (C.pending_requests >= CONCURRENT_REQUESTS);
    }
    C.pending_requests++;

    rn = (struct RequestNode *)amiga_list_remheadq(&C.freerequests);

    pthread_mutex_unlock(&C.entry_mutex);

    pthread_mutex_lock(&rn->node_mutex);
    rn->type = type;
    p = xencode_uint32(p, type);
    /* index = rn - C.requests; */
    p = xencode_uint32(p, rn->refno);
    p = xencode_uint32(p, clen);
#define REQUEST_HEADER_LEN 12

    rn->cbfunc = cbfunc;
    rn->cbdata = cbdata;

    pthread_mutex_lock(&C.write_mutex);
    wl = writev(G_fd_out, iov, iovlen);
    pthread_mutex_unlock(&C.write_mutex);

    d1(("(%d = writev(%d, %p, %d)) != %d (%s)",
	wl, G_fd_out, iov, iovlen, clen + REQUEST_HEADER_LEN, strerror(errno)));

    /* w/ this cannot use non-4-divisible clen */
    if (wl == clen + REQUEST_HEADER_LEN) {
	rn->errno_ = -1;
	do
	    pthread_cond_wait(&rn->node_cond, &rn->node_mutex);
	while (rn->errno_ < 0);

	error = rn->errno_;
    }
    else
	error = EACCES;

    rn->refno += 0x100; /* XXX define, match with 0xFF.. */

    pthread_mutex_unlock(&rn->node_mutex);

    pthread_mutex_lock(&C.entry_mutex);

    amiga_list_addtail(&C.freerequests, &rn->listnode);

    if (C.pending_requests >= CONCURRENT_REQUESTS)
	pthread_cond_signal(&C.entry_cond);

    C.pending_requests--;

    pthread_mutex_unlock(&C.entry_mutex);

    return -error;
}


static void * utfs_client_reader_thread(void * arg_unused)
{
    uint8_t * p = utfs_buffer_init(G_fd_in);

    (void)arg_unused;

    while (1)
    {
	uint32_t type, index_, refno, errno_, rlen;
	struct RequestNode * rn;

	p = utfs_buffer_reserve(p, 16);
	if (p == null)
	    break; /* XXX */

	type = xdecode_uint32(&p);
	refno = xdecode_uint32(&p);
	errno_ = xdecode_uint32(&p);
	rlen = xdecode_uint32(&p);

	d1(("response read: type %x refno %x errno %d rlen %d",
	    type, refno, errno_, rlen));

	index_ = refno & 0xFF; /* XXX define, match with 0x100 */
	if (index_ >= CONCURRENT_REQUESTS)
	    break; /* XXX */

	rn = &C.requests[index_];

	pthread_mutex_lock(&rn->node_mutex);
	if (refno != rn->refno) {
	    /* interrupted */
	    d1(("interrupted -- or an error as interrupts do not arrive yet"));
	    pthread_mutex_unlock(&rn->node_mutex);
	    if ((p = utfs_buffer_discard(p, (rlen + 3) & ~3)) == null)
		break; /* XXX */
	    continue;
	}

	if (type != rn->type)
	    break; /* XXX sanity check. */

	if (errno_ == 0) /* note: rlen == 0 if errno != 0 */
	    if ((p = rn->cbfunc(rn->cbdata, rlen, p)) == null)
		break; /* XXX */

	if (errno_ >= utfs_client_errmap_size)
	    errno_ = EDEFAULT;
	rn->errno_ =  utfs_client_errmap[errno_];
	d1(("signal rn #%d", index_));
	pthread_cond_signal(&rn->node_cond);
	rn->type = 0;
	rn->cbfunc = utfs_invalid;
	pthread_mutex_unlock(&rn->node_mutex);
    }

    d1(("exiting reader thread"));
    /* harakiri (sshfs terms :) */
    kill(getpid(), SIGTERM); /* XXX oujes :/ */
    return null;
}

/*#undef utfs_init */
static void * utfs_init(struct fuse_conn_info * fci)
{
    pthread_t thread_id;

    /* XXX */
    int maxmsgsize = C.pending_requests;
    C.pending_requests = 0;

    d1(("--- utfs_init(%p) mms %d", fci, maxmsgsize));

    /* XXX see signal handling in sshfs (for reference, at least) */
    if (pthread_create(&thread_id, NULL, utfs_client_reader_thread, NULL))
	/* XXX error message */
	exit(1);
    pthread_detach(thread_id);

    fci->async_read = false;
    fci->max_write = maxmsgsize;
    fci->max_readahead = maxmsgsize;

    return null;
}


static uint8_t * utfs_getattr_cb(void * cbdata, uint32_t rlen, uint8_t * p)
{
    struct stat * st = (struct stat *)cbdata;
    (void)rlen;

    d1(("--- %s(%p, %d, %p)", __func__, cbdata, rlen, p));

    if ((p = utfs_buffer_reserve(p, 8+8+4+4+4+4+8+8+4+8+4+4+4)) == null)
	return null;

    st->st_dev     = xdecode_uint64(&p);
    st->st_ino     = xdecode_uint64(&p);
    st->st_mode    = xdecode_uint32(&p);
    st->st_nlink   = xdecode_uint32(&p);
    st->st_uid     = xdecode_uint32(&p);
    st->st_gid     = xdecode_uint32(&p);
    st->st_rdev    = xdecode_uint64(&p);
    st->st_size    = xdecode_uint64(&p);
    st->st_blksize = xdecode_uint32(&p);
    st->st_blocks  = xdecode_uint64(&p);
    st->st_atime   = xdecode_uint32(&p);
    st->st_mtime   = xdecode_uint32(&p);
    st->st_ctime   = xdecode_uint32(&p);

    return p;
}

static int utfs_getattr(const char * path, struct stat * statbuf)
{
    uint8_t buf[32];
    struct iovec iov[3];
    int iovlen;
    int plen = strlen(path) + 1;
    int alen = (plen + 3) & ~3;

    d1(("--- %s(\"%s\", %p) %d %d", __func__, path, statbuf, plen, alen));

    iov[0].iov_base = buf; /* utfs_client_request_handle() will fill this */
    iov[0].iov_len = REQUEST_HEADER_LEN;
    iov[1].iov_base = str_unconst(path); /* const void * when writing... */
    iov[1].iov_len = plen;

    if (plen != alen) {
	iov[2].iov_base = str_unconst(G_pad000); /*const void * when writing */
	iov[2].iov_len = alen - plen;
	iovlen = 3;
    }
    else iovlen = 2;

    return utfs_client_request_handle(UTFS__GETATTR, iov, iovlen, alen,
				      utfs_getattr_cb, (void*)statbuf);
}


static uint8_t * utfs_readlink_cb(void * cbdata, uint32_t rlen, uint8_t * p)
{
    char * lbuf = ((struct iovec *)cbdata)->iov_base;
    size_t size = ((struct iovec *)cbdata)->iov_len;
    size_t min;
    int alen = (rlen + 3) & ~3;

    d1(("--- %s(%p, %d, %p)", __func__, cbdata, rlen, p));

    if ((p = utfs_buffer_reserve(p, alen)) == null)
	return null;

    /* actually, it is an error if rlen > size, protecting for such */
    min = size < rlen? size: rlen;

    ((struct iovec *)cbdata)->iov_len = min;

    /* scatter ACN */
    memcpy(lbuf, p, min); /* server NUL-terminates */
    return p + alen;
}

static int utfs_readlink(const char * path, char * lbuf, size_t size)
{
    uint8_t buf[32];
    struct iovec iov[3];
    int iovlen;
    int plen = strlen(path) + 1;
    int alen = (plen + 3) & ~3;
    struct iovec lbufdata;
    int error;

    d1(("--- %s(\"%s\", %p, %d)", __func__, path, lbuf, size));

    /* XXX use sizeof BUFFER.b - xxx */
    /* using scatter read will make this limit server-bound, later... */
    if (size > 4100)
	size = 4100;

    (void)xencode_uint32(buf + REQUEST_HEADER_LEN, size);

    iov[0].iov_base = buf; /* utfs_client_request_handle() will fill this */
    iov[0].iov_len = REQUEST_HEADER_LEN + 4;
    iov[1].iov_base = str_unconst(path); /* const void * when writing... */
    iov[1].iov_len = plen;

    if (plen != alen) {
	iov[2].iov_base = str_unconst(G_pad000); /*const void * when writing */
	iov[2].iov_len = alen - plen;
	iovlen = 3;
    }
    else iovlen = 2;

    alen += 4;

    lbufdata.iov_base = lbuf;
    lbufdata.iov_len = size;

    error = utfs_client_request_handle(UTFS__READLINK, iov, iovlen, alen,
				       utfs_readlink_cb, (void*)&lbufdata);
    if (error < 0)
	return error;
    if (lbufdata.iov_len > 4097)
	return ENAMETOOLONG;
    d1(("readlink rv: %d", lbufdata.iov_len));
#if 0
    return lbufdata.iov_len;
#else
    return 0;
#endif
}


static uint8_t * utfs_ok_cb(void * cbdata, uint32_t rlen, uint8_t * p)
{
    (void)cbdata;
    (void)rlen;

    d1(("--- %s(%p, %d, %p)", __func__, cbdata, rlen, p));

    return p;
}


static int utfs_mknod(const char * path, mode_t mode, dev_t dev)
{
    uint8_t buf[32], *p;
    struct iovec iov[3];
    int iovlen;
    int plen = strlen(path) + 1;
    int alen = (plen + 3) & ~3;

    d1(("--- %s(\"%s\", %o, %lld)", __func__, path, mode, dev));

    p = xencode_uint32(buf + REQUEST_HEADER_LEN, mode);
    (void)xencode_uint64(p, dev);

    iov[0].iov_base = buf; /* utfs_client_request_handle() will fill this */
    iov[0].iov_len = REQUEST_HEADER_LEN + 4 + 8;
    iov[1].iov_base = str_unconst(path); /* const void * when writing... */
    iov[1].iov_len = plen;

    if (plen != alen) {
	iov[2].iov_base = str_unconst(G_pad000); /*const void * when writing */
	iov[2].iov_len = alen - plen;
	iovlen = 3;
    }
    else iovlen = 2;

    alen += 4 + 8;

    return utfs_client_request_handle(UTFS__MKNOD, iov, iovlen, alen,
				      utfs_ok_cb, null);
}


static int utfs_mkdir(const char * path, mode_t mode)
{
    uint8_t buf[32];
    struct iovec iov[3];
    int iovlen;
    int plen = strlen(path) + 1;
    int alen = (plen + 3) & ~3;

    d1(("--- %s(\"%s\", %o)", __func__, path, mode));

    (void)xencode_uint32(buf + REQUEST_HEADER_LEN, mode);

    iov[0].iov_base = buf; /* utfs_client_request_handle() will fill this */
    iov[0].iov_len = REQUEST_HEADER_LEN + 4;
    iov[1].iov_base = str_unconst(path); /* const void * when writing... */
    iov[1].iov_len = plen;

    if (plen != alen) {
	iov[2].iov_base = str_unconst(G_pad000); /*const void * when writing */
	iov[2].iov_len = alen - plen;
	iovlen = 3;
    }
    else iovlen = 2;

    alen += 4;

    return utfs_client_request_handle(UTFS__MKDIR, iov, iovlen, alen,
				      utfs_ok_cb, null);
}


static int utfs_unlink(const char * path)
{
    uint8_t buf[32];
    struct iovec iov[3];
    int iovlen;
    int plen = strlen(path) + 1;
    int alen = (plen + 3) & ~3;

    d1(("--- %s(\"%s\")", __func__, path));

    iov[0].iov_base = buf; /* utfs_client_request_handle() will fill this */
    iov[0].iov_len = REQUEST_HEADER_LEN;
    iov[1].iov_base = str_unconst(path); /* const void * when writing... */
    iov[1].iov_len = plen;

    if (plen != alen) {
	iov[2].iov_base = str_unconst(G_pad000); /*const void * when writing */
	iov[2].iov_len = alen - plen;
	iovlen = 3;
    }
    else iovlen = 2;

    return utfs_client_request_handle(UTFS__UNLINK, iov, iovlen, alen,
				      utfs_ok_cb, null);
}

/* exactly the same as above. refactor! */
static int utfs_rmdir(const char * path)
{
    uint8_t buf[32];
    struct iovec iov[3];
    int iovlen;
    int plen = strlen(path) + 1;
    int alen = (plen + 3) & ~3;

    d1(("--- %s(\"%s\")", __func__, path));

    iov[0].iov_base = buf; /* utfs_client_request_handle() will fill this */
    iov[0].iov_len = REQUEST_HEADER_LEN;
    iov[1].iov_base = str_unconst(path); /* const void * when writing... */
    iov[1].iov_len = plen;

    if (plen != alen) {
	iov[2].iov_base = str_unconst(G_pad000); /*const void * when writing */
	iov[2].iov_len = alen - plen;
	iovlen = 3;
    }
    else iovlen = 2;

    return utfs_client_request_handle(UTFS__RMDIR, iov, iovlen, alen,
				      utfs_ok_cb, null);
}


static int utfs_symlink(const char * from, const char * to)
{
    uint8_t buf[32];
    struct iovec iov[4];
    int iovlen;
    int flen = strlen(from) + 1;
    int tlen = strlen(to) + 1;
    int plen = flen + tlen;
    int alen = (plen + 3) & ~3;

    d1(("--- %s(\"%s\", \"%s\")", __func__, from, to));

    (void)xencode_uint32(buf + REQUEST_HEADER_LEN, flen);

    iov[0].iov_base = buf; /* utfs_client_request_handle() will fill this */
    iov[0].iov_len = REQUEST_HEADER_LEN + 4;
    iov[1].iov_base = str_unconst(from); /* const void * when writing... */
    iov[1].iov_len = flen;
    iov[2].iov_base = str_unconst(to); /* const void * when writing... */
    iov[2].iov_len = tlen;

    if (plen != alen) {
	iov[3].iov_base = str_unconst(G_pad000); /*const void * when writing */
	iov[3].iov_len = alen - plen;
	iovlen = 4;
    }
    else iovlen = 3;

    alen += 4;

    return utfs_client_request_handle(UTFS__SYMLINK, iov, iovlen, alen,
				      utfs_ok_cb, null);
}

/* exactly the same as above. refactor! */
static int utfs_rename(const char * from, const char * to)
{
    uint8_t buf[32];
    struct iovec iov[4];
    int iovlen;
    int flen = strlen(from) + 1;
    int tlen = strlen(to) + 1;
    int plen = flen + tlen;
    int alen = (plen + 3) & ~3;

    d1(("--- %s(\"%s\", \"%s\")", __func__, from, to));

    (void)xencode_uint32(buf + REQUEST_HEADER_LEN, flen);

    iov[0].iov_base = buf; /* utfs_client_request_handle() will fill this */
    iov[0].iov_len = REQUEST_HEADER_LEN + 4;
    iov[1].iov_base = str_unconst(from); /* const void * when writing... */
    iov[1].iov_len = flen;
    iov[2].iov_base = str_unconst(to); /* const void * when writing... */
    iov[2].iov_len = tlen;

    if (plen != alen) {
	iov[3].iov_base = str_unconst(G_pad000); /*const void * when writing */
	iov[3].iov_len = alen - plen;
	iovlen = 4;
    }
    else iovlen = 3;

    alen += 4;

    return utfs_client_request_handle(UTFS__RENAME, iov, iovlen, alen,
				      utfs_ok_cb, null);
}

/* exactly the same as above. refactor! */
static int utfs_link(const char * from, const char * to)
{
    uint8_t buf[32];
    struct iovec iov[4];
    int iovlen;
    int flen = strlen(from) + 1;
    int tlen = strlen(to) + 1;
    int plen = flen + tlen;
    int alen = (plen + 3) & ~3;

    d1(("--- %s(\"%s\", \"%s\")", __func__, from, to));

    (void)xencode_uint32(buf + REQUEST_HEADER_LEN, flen);

    iov[0].iov_base = buf; /* utfs_client_request_handle() will fill this */
    iov[0].iov_len = REQUEST_HEADER_LEN + 4;
    iov[1].iov_base = str_unconst(from); /* const void * when writing... */
    iov[1].iov_len = flen;
    iov[2].iov_base = str_unconst(to); /* const void * when writing... */
    iov[2].iov_len = tlen;

    if (plen != alen) {
	iov[3].iov_base = str_unconst(G_pad000); /*const void * when writing */
	iov[3].iov_len = alen - plen;
	iovlen = 4;
    }
    else iovlen = 3;

    alen += 4;

    return utfs_client_request_handle(UTFS__LINK, iov, iovlen, alen,
				      utfs_ok_cb, null);
}


/* like mkdir. refactor */
static int utfs_chmod(const char * path, mode_t mode)
{
    uint8_t buf[32];
    struct iovec iov[3];
    int iovlen;
    int plen = strlen(path) + 1;
    int alen = (plen + 3) & ~3;

    d1(("--- %s(\"%s\", %o)", __func__, path, mode));

    (void)xencode_uint32(buf + REQUEST_HEADER_LEN, mode);

    iov[0].iov_base = buf; /* utfs_client_request_handle() will fill this */
    iov[0].iov_len = REQUEST_HEADER_LEN + 4;
    iov[1].iov_base = str_unconst(path); /* const void * when writing... */
    iov[1].iov_len = plen;

    if (plen != alen) {
	iov[2].iov_base = str_unconst(G_pad000); /*const void * when writing */
	iov[2].iov_len = alen - plen;
	iovlen = 3;
    }
    else iovlen = 2;

    alen += 4;

    return utfs_client_request_handle(UTFS__CHMOD, iov, iovlen, alen,
				      utfs_ok_cb, null);
}


static int utfs_chown(const char * path, uid_t uid, gid_t gid)
{
    uint8_t buf[32], *p;
    struct iovec iov[3];
    int iovlen;
    int plen = strlen(path) + 1;
    int alen = (plen + 3) & ~3;

    d1(("--- %s(\"%s\", %d, %d)", __func__, path, uid, gid));

    p = xencode_uint32(buf + REQUEST_HEADER_LEN, uid);
    (void) xencode_uint32(p, gid);

    iov[0].iov_base = buf; /* utfs_client_request_handle() will fill this */
    iov[0].iov_len = REQUEST_HEADER_LEN + 8;
    iov[1].iov_base = str_unconst(path); /* const void * when writing... */
    iov[1].iov_len = plen;

    if (plen != alen) {
	iov[2].iov_base = str_unconst(G_pad000); /*const void * when writing */
	iov[2].iov_len = alen - plen;
	iovlen = 3;
    }
    else iovlen = 2;

    alen += 8;

    return utfs_client_request_handle(UTFS__CHOWN, iov, iovlen, alen,
				      utfs_ok_cb, null);
}

static int utfs_truncate(const char * path, off_t size)
{
    uint8_t buf[32];
    struct iovec iov[3];
    int iovlen;
    int plen = strlen(path) + 1;
    int alen = (plen + 3) & ~3;

    d1(("--- %s(\"%s\", %lld)", __func__, path, size));

    (void)xencode_uint64(buf + REQUEST_HEADER_LEN, size);

    iov[0].iov_base = buf; /* utfs_client_request_handle() will fill this */
    iov[0].iov_len = REQUEST_HEADER_LEN + 8;
    iov[1].iov_base = str_unconst(path); /* const void * when writing... */
    iov[1].iov_len = plen;

    if (plen != alen) {
	iov[2].iov_base = str_unconst(G_pad000); /*const void * when writing */
	iov[2].iov_len = alen - plen;
	iovlen = 3;
    }
    else iovlen = 2;

    alen += 8;

    return utfs_client_request_handle(UTFS__TRUNCATE, iov, iovlen, alen,
				      utfs_ok_cb, null);
}


/* XXX deprecated, just noticed (2008-01-05 16:05) */
static int utfs_utime(const char * path, struct utimbuf * utbuf)
{
    uint8_t buf[32], *p;
    struct iovec iov[3];
    int iovlen;
    int plen = strlen(path) + 1;
    int alen = (plen + 3) & ~3;

    d1(("--- %s(\"%s\", %p)", __func__, path, utbuf));

    p = xencode_uint32(buf + REQUEST_HEADER_LEN, utbuf->actime);
    (void)xencode_uint32(p, utbuf->modtime);

    iov[0].iov_base = buf;  /* utfs_client_request_handle() will fill this */
    iov[0].iov_len = REQUEST_HEADER_LEN + 8;
    iov[1].iov_base = str_unconst(path); /* const void * when writing... */
    iov[1].iov_len = plen;

    if (plen != alen) {
	iov[2].iov_base = str_unconst(G_pad000); /*const void * when writing */
	iov[2].iov_len = alen - plen;
	iovlen = 3;
    }
    else iovlen = 2;

    alen += 8;

    return utfs_client_request_handle(UTFS__UTIME, iov, iovlen, alen,
				      utfs_ok_cb, null);
}

static uint8_t * utfs_retval_cb(void * cbdata, uint32_t rlen, uint8_t * p)
{
    uint32_t * rv32p = (uint32_t *)cbdata;
    (void)rlen;

    d1(("--- %s(%p, %d, %p)", __func__, cbdata, rlen, p));

    if ((p = utfs_buffer_reserve(p, 4)) == null)
	return null;

    *rv32p = xdecode_uint32(&p);
    return p;
}

static int utfs_open(const char * path, struct fuse_file_info * fi)
{
    uint8_t buf[32];
    struct iovec iov[3];
    int iovlen, error;
    int plen = strlen(path) + 1;
    int alen = (plen + 3) & ~3;
    uint32_t fd;

    d1(("--- %s(\"%s\", %p) %d", __func__, path, fi, fi->flags));

    (void)xencode_uint32(buf + REQUEST_HEADER_LEN, fi->flags);

    iov[0].iov_base = buf; /* utfs_client_request_handle() will fill this */
    iov[0].iov_len = REQUEST_HEADER_LEN + 4;
    iov[1].iov_base = str_unconst(path); /* const void * when writing... */
    iov[1].iov_len = plen;

    if (plen != alen) {
	iov[2].iov_base = str_unconst(G_pad000); /*const void * when writing */
	iov[2].iov_len = alen - plen;
	iovlen = 3;
    }
    else iovlen = 2;

    alen += 4;

    error = utfs_client_request_handle(UTFS__OPEN, iov, iovlen, alen,
				       utfs_retval_cb, (void *)&fd);
    if (error < 0)
	return error;
    fi->fh = fd;
    return 0;
}


static uint8_t * utfs_read_cb(void * cbdata, uint32_t rlen, uint8_t * p)
{
    uint8_t * data = ((struct iovec *)cbdata)->iov_base;
    size_t size = ((struct iovec *)cbdata)->iov_len;
    uint32_t i, alen;

    d1(("--- %s(%p, %d, %p) size %d", __func__, cbdata, rlen, p, size));

    alen = rlen & ~3;

    if (rlen < size)
	memset(data + alen, 0, size - alen);

    ((struct iovec *)cbdata)->iov_len = rlen; /* return value */

    i = utfs_buffer_rest(p);

    if (i >= rlen)
    {
	memcpy(data, p, rlen);
	if (rlen == alen)
	    return p + alen;
	return utfs_buffer_discard(p + alen, 4);
	/* return utfs_buffer_discard(p, (rlen + 3) & ~3); */
    }
    memcpy(data, p, i);
    return utfs_buffer_scatter(data, i, rlen);
}

/* Note: sparse file support would be nice (but not trivial) */
static int utfs_read(const char * path, char * data, size_t size,
		     off_t offset, struct fuse_file_info * fi)
{
    uint8_t buf[32], *p;
    struct iovec iov[1];
    struct iovec readdata;
    int error;

    (void)path;
    d1(("--- %s(\"%s\", %d, %lld)", __func__, path, size, offset));

    p = xencode_uint32(buf + REQUEST_HEADER_LEN, (uint32_t)fi->fh);
    p = xencode_uint32(p, size);
    (void)xencode_uint64(p, offset);

    iov[0].iov_base = buf; /* utfs_client_request_handle() will fill this */
    iov[0].iov_len = REQUEST_HEADER_LEN + 4 + 4 + 8;

    readdata.iov_base = data;
    readdata.iov_len = size;

    error = utfs_client_request_handle(UTFS__READ, iov, 1, 4 + 4 + 8,
				       utfs_read_cb, &readdata);
    if (error < 0)
	return error;
    return readdata.iov_len;
}


static int utfs_write(const char * path, const char * data, size_t size,
		      off_t offset, struct fuse_file_info * fi)
{
    uint8_t buf[32], *p;
    struct iovec iov[3];
    int iovlen, error, alen;
    uint32_t rlen;

    (void)path;
    d1(("--- %s(\"%s\", %d, %lld)", __func__, path, size, offset));

    p = xencode_uint32(buf + REQUEST_HEADER_LEN, (uint32_t)fi->fh);
    p = xencode_uint32(p, size);
    p = xencode_uint64(p, offset);

    iov[0].iov_base = buf; /* utfs_client_request_handle() will fill this */
    iov[0].iov_len = REQUEST_HEADER_LEN + 4 + 4 + 8;
    iov[1].iov_base = str_unconst(data); /* const void * when writing... */
    iov[1].iov_len = size;

    if ((size & 3) != 0) {
	int pad = 4 - (size & 3);
	iov[2].iov_base = str_unconst(G_pad000);
	iov[2].iov_len = pad;
	size += pad;
	iovlen = 3;
    }
    else iovlen = 2;

    alen = 4 + 8 + 4 + size;

    error = utfs_client_request_handle(UTFS__WRITE, iov, iovlen, alen,
				       utfs_retval_cb, (void *)&rlen);
    if (error < 0)
	return error;
    return (int)rlen; /* XXX */
}


static uint8_t * utfs_statfs_cb(void * cbdata, uint32_t rlen, uint8_t * p)
{
    struct statvfs * stbuf = (struct statvfs *)cbdata;
    (void)rlen;

    d1(("--- %s(%p, %d, %p)", __func__, cbdata, rlen, p));

    if ((p = utfs_buffer_reserve(p, 2*4 + 6*8 + 3*4)) == null)
	return null;

    stbuf->f_bsize = xdecode_uint32(&p);
    stbuf->f_frsize = xdecode_uint32(&p);

    stbuf->f_blocks = xdecode_uint64(&p);
    stbuf->f_bfree = xdecode_uint64(&p);
    stbuf->f_bavail = xdecode_uint64(&p);
    stbuf->f_files = xdecode_uint64(&p);
    stbuf->f_ffree = xdecode_uint64(&p);
    stbuf->f_favail = xdecode_uint64(&p);

    stbuf->f_fsid = xdecode_uint32(&p);
    stbuf->f_flag = xdecode_uint32(&p);
    stbuf->f_namemax = xdecode_uint32(&p);

    return p;
}

static int utfs_statfs(const char * path, struct statvfs * stbuf)
{
    uint8_t buf[32];
    struct iovec iov[3];
    int iovlen;
    int plen = strlen(path) + 1;
    int alen = (plen + 3) & ~3;

    d1(("--- %s(\"%s\", %p) %d %d", __func__, path, buf, plen, alen));

    iov[0].iov_base = buf;  /* utfs_client_request_handle() will fill this */
    iov[0].iov_len = REQUEST_HEADER_LEN;
    iov[1].iov_base = str_unconst(path); /* const void * when writing... */
    iov[1].iov_len = plen;

    if (plen != alen) {
	iov[2].iov_base = str_unconst(G_pad000); /*const void * when writing */
	iov[2].iov_len = alen - plen;
	iovlen = 3;
    }
    else iovlen = 2;

    return utfs_client_request_handle(UTFS__STATFS, iov, iovlen, alen,
				      utfs_statfs_cb, (void*)stbuf);
}


static int utfs_release(const char * path, struct fuse_file_info * fi)
{
    uint8_t buf[32], *p;
    struct iovec iov[1];

    (void)path;
    d1(("--- %s(\"%s\", %lld)", __func__, path, fi->fh));

    p = xencode_uint32(buf + REQUEST_HEADER_LEN, (uint32_t)fi->fh);
    iov[0].iov_base = buf; /* utfs_client_request_handle() will fill this */
    iov[0].iov_len = REQUEST_HEADER_LEN + 4;

    return utfs_client_request_handle(UTFS__RELEASE, iov, 1, 4,
					utfs_ok_cb, null);
}


struct utfs_readdir_st
{
    void * fbuf;
    fuse_fill_dir_t filler;
    off_t offset;
};

static uint8_t * utfs_readdir_cb(void * cbdata, uint32_t rlen, uint8_t * p)
{
    struct utfs_readdir_st * rdd_data = (struct utfs_readdir_st *)cbdata;
    void * fbuf = rdd_data->fbuf;
    fuse_fill_dir_t filler = rdd_data->filler;
    off_t offset = rdd_data->offset;
    bool fill = true;
    struct stat st;

    memset(&st, 0, sizeof st);

    d1(("--- %s(%p, %d, %p)", __func__, cbdata, rlen, p));

    while (rlen != 0) {
	uint8_t * q;

	if ((p = utfs_buffer_reserve(p, rlen)) == null)
	    return null;

	q = p + rlen;

	st.st_ino = xdecode_uint64(&p);
	st.st_mode = xdecode_uint8(&p) << 12;
	d1(("dir entry: %s", p));
	if (fill && filler(fbuf, U2S(p, char, *, *), &st, offset) != 0)
	    fill = false;

	/* next header */
	p = utfs_buffer_reserve(q, 16);
#if 0
	type = xdecode_uint32(&p);
	refno = xdecode_uint32(&p);
	errno_ = xdecode_uint32(&p);
#else
	p += 12;
#endif
	rlen = xdecode_uint32(&p);
    }
    return p;
}


static int utfs_readdir(const char* path, void* fbuf, fuse_fill_dir_t filler,
			off_t offset, struct fuse_file_info * fi)
{
    uint8_t buf[32];
    struct iovec iov[3];
    int iovlen;
    int plen = strlen(path) + 1;
    int alen = (plen + 3) & ~3;
    struct utfs_readdir_st rdd_data;

    rdd_data.fbuf = fbuf;
    rdd_data.filler = filler;
    rdd_data.offset = offset;
    (void)fi;

    d1(("--- %s(\"%s\", %p) %d %d", __func__, path, buf, plen, alen));

    (void)xencode_uint64(buf + REQUEST_HEADER_LEN, offset);

    iov[0].iov_base = buf;  /* utfs_client_request_handle() will fill this */
    iov[0].iov_len = REQUEST_HEADER_LEN + 8;
    iov[1].iov_base = str_unconst(path); /* const void * when writing... */
    iov[1].iov_len = plen;

    if (plen != alen) {
	iov[2].iov_base = str_unconst(G_pad000); /*const void * when writing */
	iov[2].iov_len = alen - plen;
	iovlen = 3;
    }
    else iovlen = 2;

    alen += 8;

    return utfs_client_request_handle(UTFS__READDIR, iov, iovlen, alen,
				      utfs_readdir_cb, (void *)&rdd_data);
}


struct fuse_operations utfs_operations = {
    /*getattr*/      utfs_getattr,
    /*readlink*/     utfs_readlink,
    /*getdir*/       utfs_getdir,
    /*mknod*/        utfs_mknod,
    /*mkdir*/        utfs_mkdir,
    /*unlink*/       utfs_unlink,
    /*rmdir*/        utfs_rmdir,
    /*symlink*/      utfs_symlink,
    /*rename*/       utfs_rename,
    /*link*/         utfs_link,
    /*chmod*/        utfs_chmod,
    /*chown*/        utfs_chown,
    /*truncate*/     utfs_truncate,
    /*utime*/        utfs_utime,
    /*open*/         utfs_open,
    /*read*/         utfs_read,
    /*write*/        utfs_write,
    /*statfs*/       utfs_statfs,
    /*flush*/        utfs_flush,
    /*release*/      utfs_release,
    /*fsync*/        utfs_fsync,
    /*setxattr*/     utfs_setxattr,
    /*getxattr*/     utfs_getxattr,
    /*listxattr*/    utfs_listxattr,
    /*removexattr*/  utfs_removexattr,
    /* 2.3 */
    /*opendir*/      utfs_opendir,
    /*readdir*/      utfs_readdir,
    /*releasedir*/   utfs_releasedir,
    /*fsyncdir*/     utfs_fsyncdir,
    /*init*/         utfs_init,
    /*destroy*/      utfs_destroy,
    /* 2.5 */
    /*access*/       utfs_access,
    /*create*/       utfs_create,
    /*ftruncate*/    utfs_ftruncate,
    /*fgetattr*/     utfs_fgetattr,
    /* 2.6 */
    /*lock*/         utfs_lock,
    /*utimens*/      utfs_utimens,
    /*bmap*/         utfs_bmap
};


void utfs_client_fuse_usage(void)
{
    struct fuse_args args = { 0, 0, 0 };

    fuse_opt_add_arg(&args, "...");
    fuse_opt_add_arg(&args, "-V");
    fuse_opt_add_arg(&args, "--help");
    fuse_main(args.argc, args.argv, &utfs_operations, null);
    exit(1);
}

#if 0
static void utfs_client_set_fuse_fsname_arg(struct clientargs * cargs,
					    struct fuse_args * fargs)
{
    char buf[1024];
    /* thanks sshfs */
#if FUSE_VERSION >= 27
    snprintf(buf, sizeof buf, "-osubtype=utfs,fsname=%s:%s",
	     cargs->sshhost, cargs->sshpath);
#else
    snprintf(buf, sizeof buf, "-ofsname=utfs#%s:%s",
	     cargs->sshhost, cargs->sshpath);
#endif
    fuse_opt_insert_arg(fargs, 1, buf);
}
#endif

static int utfs_handle_args_dummy(void * data, const char * arg, int key,
				  struct fuse_args * outargs)
{
    (void)data; (void)arg; (void)key; (void)outargs;
    // let fuse handle all (rest of, that is) args -- if ever here
    return 1;
}

static struct fuse_opt utfs_opts[] = { { null, 0, 0 } };

void utfs_client_handle_args(int * argcp, char *** argvp)
{
    struct fuse_args args = { 0, 0, 0 };

    args.argc = *argcp;
    args.argv = *argvp;

    fuse_opt_parse(&args, null, utfs_opts, utfs_handle_args_dummy);

    *argcp = args.argc;
    *argvp = args.argv;
}

void utfs_client_fuse_main(int argc, char ** argv)
{
    utfs_init_C();
    exit(fuse_main(argc, argv, &utfs_operations, null));
}

/*
 * Local variables:
 * mode: c
 * c-file-style: "stroustrup"
 * tab-width: 8
 * End:
 */
