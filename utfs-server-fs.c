/*
 * $Id; utfs-server-fs.c $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *	Copyright (c) 2007 Tomi Ollila
 *	    All rights reserved
 *
 * Created: Tue Aug 28 21:47:42 EEST 2007 too
 * Last modified: Fri 04 Sep 2009 19:42:17 EEST too
 */

// FIX all XXX in code, then remove this line (gcc warnings)


#include "sockfd.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>
#include <fcntl.h>

#include "defs.h"

#if W32_HAXES
#include "w32x.h"
#else
#include <sys/statvfs.h>
#endif
#include <dirent.h>

#define UTFS_SERVER 1
#define UTFS_PROTOCOL 1
#include "utfs.h"
#include "utfs-errmaps.h"

#include "utfs-buffer.h"

#define SIMPLEXDR_INLINE
#include "simplexdr.h"


static struct {
#define FDOFFSETS 1024
    int64_t fdoffsets[FDOFFSETS];
} S;


static void init_S(void)
{
    int i;

    for (i = 0; i < FDOFFSETS; i++)
	S.fdoffsets[i] = -1;
}

static void _write_simple(uint8_t * s, uint8_t * e)
{
    int len = sockwrite(G_fd_out, s, e - s);
    /* XXX do checks... ? */
    (void)len;
    d1(("--- write_simple: %d = write(%d, ..., %d)", len, G.fd_out, e - s));
#if 0
    for (len = 0; len < e - s; len++)
	printf("%d:%d  ", len, s[len]);
    printf("\n"); fflush(stdout);
#endif
}

static void _write_status(uint32_t type, uint32_t id, uint32_t errNo)
{
    uint8_t buf[32];
    uint8_t * p = buf;

    /* in some architecures, some cycles are saved when comparing w/ 0 */
    if (errNo && errNo >= utfs_server_errmap_size)
	errNo = EDEFAULT;

    p = xencode_uint32(p, type);
    p = xencode_uint32(p, id);
    p = xencode_uint32(p, utfs_server_errmap[errNo]);
    p = xencode_uint32(p, 0); /* length */
    _write_simple(buf, p);
}

/* retvall for 64bits ;) if ever needed ? */
static void _write_retval(uint32_t type, uint32_t id, uint32_t value)
{
    uint8_t buf[32];
    uint8_t * p = buf;

    p = xencode_uint32(p, type);
    p = xencode_uint32(p, id);
    p = xencode_uint32(p, 0); /* no error */
    p = xencode_uint32(p, 4); /* response length */
    p = xencode_uint32(p, value);

    _write_simple(buf, p);
}


static uint8_t * unsupported(uint8_t * p, uint32_t type, uint32_t id, int clen)
{
    d1(("--- unsupported(%p, %x, %d, %d)", p, type, id, clen));
    p = utfs_buffer_discard(p, (clen + 3) & ~3);
    _write_status(type, id, ENOSYS);
    return p;
}

static const char * _mypath(const uint8_t * p)
{
    /* we trust client to be friendly and not send '/../' path components. */
    if (p[0] != '/')  return (const char *)p;
    if (p[1] == '\0') return ".";
    return (const char *)p + 1;
}


