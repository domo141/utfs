#ifndef W32X_H
#define W32X_H

#if ! WIN32
#error for w32 only
#endif

#include <sys/types.h>

#include "sockfd.h"

#include "w32x_ext.h"

struct iovec {
    void * iov_base;
    size_t iov_len;
};

ssize_t readv(sockfd fd, const struct iovec * iov, int iovcnt);
ssize_t writev(sockfd fd, const struct iovec * iov, int iovcnt);

#endif /* W32X_H */
