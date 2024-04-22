/*
 * Copyright (c) 2019
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* MP support header */
#include "MacportsLegacySupport.h"

/* realpath wrap */
#if __MP_LEGACY_SUPPORT_REALPATH_WRAP__

#include <limits.h>
#include <stdlib.h>
#include <dlfcn.h>

char *realpath(const char * __restrict stringsearch, char * __restrict buffer)
{
    char *(*real_realpath)(const char * __restrict, char * __restrict);
#if (__DARWIN_UNIX03 && !defined(_POSIX_C_SOURCE)) || defined(_DARWIN_C_SOURCE) || defined(_DARWIN_BETTER_REALPATH)
    real_realpath = dlsym(RTLD_NEXT, "realpath$DARWIN_EXTSN");
# else
    real_realpath = dlsym(RTLD_NEXT, "realpath");
#endif
    if (real_realpath == NULL) {
	exit(EXIT_FAILURE);
    }

    if (buffer == NULL) {
        char *myrealpathbuf = malloc(PATH_MAX);
        if (myrealpathbuf != NULL) {
            return(real_realpath(stringsearch, myrealpathbuf));
        } else {
            return(NULL);
        }
    } else {
        return(real_realpath(stringsearch, buffer));
    }
}

/* compatibility function so code does not have to be recompiled */
char *macports_legacy_realpath(const char * __restrict stringsearch, char * __restrict buffer) { return realpath(stringsearch, buffer); }

#endif /*__MP_LEGACY_SUPPORT_REALPATH_WRAP__*/