static uint8_t * utfs_server_getattr(uint8_t * p,
				     uint32_t type, uint32_t id, int clen)
{
    struct stat st;
    uint8_t buf[128];
    uint8_t * q, *r;

    d1(("--- utfs_server_getattr(%p, %x, %d, %d)", p, type, id, clen));

    if ((p = utfs_buffer_reserve(p, clen)) == null)
	return null;

    r = p + clen;

#if W32_HAXES
#define lstat stat
#endif

    if (lstat(_mypath(p), &st) != 0) {
	_write_status(type, id, errno);
	return r;
    }
    p = buf;
    p = xencode_uint32(p, type);
    p = xencode_uint32(p, id);
    p = xencode_uint32(p, 0); /* no error */
    q = p; p+= 4;
    p = xencode_uint64(p, st.st_dev);
    p = xencode_uint64(p, st.st_ino);
#if W32_HAXES
    p = xencode_uint32(p, st.st_mode | 0x49); /*S_IXUSR | S_IXGRP | S_IXOTH*/
#else
    p = xencode_uint32(p, st.st_mode);
#endif
    p = xencode_uint32(p, st.st_nlink);
    p = xencode_uint32(p, st.st_uid);
    p = xencode_uint32(p, st.st_gid);
    p = xencode_uint64(p, st.st_rdev);
    p = xencode_uint64(p, st.st_size);
#if W32_HAXES
    p = xencode_uint32(p, 1024);
    p = xencode_uint64(p, (st.st_size + 1023) / 1024);
#else
    p = xencode_uint32(p, st.st_blksize);
    p = xencode_uint64(p, st.st_blocks);
#endif
    p = xencode_uint32(p, st.st_atime);
    p = xencode_uint32(p, st.st_mtime);
    p = xencode_uint32(p, st.st_ctime);

    /* 3 ways to optimize this, but no need. */
    (void)xencode_uint32(q, p - q - 4);

    _write_simple(buf, p);
    return r;
}


static uint8_t * utfs_server_readlink(uint8_t * p,
				      uint32_t type, uint32_t id, int clen)
{
    uint8_t buf[4128];
    uint8_t * r;
    uint32_t size;
    ssize_t len;

    d1(("--- utfs_server_readlink(%p, %x, %d, %d)", p, type, id, clen));

    if ((p = utfs_buffer_reserve(p, clen)) == null)
	return null;

    r = p + clen;

    size = xdecode_uint32(&p);
    if (size > sizeof buf - 28) {
	_write_status(type, id, ENAMETOOLONG);
	return r;
    }
    size--;

#if W32_HAXES
    _write_status(type, id, EDEFAULT);
#else
    if ((len = readlink(_mypath(p), U2S(buf + 16, char, *, *), size)) < 0) {
	_write_status(type, id, errno);
	return r;
    }
    buf[len++ + 16] = '\0';
    /* len = (len + 1 + 3) & ~3; (old) */

    p = buf;
    p = xencode_uint32(p, type);
    p = xencode_uint32(p, id);
    p = xencode_uint32(p, 0); /* no error */
    p = xencode_uint32(p, len);

    /* 0 to 3 chars junk, which is ignored by reader (but not valgrind?) */
    _write_simple(buf, p + ((len + 3) & ~3));
#endif
    return r;
}


static uint8_t * utfs_server_mknod(uint8_t * p,
				   uint32_t type, uint32_t id, int clen)
{
    uint8_t * r;
    mode_t mode;
    dev_t dev;  /* is this cross-system compatible */

    d1(("--- utfs_server_mknod(%p, %x, %d, %d)", p, type, id, clen));

    if ((p = utfs_buffer_reserve(p, clen)) == null)
	return null;

    r = p + clen;

    mode = xdecode_uint32(&p);
    dev = xdecode_uint64(&p);

#if W32_HAXES
    errno = EDEFAULT;
#else
    if (mknod(_mypath(p), mode, dev) == 0)
	errno = 0;
#endif
    _write_status(type, id, errno);
    return r;
}


static uint8_t * utfs_server_mkdir(uint8_t * p,
				   uint32_t type, uint32_t id, int clen)
{
    uint8_t * r;
    mode_t mode;

    d1(("--- utfs_server_mkdir(%p, %x, %d, %d)", p, type, id, clen));

    if ((p = utfs_buffer_reserve(p, clen)) == null)
	return null;

    r = p + clen;

    mode = xdecode_uint32(&p);
#if W32_HAXES
    if (mkdir(_mypath(p)) == 0)
	errno = 0;
#else
    if (mkdir(_mypath(p), mode) == 0)
	errno = 0;
#endif
    _write_status(type, id, errno);
    return r;
}


static uint8_t * utfs_server_unlink(uint8_t * p,
				    uint32_t type, uint32_t id, int clen)
{
    uint8_t * r;

    d1(("--- utfs_server_unlink(%p, %x, %d, %d)", p, type, id, clen));

    if ((p = utfs_buffer_reserve(p, clen)) == null)
	return null;

    r = p + clen;

    if (unlink(_mypath(p)) == 0)
	errno = 0;

    _write_status(type, id, errno);
    return r;
}


