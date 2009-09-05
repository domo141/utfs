/*
 * $Id; utfs-client-errmap.c $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *	Copyright (c) 2008 Tomi Ollila
 *	    All rights reserved
 *
 * Created: Mon Jan 07 19:28:29 EET 2008 too
 * Last modified: Fri 04 Sep 2009 18:12:22 EEST too
 */

#if 0 /*
#!perl
open I, '<', $0 or die;
while (<I>) { push @head, $_; last if /<[v]>/; }
while (<I>) { last if /<[\^]>/; }; push @tail, $_;
while (<I>) { push @tail, $_; push @list, $1 if /^\s*(E[A-Z]\w+),?\s/; }
close I;
rename $0, "$0.bak" unless -e "$0.bak"; open O, '>', $0 or die; select O;
print @head, "\n";
for (@list) { print "#ifndef $_\n#define $_ EDEFAULT\n#endif\n"; }
print "\n", @tail;
__END__
 */
#endif

#include "utfs-errmaps.h"

#if EDEFAULT_FOR_PP /* used when preprocessing for utfs-server-errmap.c */
edefault=EDEFAULT
#endif

/* The defines below (until  <^> ) generated. do not edit by hand */

/*<v>*/

#ifndef EPERM
#define EPERM EDEFAULT
#endif
#ifndef ENOENT
#define ENOENT EDEFAULT
#endif
#ifndef ESRCH
#define ESRCH EDEFAULT
#endif
#ifndef EINTR
#define EINTR EDEFAULT
#endif
#ifndef EIO
#define EIO EDEFAULT
#endif
#ifndef ENXIO
#define ENXIO EDEFAULT
#endif
#ifndef ENOEXEC
#define ENOEXEC EDEFAULT
#endif
#ifndef EBADF
#define EBADF EDEFAULT
#endif
#ifndef ECHILD
#define ECHILD EDEFAULT
#endif
#ifndef EAGAIN
#define EAGAIN EDEFAULT
#endif
#ifndef ENOMEM
#define ENOMEM EDEFAULT
#endif
#ifndef EACCES
#define EACCES EDEFAULT
#endif
#ifndef EFAULT
#define EFAULT EDEFAULT
#endif
#ifndef ENOTBLK
#define ENOTBLK EDEFAULT
#endif
#ifndef EBUSY
#define EBUSY EDEFAULT
#endif
#ifndef EEXIST
#define EEXIST EDEFAULT
#endif
#ifndef EXDEV
#define EXDEV EDEFAULT
#endif
#ifndef ENODEV
#define ENODEV EDEFAULT
#endif
#ifndef ENOTDIR
#define ENOTDIR EDEFAULT
#endif
#ifndef EISDIR
#define EISDIR EDEFAULT
#endif
#ifndef EINVAL
#define EINVAL EDEFAULT
#endif
#ifndef ENFILE
#define ENFILE EDEFAULT
#endif
#ifndef EMFILE
#define EMFILE EDEFAULT
#endif
#ifndef ENOTTY
#define ENOTTY EDEFAULT
#endif
#ifndef ETXTBSY
#define ETXTBSY EDEFAULT
#endif
#ifndef EFBIG
#define EFBIG EDEFAULT
#endif
#ifndef ENOSPC
#define ENOSPC EDEFAULT
#endif
#ifndef ESPIPE
#define ESPIPE EDEFAULT
#endif
#ifndef EROFS
#define EROFS EDEFAULT
#endif
#ifndef EMLINK
#define EMLINK EDEFAULT
#endif
#ifndef EPIPE
#define EPIPE EDEFAULT
#endif
#ifndef EDOM
#define EDOM EDEFAULT
#endif
#ifndef ERANGE
#define ERANGE EDEFAULT
#endif
#ifndef EDEADLK
#define EDEADLK EDEFAULT
#endif
#ifndef ENAMETOOLONG
#define ENAMETOOLONG EDEFAULT
#endif
#ifndef ENOLCK
#define ENOLCK EDEFAULT
#endif
#ifndef ENOSYS
#define ENOSYS EDEFAULT
#endif
#ifndef ENOTEMPTY
#define ENOTEMPTY EDEFAULT
#endif
#ifndef ELOOP
#define ELOOP EDEFAULT
#endif
#ifndef EWOULDBLOCK
#define EWOULDBLOCK EDEFAULT
#endif
#ifndef ENOMSG
#define ENOMSG EDEFAULT
#endif
#ifndef EIDRM
#define EIDRM EDEFAULT
#endif
#ifndef ECHRNG
#define ECHRNG EDEFAULT
#endif
#ifndef EL2NSYNC
#define EL2NSYNC EDEFAULT
#endif
#ifndef EL3HLT
#define EL3HLT EDEFAULT
#endif
#ifndef EL3RST
#define EL3RST EDEFAULT
#endif
#ifndef ELNRNG
#define ELNRNG EDEFAULT
#endif
#ifndef EUNATCH
#define EUNATCH EDEFAULT
#endif
#ifndef ENOCSI
#define ENOCSI EDEFAULT
#endif
#ifndef EL2HLT
#define EL2HLT EDEFAULT
#endif
#ifndef EBADE
#define EBADE EDEFAULT
#endif
#ifndef EBADR
#define EBADR EDEFAULT
#endif
#ifndef EXFULL
#define EXFULL EDEFAULT
#endif
#ifndef ENOANO
#define ENOANO EDEFAULT
#endif
#ifndef EBADRQC
#define EBADRQC EDEFAULT
#endif
#ifndef EBADSLT
#define EBADSLT EDEFAULT
#endif
#ifndef EDEADLOCK
#define EDEADLOCK EDEFAULT
#endif
#ifndef EBFONT
#define EBFONT EDEFAULT
#endif
#ifndef ENOSTR
#define ENOSTR EDEFAULT
#endif
#ifndef ENODATA
#define ENODATA EDEFAULT
#endif
#ifndef ETIME
#define ETIME EDEFAULT
#endif
#ifndef ENOSR
#define ENOSR EDEFAULT
#endif
#ifndef ENONET
#define ENONET EDEFAULT
#endif
#ifndef ENOPKG
#define ENOPKG EDEFAULT
#endif
#ifndef EREMOTE
#define EREMOTE EDEFAULT
#endif
#ifndef ENOLINK
#define ENOLINK EDEFAULT
#endif
#ifndef EADV
#define EADV EDEFAULT
#endif
#ifndef ESRMNT
#define ESRMNT EDEFAULT
#endif
#ifndef ECOMM
#define ECOMM EDEFAULT
#endif
#ifndef EPROTO
#define EPROTO EDEFAULT
#endif
#ifndef EMULTIHOP
#define EMULTIHOP EDEFAULT
#endif
#ifndef EDOTDOT
#define EDOTDOT EDEFAULT
#endif
#ifndef EBADMSG
#define EBADMSG EDEFAULT
#endif
#ifndef EOVERFLOW
#define EOVERFLOW EDEFAULT
#endif
#ifndef ENOTUNIQ
#define ENOTUNIQ EDEFAULT
#endif
#ifndef EBADFD
#define EBADFD EDEFAULT
#endif
#ifndef EREMCHG
#define EREMCHG EDEFAULT
#endif
#ifndef ELIBACC
#define ELIBACC EDEFAULT
#endif
#ifndef ELIBBAD
#define ELIBBAD EDEFAULT
#endif
#ifndef ELIBSCN
#define ELIBSCN EDEFAULT
#endif
#ifndef ELIBMAX
#define ELIBMAX EDEFAULT
#endif
#ifndef ELIBEXEC
#define ELIBEXEC EDEFAULT
#endif
#ifndef EILSEQ
#define EILSEQ EDEFAULT
#endif
#ifndef ERESTART
#define ERESTART EDEFAULT
#endif
#ifndef ESTRPIPE
#define ESTRPIPE EDEFAULT
#endif
#ifndef EUSERS
#define EUSERS EDEFAULT
#endif
#ifndef ENOTSOCK
#define ENOTSOCK EDEFAULT
#endif
#ifndef EDESTADDRREQ
#define EDESTADDRREQ EDEFAULT
#endif
#ifndef EMSGSIZE
#define EMSGSIZE EDEFAULT
#endif
#ifndef EPROTOTYPE
#define EPROTOTYPE EDEFAULT
#endif
#ifndef ENOPROTOOPT
#define ENOPROTOOPT EDEFAULT
#endif
#ifndef EPROTONOSUPPORT
#define EPROTONOSUPPORT EDEFAULT
#endif
#ifndef ESOCKTNOSUPPORT
#define ESOCKTNOSUPPORT EDEFAULT
#endif
#ifndef EOPNOTSUPP
#define EOPNOTSUPP EDEFAULT
#endif
#ifndef EPFNOSUPPORT
#define EPFNOSUPPORT EDEFAULT
#endif
#ifndef EAFNOSUPPORT
#define EAFNOSUPPORT EDEFAULT
#endif
#ifndef EADDRINUSE
#define EADDRINUSE EDEFAULT
#endif
#ifndef EADDRNOTAVAIL
#define EADDRNOTAVAIL EDEFAULT
#endif
#ifndef ENETDOWN
#define ENETDOWN EDEFAULT
#endif
#ifndef ENETUNREACH
#define ENETUNREACH EDEFAULT
#endif
#ifndef ENETRESET
#define ENETRESET EDEFAULT
#endif
#ifndef ECONNABORTED
#define ECONNABORTED EDEFAULT
#endif
#ifndef ECONNRESET
#define ECONNRESET EDEFAULT
#endif
#ifndef ENOBUFS
#define ENOBUFS EDEFAULT
#endif
#ifndef EISCONN
#define EISCONN EDEFAULT
#endif
#ifndef ENOTCONN
#define ENOTCONN EDEFAULT
#endif
#ifndef ESHUTDOWN
#define ESHUTDOWN EDEFAULT
#endif
#ifndef ETOOMANYREFS
#define ETOOMANYREFS EDEFAULT
#endif
#ifndef ETIMEDOUT
#define ETIMEDOUT EDEFAULT
#endif
#ifndef ECONNREFUSED
#define ECONNREFUSED EDEFAULT
#endif
#ifndef EHOSTDOWN
#define EHOSTDOWN EDEFAULT
#endif
#ifndef EHOSTUNREACH
#define EHOSTUNREACH EDEFAULT
#endif
#ifndef EALREADY
#define EALREADY EDEFAULT
#endif
#ifndef EINPROGRESS
#define EINPROGRESS EDEFAULT
#endif
#ifndef ESTALE
#define ESTALE EDEFAULT
#endif
#ifndef EUCLEAN
#define EUCLEAN EDEFAULT
#endif
#ifndef ENOTNAM
#define ENOTNAM EDEFAULT
#endif
#ifndef ENAVAIL
#define ENAVAIL EDEFAULT
#endif
#ifndef EISNAM
#define EISNAM EDEFAULT
#endif
#ifndef EREMOTEIO
#define EREMOTEIO EDEFAULT
#endif
#ifndef EDQUOT
#define EDQUOT EDEFAULT
#endif
#ifndef ENOMEDIUM
#define ENOMEDIUM EDEFAULT
#endif
#ifndef EMEDIUMTYPE
#define EMEDIUMTYPE EDEFAULT
#endif
#ifndef ECANCELED
#define ECANCELED EDEFAULT
#endif
#ifndef ENOKEY
#define ENOKEY EDEFAULT
#endif
#ifndef EKEYEXPIRED
#define EKEYEXPIRED EDEFAULT
#endif
#ifndef EKEYREVOKED
#define EKEYREVOKED EDEFAULT
#endif
#ifndef EKEYREJECTED
#define EKEYREJECTED EDEFAULT
#endif
#ifndef EOWNERDEAD
#define EOWNERDEAD EDEFAULT
#endif
#ifndef ENOTRECOVERABLE
#define ENOTRECOVERABLE EDEFAULT
#endif

