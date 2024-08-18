/*
 * Copyright (c) 2021 Mihai Moldovan <ionic@ionic.de>
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
#if __MP_LEGACY_SUPPORT_FSETATTRLIST__

#include <sys/attr.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <sys/errno.h>

#include <fcntl.h>
#include <string.h>
#include <unistd.h>


#ifdef __LP64__
int fsetattrlist(int fd, void *a, void *buf, size_t size, unsigned int flags)
#else /* defined (__LP64__) */
int fsetattrlist(int fd, void *a, void *buf, size_t size, unsigned long flags)
#endif /* defined (__LP64__) */
{
    int cont = 1,
        ret = 0;

    char fpath[MAXPATHLEN];
    memset (fpath, 0, MAXPATHLEN);
    if (-1 == fcntl(fd, F_GETPATH, fpath)) {
        ret = EBADF;
        cont = 0;
    }

#if __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1080
    /*
     * Older systems don't correctly check if no attributes are to be set, which usually
     * means a buffer size of zero and return an error since they malloc a block of
     * memory with size zero, leading to ENOMEM.
     *
     * Emulate the fix from 10.8 for those.
     */
    const struct attrlist *al = a;
    if (al->commonattr == 0 &&
        (al->volattr & ~ATTR_VOL_INFO) == 0 &&
        al->dirattr == 0 &&
        al->fileattr == 0 &&
        al->forkattr == 0) {
        cont = 0;

        /*
         * Explicitly let the potential error from above pass through, since that's what
         * the original function seems to do as well.
         */
    }
#endif

    if (cont) {
        ret = setattrlist(fpath, a, buf, size, flags);
    }

    return ret;
}

#endif  /* __MP_LEGACY_SUPPORT_FSETATTRLIST__ */