static uint8_t * utfs_server_rmdir(uint8_t * p,
				   uint32_t type, uint32_t id, int clen)
{
    uint8_t * r;

    d1(("--- utfs_server_rmdir(%p, %x, %d, %d)", p, type, id, clen));

    if ((p = utfs_buffer_reserve(p, clen)) == null)
	return null;

    r = p + clen;

    if (rmdir(_mypath(p)) == 0)
	errno = 0;

    _write_status(type, id, errno);
    return r;
}


static uint8_t * utfs_server_symlink(uint8_t * p,
				     uint32_t type, uint32_t id, int clen)
{
    uint8_t * r;
    uint32_t palen;

    d1(("--- utfs_server_symlink(%p, %x, %d, %d)", p, type, id, clen));

    if ((p = utfs_buffer_reserve(p, clen)) == null)
	return null;

    r = p + clen;

    palen = xdecode_uint32(&p);

#if W32_HAXES
    errno = EDEFAULT;
#else
    /* when combining these 3, enter _mypath-handled strings to common func */
    if (symlink(U2S(p, char, *, *), _mypath(p + palen)) == 0)
	errno = 0;
#endif
    _write_status(type, id, errno);
    return r;
}

/* exactly the same as above. refactor! */
static uint8_t * utfs_server_rename(uint8_t * p,
				    uint32_t type, uint32_t id, int clen)
{
    uint8_t * r;
    uint32_t palen;

    d1(("--- utfs_server_rename(%p, %x, %d, %d)", p, type, id, clen));

    if ((p = utfs_buffer_reserve(p, clen)) == null)
	return null;

    r = p + clen;

    palen = xdecode_uint32(&p);

    if (rename(_mypath(p), _mypath(p + palen)) == 0)
	errno = 0;

    _write_status(type, id, errno);
    return r;
}

/* exactly the same as above. refactor! */
static uint8_t * utfs_server_link(uint8_t * p,
				  uint32_t type, uint32_t id, int clen)
{
    uint8_t * r;
    uint32_t palen;

    d1(("--- utfs_server_link(%p, %x, %d, %d)", p, type, id, clen));

    if ((p = utfs_buffer_reserve(p, clen)) == null)
	return null;

    r = p + clen;

    palen = xdecode_uint32(&p);

#if W32_HAXES
    errno = EDEFAULT;
#else
    if (link(_mypath(p), _mypath(p + palen)) == 0)
	errno = 0;
#endif
    _write_status(type, id, errno);
    return r;
}

/* like mkdir. refactor */
static uint8_t * utfs_server_chmod(uint8_t * p,
				   uint32_t type, uint32_t id, int clen)
{
    uint8_t * r;
    mode_t mode;

    d1(("--- utfs_server_chmod(%p, %x, %d, %d)", p, type, id, clen));

    if ((p = utfs_buffer_reserve(p, clen)) == null)
	return null;

    r = p + clen;

    mode = xdecode_uint32(&p);

    if (chmod(_mypath(p), mode) == 0)
	errno = 0;

    _write_status(type, id, errno);
    return r;
}


static uint8_t * utfs_server_chown(uint8_t * p,
				   uint32_t type, uint32_t id, int clen)
{
    uint8_t * r;
#if W32_HAXES == 0
    uid_t uid;
    gid_t gid;
#endif
    d1(("--- utfs_server_chown(%p, %x, %d, %d)", p, type, id, clen));

    if ((p = utfs_buffer_reserve(p, clen)) == null)
	return null;

    r = p + clen;

#if W32_HAXES
    errno = EDEFAULT;
#else
    uid = xdecode_uint32(&p);
    gid = xdecode_uint32(&p);

    if (chown(_mypath(p), uid, gid) == 0)
	errno = 0;
#endif
    _write_status(type, id, errno);
    return r;
}


