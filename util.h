/*
 * $Id; util.h $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *	Copyright (c) 2007 Tomi Ollila
 *	    All rights reserved
 *
 * Created: Sun Aug 26 22:59:58 EEST 2007 too
 * Last modified: Fri 04 Sep 2009 18:12:22 EEST too
 */

#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h>

#include "defs.h"

/* Life is constant learning. That's why this file, and some of the
   others use different ways to do some things (not so consistent always,
   that is). Also, I am too lazy to consolidate all different techniques
   to the one (probably not best, but consistent at least). Also, I've
   thougth of merging defs.h and utils.h to one, but that probably won't
   happen either (or putting all of these to utfs.h (and .c) */

/* to be moved (or not...) */
extern const char G_pad000[3];

extern struct DieHookStruct {
    void (*func)(void * data);
    void * data;
} die_hook;

#if ! UTIL_C
extern
#endif
/**/ struct {
    const char * component;
} Util;

void util_init(const char * component);

void vwarn(const char * format, va_list ap);
void warn(const char * format, ...);
void die(const char * format, ...) GCCATTR_NORETURN;
#endif /* UTIL_H */

/*
 * Local variables:
 * mode: c
 * c-file-style: "stroustrup"
 * tab-width: 8
 * End:
 */
