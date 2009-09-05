/*
 * $Id; defs.h $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *	Copyright (c) 2007 Tomi Ollila
 *	    All rights reserved
 *
 * Created: Tue Aug 21 18:40:17 EEST 2007 too
 * Last modified: Tue 12 May 2009 22:46:52 EEST too
 */

#ifndef DEFS_H
#define DEFS_H

/* we'll use C99 for now */
#include <stdint.h>
#if 0
typedef int8_t  ei8;            typedef uint8_t  eu8;
typedef int16_t ei16;           typedef uint16_t eu16;
typedef int32_t ei32;           typedef uint32_t eu32;

typedef int_least8_t  li8;      typedef uint_least8_t  lu8;
typedef int_least16_t li16;     typedef uint_least16_t lu16;
typedef int_least32_t li32;     typedef uint_least32_t lu32;

typedef int_fast8_t  fi8;       typedef uint_fast8_t  fu8;
typedef int_fast16_t fi16;      typedef uint_fast16_t fu16;
typedef int_fast32_t fi32;      typedef uint_fast32_t fu32;
#endif

#define null ((void*)0)
typedef enum { false = 0, true = 1 } bool; typedef char bool8;

/* this is for quick hacks -- set 0 for WIN32 too to look haxes again */
#if WIN32
#define W32_HAXES 1
#else
#define W32_HAXES 0
#endif

#if DEBUG
#include <string.h> /* for strerror() */
void warn(const char * format, ...);
#define d1(x) do { warn x; } while (0)
#define d0(x) do {} while (0)
#define d1x(x) do { x; } while (0)
#define d0x(x) do {} while (0)
#else
#define d1(x) do {} while (0)
#define d0(x) do {} while (0)
#define d1x(x) do {} while (0)
#define d0x(x) do {} while (0)
#endif

#if (__GNUC__ >= 4)
#define GCCATTR_SENTINEL __attribute__ ((sentinel))
#else
#define GCCATTR_SENTINEL
#endif

#if (__GNUC__ >= 3)
#define GCCATTR_PRINTF(m, n) __attribute__ ((format (printf, m, n)))
#define GCCATTR_NORETURN __attribute__ ((noreturn))
#define GCCATTR_UNUSED   __attribute__ ((unused))
#define GCCATTR_CONST    __attribute__ ((const))

#define S2U(v, t, i, o)	\
    __builtin_choose_expr (__builtin_types_compatible_p \
			   (typeof (v), t i), ((unsigned t o)(v)), (void)0)
#define U2S(v, t, i, o)	\
    __builtin_choose_expr (__builtin_types_compatible_p	\
			   (typeof (v), unsigned t i), ((t o)(v)), (void)0)
#else
#define GCCATTR_PRINTF(m, n)
#define GCCATTR_NORETURN
#define GCCATTR_UNUSED
#define GCCATTR_CONST

#define S2U(v, t, i, o) ((unsigned t o)(v))
#define U2S(v, t, i, o) ((t o)(v))
#endif

#define UU GCCATTR_UNUSED /* convenience macro */

/* use this only to cast quoted strings in function calls */
#define CUS (const unsigned char *)

/* CS == constant string */
#define WriteCS(fd, s) writeall(fd, s, sizeof s - 1)
#define StrCpyCS(d, s) memcpy(d, s, sizeof s)
#define StrCmpCS0(v, c) memcmp(v, c, sizeof c - 1)
#define StrCmpCS(v, c) memcmp(v, c, sizeof c)

#endif /* DEFS_H */

/*
 * Local variables:
 * mode: c
 * c-file-style: "stroustrup"
 * tab-width: 8
 * End:
 */