static uint8_t * utfs_server_truncate(uint8_t * p,
				      uint32_t type, uint32_t id, int clen)
{
    uint8_t * r;
    off_t size;

    d1(("--- utfs_server_truncate(%p, %x, %d, %d)", p, type, id, clen));

    if ((p = utfs_buffer_reserve(p, clen)) == null)
	return null;

    r = p + clen;

    size = xdecode_uint64(&p);

#if W32_HAXES
    errno = EDEFAULT;
#else
    if (truncate(_mypath(p), size) == 0)
	errno = 0;
#endif
    _write_status(type, id, errno);
    return r;
}


static uint8_t * utfs_server_utime(uint8_t * p,
				   uint32_t type, uint32_t id, int clen)
{
    uint8_t * r;
    struct utimbuf utbuf;

    d1(("--- utfs_server_utime(%p, %x, %d, %d)", p, type, id, clen));

    if ((p = utfs_buffer_reserve(p, clen)) == null)
	return null;

    r = p + clen;

    utbuf.actime  = xdecode_uint32(&p);
    utbuf.modtime = xdecode_uint32(&p);

    if (utime(_mypath(p), &utbuf) == 0)
	errno = 0;

    _write_status(type, id, errno);
    return r;
}

static uint8_t * utfs_server_open(uint8_t * p,
				  uint32_t type, uint32_t id, int clen)
{
    uint8_t * r;
    int fd, flags;

    d1(("--- utfs_server_open(%p, %x, %d, %d)", p, type, id, clen));

    if ((p = utfs_buffer_reserve(p, clen)) == null)
	return null;

    r = p + clen;

#ifndef O_BINARY
#define O_BINARY 0
#endif

    flags = xdecode_uint32(&p) | O_BINARY;

    if ((fd = open(_mypath(p), flags)) >= 0) {
	if (fd < FDOFFSETS) {
	    S.fdoffsets[fd] = 0;
	    _write_retval(type, id, (uint32_t)fd);
	    return r;
	}
	/* else */
	close(fd);
	errno = ENFILE;
    }
    _write_status(type, id, errno);
    return r;
}


static int areadfully(int fd, uint8_t * buf, int len)
{
    int l, ll;

    l = read(fd, buf, len);
    if (l == len || l <= 0)
        return len;

    ll = l;
    buf += l;
    len -= l;
    while ((l = read(fd, buf, len)) > 0) {
        if (l == len)
            return ll + l;
        else
        { ll += l; buf += l; len -= l; }
    }
    /* XXX should this return something else on error */
    return ll;
}


static uint8_t * utfs_server_read(uint8_t * p,
				  uint32_t type, uint32_t id, int clen)
{
    uint8_t * r;
    int32_t fd, len;
    uint32_t size;
    off_t offset;
    uint8_t buf[32768 + 16];

    d1(("--- utfs_server_read(%p, %x, %d, %d)", p, type, id, clen));

    if ((p = utfs_buffer_reserve(p, clen)) == null)
	return null;

    r = p + clen;

    fd = (int32_t)xdecode_uint32(&p);
    size = xdecode_uint32(&p);
    offset = xdecode_uint64(&p);

    if ((uint32_t)fd >= FDOFFSETS || S.fdoffsets[fd] < 0) {
	_write_status(type, id, EBADF);
	return r;
    }
    if (S.fdoffsets[fd] == offset)
	d1(("Optimal offset hit at offset %lld", offset));
    else if (lseek(fd, offset, SEEK_SET) < 0) {
	_write_status(type, id, errno);
	return r;
    }

    if (size > 32768)
	size = 32768;

    len = areadfully(fd, buf + 16, size);
    d1(("%d = read(%d, %p, %d)", len, fd, buf + 16, size));

    if (len < 0) {
	_write_status(type, id, errno);
	return r;
    }

    S.fdoffsets[fd] = offset + len;

    p = buf;
    p = xencode_uint32(p, type);
    p = xencode_uint32(p, id);
    p = xencode_uint32(p, 0); /* no error */
    p = xencode_uint32(p, len); /* note: unaligned value */

    /* 0 to 3 bytes of junk. reader ignores. valgrind(1) may not */
    _write_simple(buf, p + ((len + 3) & ~3));
    return r;
}

