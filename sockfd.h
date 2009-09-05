#ifndef SOCKFD_H
#define SOCKFD_H

#if WIN32
#include <winsock2.h>
typedef SOCKET sockfd;
#define sockread(s, b, l) recv(s, (char *)(b), l, 0)
#define sockwrite(s, b, l) send(s, (const char *)(b), l, 0)
#define sockclose(s) closesocket(s)
#else
typedef int sockfd;
#define sockread(s, b, l) read(s, b, l)
#define sockwrite(s, b, l) write(s, b, l)
#define sockclose(s) close(s)
#endif

#endif /* SOCKFD_H */