/*<^>*/

/* Currently this list contains entries in order they are in linux... */
/* It does not matter what the order is ... */
/* Add more on demand... */
/* And run 'perl -x utfs-client-errmap.c' to regenerate defines above */

const unsigned char utfs_client_errmap[] = {
    0,
    EPERM,		/* Operation not permitted */
    ENOENT,		/* No such file or directory */
    ESRCH,		/* No such process */
    EINTR,		/* Interrupted system call */
    EIO,		/* I/O error */
    ENXIO,		/* No such device or address */
    E2BIG,		/* Argument list too long */
    ENOEXEC,		/* Exec format error */
    EBADF,		/* Bad file number */
    ECHILD,		/* No child processes */
    EAGAIN,		/* Try again */
    ENOMEM,		/* Out of memory */
    EACCES,		/* Permission denied */
    EFAULT,		/* Bad address */
    ENOTBLK,		/* Block device required */
    EBUSY,		/* Device or resource busy */
    EEXIST,		/* File exists */
    EXDEV,		/* Cross-device link */
    ENODEV,		/* No such device */
    ENOTDIR,		/* Not a directory */
    EISDIR,		/* Is a directory */
    EINVAL,		/* Invalid argument */
    ENFILE,		/* File table overflow */
    EMFILE,		/* Too many open files */
    ENOTTY,		/* Not a typewriter */
    ETXTBSY,		/* Text file busy */
    EFBIG,		/* File too large */
    ENOSPC,		/* No space left on device */
    ESPIPE,		/* Illegal seek */
    EROFS,		/* Read-only file system */
    EMLINK,		/* Too many links */
    EPIPE,		/* Broken pipe */
    EDOM,		/* Math argument out of domain of func */
    ERANGE,		/* Math result not representable */

    /* end of errno-base.h (in linux...) */

    EDEADLK,		/* Resource deadlock would occur */
    ENAMETOOLONG,	/* File name too long */
    ENOLCK,		/* No record locks available */
    ENOSYS,		/* Function not implemented */
    ENOTEMPTY,		/* Directory not empty */
    ELOOP,		/* Too many symbolic links encountered */
    EWOULDBLOCK,	/* Operation would block */
    ENOMSG,		/* No message of desired type */
    EIDRM,		/* Identifier removed */
    ECHRNG,		/* Channel number out of range */
    EL2NSYNC,		/* Level 2 not synchronized */
    EL3HLT,		/* Level 3 halted */
    EL3RST,		/* Level 3 reset */
    ELNRNG,		/* Link number out of range */
    EUNATCH,		/* Protocol driver not attached */
    ENOCSI,		/* No CSI structure available */
    EL2HLT,		/* Level 2 halted */
    EBADE,		/* Invalid exchange */
    EBADR,		/* Invalid request descriptor */
    EXFULL,		/* Exchange full */
    ENOANO,		/* No anode */
    EBADRQC,		/* Invalid request code */
    EBADSLT,		/* Invalid slot */

    EDEADLOCK,

    EBFONT,		/* Bad font file format */
    ENOSTR,		/* Device not a stream */
    ENODATA,		/* No data available */
    ETIME,		/* Timer expired */
    ENOSR,		/* Out of streams resources */
    ENONET,		/* Machine is not on the network */
    ENOPKG,		/* Package not installed */
    EREMOTE,		/* Object is remote */
    ENOLINK,		/* Link has been severed */
    EADV,		/* Advertise error */
    ESRMNT,		/* Srmount error */
    ECOMM,		/* Communication error on send */
    EPROTO,		/* Protocol error */
    EMULTIHOP,		/* Multihop attempted */
    EDOTDOT,		/* RFS specific error */
    EBADMSG,		/* Not a data message */
    EOVERFLOW,		/* Value too large for defined data type */
    ENOTUNIQ,		/* Name not unique on network */
    EBADFD,		/* File descriptor in bad state */
    EREMCHG,		/* Remote address changed */
    ELIBACC,		/* Can not access a needed shared library */
    ELIBBAD,		/* Accessing a corrupted shared library */
    ELIBSCN,		/* .lib section in a.out corrupted */
    ELIBMAX,		/* Attempting to link in too many shared libraries */
    ELIBEXEC,		/* Cannot exec a shared library directly */
    EILSEQ,		/* Illegal byte sequence */
    ERESTART,		/* Interrupted system call should be restarted */
    ESTRPIPE,		/* Streams pipe error */
    EUSERS,		/* Too many users */
    ENOTSOCK,		/* Socket operation on non-socket */
    EDESTADDRREQ,	/* Destination address required */
    EMSGSIZE,		/* Message too long */
    EPROTOTYPE,		/* Protocol wrong type for socket */
    ENOPROTOOPT,	/* Protocol not available */
    EPROTONOSUPPORT,	/* Protocol not supported */
    ESOCKTNOSUPPORT,	/* Socket type not supported */
    EOPNOTSUPP,		/* Operation not supported on transport endpoint */
    EPFNOSUPPORT,	/* Protocol family not supported */
    EAFNOSUPPORT,	/* Address family not supported by protocol */
    EADDRINUSE,		/* Address already in use */
    EADDRNOTAVAIL,	/* Cannot assign requested address */
    ENETDOWN,		/* Network is down */
    ENETUNREACH,	/* Network is unreachable */
    ENETRESET,		/* Network dropped connection because of reset */
    ECONNABORTED,	/* Software caused connection abort */
    ECONNRESET,		/* Connection reset by peer */
    ENOBUFS,		/* No buffer space available */
    EISCONN,		/* Transport endpoint is already connected */
    ENOTCONN,		/* Transport endpoint is not connected */
    ESHUTDOWN,		/* Cannot send after transport endpoint shutdown */
    ETOOMANYREFS,	/* Too many references: cannot splice */
    ETIMEDOUT,		/* Connection timed out */
    ECONNREFUSED,	/* Connection refused */
    EHOSTDOWN,		/* Host is down */
    EHOSTUNREACH,	/* No route to host */
    EALREADY,		/* Operation already in progress */
    EINPROGRESS,	/* Operation now in progress */
    ESTALE,		/* Stale NFS file handle */
    EUCLEAN,		/* Structure needs cleaning */
    ENOTNAM,		/* Not a XENIX named type file */
    ENAVAIL,		/* No XENIX semaphores available */
    EISNAM,		/* Is a named type file */
    EREMOTEIO,		/* Remote I/O error */
    EDQUOT,		/* Quota exceeded */

    ENOMEDIUM,		/* No medium found */
    EMEDIUMTYPE,	/* Wrong medium type */
    ECANCELED,		/* Operation Canceled */
    ENOKEY,		/* Required key not available */
    EKEYEXPIRED,	/* Key has expired */
    EKEYREVOKED,	/* Key has been revoked */
    EKEYREJECTED,	/* Key was rejected by service */

    EOWNERDEAD,		/* Owner died */
    ENOTRECOVERABLE	/* State not recoverable */
};

const unsigned int utfs_client_errmap_size =
    sizeof utfs_client_errmap / sizeof utfs_client_errmap[0];

/*
 * Local variables:
 * mode: c
 * c-file-style: "stroustrup"
 * tab-width: 8
 * End:
 */