static uint8_t * utfs_server_write(uint8_t * p,
				   uint32_t type, uint32_t id, int clen)
{
    uint8_t * r;
    int32_t fd;
    uint32_t size, len;
    off_t offset;

    d1(("utfs_server_write(%p, %x, %d, %d)", p, type, id, clen));

    if ((p = utfs_buffer_reserve(p, 4 + 4 + 8)) == null)
	return null;

    fd = (int32_t)xdecode_uint32(&p);
    size = xdecode_uint32(&p);
    offset = xdecode_uint64(&p);

    r = p;
    clen -= (4 + 8 + 4);

    if ((uint32_t)fd >= FDOFFSETS || S.fdoffsets[fd] < 0) {
	_write_status(type, id, EBADF);
	return r;
    }
    if (S.fdoffsets[fd] == offset)
	d1(("Optimal offset hit at offset %lld", offset));
    else if (lseek(fd, offset, SEEK_SET) < 0) {
	_write_status(type, id, errno);
	return utfs_buffer_discard(r, clen);
    }

    len = utfs_buffer_rest(p);

    if ((int32_t)(len = write(fd, p, len < size? len: size)) < 0) {
	_write_status(type, id, errno);
	return utfs_buffer_discard(r, clen);
    }
    /* XXX partial write above is a problem ! need a wrapper or something */

    if (len >= size)
	r = utfs_buffer_discard(r, clen);
    else
    {
	int needreserve = ((size + 3) & ~3) - len;
	int len2, missing = (size - len);
	if ((p = utfs_buffer_reserve(p + len, needreserve)) == null)
	    return null;
	r = p + needreserve;
	if ((len2 = write(fd, p, missing)) < 0) {
	    _write_status(type, id, errno);
	    return r;
	}
	len += len2;
    }

    S.fdoffsets[fd] = offset + len;

    _write_retval(type, id, len);
    return r;
}


static uint8_t * utfs_server_statfs(uint8_t * p,
				    uint32_t type, uint32_t id, int clen)
{
    struct statvfs stbuf;
    uint8_t buf[128];
    uint8_t * q, *r;

    d1(("utfs_server_statfs(%p, %x, %d, %d)", p, type, id, clen));

    if ((p = utfs_buffer_reserve(p, clen)) == null)
	return null;

    r = p + clen;

    if (statvfs(_mypath(p), &stbuf) != 0) {
	d1(("statvfs('%s') failed", p));
	_write_status(type, id, errno);
	return r;
    }
    p = buf;
    p = xencode_uint32(p, type);
    p = xencode_uint32(p, id);
    p = xencode_uint32(p, 0); /* no error */
    q = p; p+= 4;
    p = xencode_uint32(p, stbuf.f_bsize);
    p = xencode_uint32(p, stbuf.f_frsize);

    p = xencode_uint64(p, stbuf.f_blocks);
    p = xencode_uint64(p, stbuf.f_bfree);
    p = xencode_uint64(p, stbuf.f_bavail);
    p = xencode_uint64(p, stbuf.f_files);
    p = xencode_uint64(p, stbuf.f_ffree);
    p = xencode_uint64(p, stbuf.f_favail);

    p = xencode_uint32(p, stbuf.f_fsid);
    p = xencode_uint32(p, stbuf.f_flag);
    p = xencode_uint32(p, stbuf.f_namemax);

    (void)xencode_uint32(q, p - q - 4);
#if 0
    sleep(5); /* interrupt test */
#endif

    _write_simple(buf, p);
    return r;
}


static uint8_t * utfs_server_release(uint8_t * p,
				     uint32_t type, uint32_t id, int clen)
{
    uint8_t * r;
    int32_t fd;

    d1(("--- utfs_server_release(%p, %x, %d, %d)", p, type, id, clen));

    if ((p = utfs_buffer_reserve(p, clen)) == null)
	return null;

    r = p + clen;

    fd = (int32_t)xdecode_uint32(&p);

    if ((uint32_t)fd >= FDOFFSETS || S.fdoffsets[fd] < 0)
	errno = EBADF;
    else {
	S.fdoffsets[fd] = -1;
	errno = 0;
	close(fd);
    }
    _write_status(type, id, errno);
    return r;
}


