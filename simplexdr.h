/*
 * $Id; simplexdr.h $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *	Copyright (c) 2007 Tomi Ollila
 *	    All rights reserved
 *
 * Created: Tue Dec 25 11:05:52 EET 2007 too
 * Last modified: Tue 12 May 2009 23:13:22 EEST too
 */

#ifndef SIMPLEXDR_H
#define SIMPLEXDR_H

#ifndef SIMPLEXDR_HAVECODE /* trigger. defined only in simplexdr.c */

#ifdef SIMPLEXDR_INLINE
#define SIMPLEXDR_HAVECODE 1
#define __SIMPLEXDR_INLINE static inline
#else
#define SIMPLEXDR_HAVECODE 0
#define __SIMPLEXDR_INLINE
#endif

#else /* SIMPLEXDR_HAVECODE not defined */

#undef SIMPLEXDR_HAVECODE
#define SIMPLEXDR_HAVECODE 1
#define __SIMPLEXDR_INLINE

#endif

/* cross-gcc -dM -E - </dev/null */

#include <stdint.h>
#if WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <string.h>


__SIMPLEXDR_INLINE
uint8_t * xencode_uint8(uint8_t * p, uint8_t v)
#if ! SIMPLEXDR_HAVECODE
    ;
#else
{
    *(uint32_t *)p = htonl((uint32_t)v);
    return p + 4;
}
#endif

__SIMPLEXDR_INLINE
uint8_t xdecode_uint8(uint8_t ** pp)
#if ! SIMPLEXDR_HAVECODE
    ;
#else
{
    uint8_t * p = *pp;
    *pp += 4;
    return (uint8_t)ntohl(*(uint32_t *)p);
}
#endif

/* --- */

__SIMPLEXDR_INLINE
uint8_t * xencode_uint32(uint8_t * p, uint32_t v)
#if ! SIMPLEXDR_HAVECODE
    ;
#else
{
    *(uint32_t *)p = htonl(v);
    return p + 4;
}
#endif

__SIMPLEXDR_INLINE
uint32_t xdecode_uint32(uint8_t ** pp)
#if ! SIMPLEXDR_HAVECODE
    ;
#else
{
    uint8_t * p = *pp;
    *pp += 4;
    return (uint32_t)ntohl(*(uint32_t *)p);
}
#endif

/* --- */

__SIMPLEXDR_INLINE
uint8_t * xencode_uint64(uint8_t * p, uint64_t v)
#if ! SIMPLEXDR_HAVECODE
    ;
#else
{
    *(uint32_t *)p = htonl(v >> 32);
    *(uint32_t *)(p + 4) = htonl(v);

    return p + 8;
}
#endif

__SIMPLEXDR_INLINE
uint64_t xdecode_uint64(uint8_t ** pp)
#if ! SIMPLEXDR_HAVECODE
    ;
#else
{
    uint64_t v;
    uint8_t * p = *pp;
    *pp += 8;

    v  = (uint64_t)ntohl(*(uint32_t *)p) << 32;
    v |= ntohl(*(uint32_t *)(p + 4));
    return v;
}
#endif


#endif /* SIMPLEXDR_H */

/*
 * Local variables:
 * mode: c
 * c-file-style: "stroustrup"
 * tab-width: 8
 * End:
 */
