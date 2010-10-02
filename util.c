/*
 * $Id; util.c $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *	Copyright (c) 2007 Tomi Ollila
 *	    All rights reserved
 *
 * Created: Sun Aug 26 22:59:02 EEST 2007 too
 * Last modified: Sat 02 Oct 2010 13:50:22 EEST too
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define UTIL_C 1
#include "util.h"

/* prgname without any path components*/
extern const char * prgname0;

void vwarn(const char * format, va_list ap)
{
    int error = errno; /* XXX is this too late ? */

    if (memcmp(format, "%C:", 3) == 0) {
	fputs(prgname0, stderr);
	format += 2;
    }
    vfprintf(stderr, format, ap);
    if (format[strlen(format) - 1] == ':')
	fprintf(stderr, " %s\n", strerror(error));
    else
	fputs("\n", stderr);
    fflush(stderr);
}

struct DieHookStruct die_hook = { null, null };

void die(const char * format, ...)
{
    va_list ap;

    va_start(ap, format);
    vwarn(format, ap);
    va_end(ap);

    if (die_hook.func) die_hook.func(die_hook.data);
    exit(1);
}

void warn(const char * format, ...)
{
    va_list ap;

    va_start(ap, format);
    vwarn(format, ap);
    va_end(ap);
}

/*
 * Local variables:
 * mode: c
 * c-file-style: "stroustrup"
 * tab-width: 8
 * End:
 */