static uint8_t * utfs_server_readdir(uint8_t * p,
				     uint32_t type, uint32_t id, int clen)
{
    uint8_t buf[32];
    struct iovec iov[3];
    int iovlen;
    uint8_t * q, *r;
    off_t offset;
    DIR * dir;
    struct dirent * de;

    d1(("utfs_server_readdir(%p, %x, %d, %d)", p, type, id, clen));

    if ((p = utfs_buffer_reserve(p, clen)) == null)
	return null;

    r = p + clen;

    offset = xdecode_uint64(&p);

    if ((dir = opendir(_mypath(p))) == null) {
	_write_status(type, id, errno);
	return r;
    }
    iov[0].iov_base = buf;
    iov[0].iov_len = 16 + 8 + 4;

    p = buf;
    p = xencode_uint32(p, type);
    p = xencode_uint32(p, id);
    p = xencode_uint32(p, 0); /* no error */
    q = p;

    while ((de = readdir(dir)) != null) {
	int plen, alen;
	if (offset > 0) { offset--; continue; }
	p = q + 4;

	p = xencode_uint64(p, de->d_ino);
#if W32_HAXES
	p = xencode_uint8(p, 8); // regular file ?
#else
	p = xencode_uint8(p, de->d_type);
#endif
	iov[1].iov_base = de->d_name;
	plen = strlen(de->d_name) + 1;
	alen = (plen + 3) & ~3;
	iov[1].iov_len = plen;

	if (plen != alen) {
	    iov[2].iov_base = G_pad000;  /* const void * when writing... */
	    iov[2].iov_len = alen - plen;
	    iovlen = 3;
	}
	else iovlen = 2;

	(void)xencode_uint32(q, p - q - 4 + alen);
	writev(G_fd_out, iov, iovlen); /* XXX all checks... */
    }
    closedir(dir);

    /* write "eod" info */
    (void)xencode_uint32(q, 0);

    _write_simple(buf, q + 4);
    return r;
}


uint8_t * (*ft[])(uint8_t * p, uint32_t type, uint32_t id, int clen) = {
    unsupported, /* client-server communication type, not defined (yet) */
    utfs_server_getattr, /* 0 */
    utfs_server_readlink,
    unsupported,
    utfs_server_mknod,
    utfs_server_mkdir,
    utfs_server_unlink,
    utfs_server_rmdir,
    utfs_server_symlink,
    utfs_server_rename,
    utfs_server_link,

    utfs_server_chmod,
    utfs_server_chown,
    utfs_server_truncate,
    utfs_server_utime,

    utfs_server_open,
    utfs_server_read,
    utfs_server_write,
    utfs_server_statfs, /* 17 */
    unsupported,
    utfs_server_release,
    unsupported,
    unsupported,
    unsupported,
    unsupported,
    unsupported,
    /* 2.3 */
    unsupported,
    utfs_server_readdir, /* 25 */
    unsupported,
    unsupported,
    unsupported,
    unsupported
    /* 2.5 */
};

static const uint32_t ftsize = sizeof ft / sizeof ft[0];

void utfs_server()
{
    uint8_t * p;

    init_S();
    p = utfs_buffer_init(G_fd_in);

    while (1)
    {
	uint32_t type, id, clen, type0;

	p = utfs_buffer_reserve(p, 12);
	if (p == null)
	    exit(1); // XXX

	type = xdecode_uint32(&p);
	id = xdecode_uint32(&p);
	clen = xdecode_uint32(&p);

	type0 = type - UTFS__OFFSET;
	d0(("type %d", type0 - 1));
	if (type0 < ftsize) {
	    if ((p = ft[type0](p, type, id, clen)) == null)
		exit(1); // XXX
	}
	else if (type > 1023
		 || (p = unsupported(p, type, id, clen)) == null)
	    exit(1); // XXX
	    /* else continue */
    }
}

/*
 * Local variables:
 * mode: c
 * c-file-style: "stroustrup"
 * tab-width: 8
 * End:
 */
