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

/* sysconf wrap */
#if __MP_LEGACY_SUPPORT_SYSCONF_WRAP__

#include <sys/types.h>
#include <sys/sysctl.h>

#include <unistd.h>
#include <dlfcn.h>
#include <stdlib.h>

/* emulate several commonly used but missing selectors from sysconf() on various OS versions */

long sysconf(int name) {
    long (*real_sysconf)(int);

#if __MP_LEGACY_SUPPORT_SYSCONF_WRAP_NEED_SC_NPROCESSORS_ONLN__
    if ( name == _SC_NPROCESSORS_ONLN ) {

        int nm[2];
        int ret;
        size_t len = 4;
        uint32_t count;

        nm[0] = CTL_HW; nm[1] = HW_AVAILCPU;
        ret = sysctl(nm, 2, &count, &len, NULL, 0);

        if (ret < 0 || count < 1) {
            /* try again with _SC_NPROCESSORS_CONF */
            return sysconf(_SC_NPROCESSORS_CONF);
        } else {
            return (long)count;
        }
    }
#endif

#if __MP_LEGACY_SUPPORT_SYSCONF_WRAP_NEED_SC_NPROCESSORS_CONF__
    if ( name == _SC_NPROCESSORS_CONF ) {

        int nm[2];
        int ret;
        size_t len = 4;
        uint32_t count;

        nm[0] = CTL_HW; nm[1] = HW_NCPU;
        ret = sysctl(nm, 2, &count, &len, NULL, 0);

        /* there has to be at least 1 processor */
        if (ret < 0 || count < 1) { count = 1; }
        return (long)count;
    }
#endif

#if __MP_LEGACY_SUPPORT_SYSCONF_WRAP_NEED_SC_PHYS_PAGES__
    if ( name == _SC_PHYS_PAGES ) {

        /* the number of pages is the total memory / pagesize */
        uint64_t mem_size;
        size_t len = sizeof(mem_size);
        int pagesize = getpagesize();

        sysctlbyname("hw.memsize", &mem_size, &len, NULL, 0);

        return (long)(mem_size/pagesize);

    }
#endif

    /* for any other values of "name", call the real sysconf() */
    real_sysconf = dlsym(RTLD_NEXT, "sysconf");
    if (real_sysconf == NULL) {
        exit(EXIT_FAILURE);
    }
    return real_sysconf(name);
}

/* compatibility function so code does not have to be recompiled */
long macports_legacy_sysconf(int name) { return sysconf(name); }

#endif /*__MP_LEGACY_SUPPORT_SYSCONF_WRAP__*/
