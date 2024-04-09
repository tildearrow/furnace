/*
 * Copyright (c) 2024
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

#if __MP_LEGACY_SUPPORT_SYMBOL____bzero__
#include <strings.h>
/*
The need for this function is highly limited.
The symbol `__bzero` does not exist prior to 10.6.
When the Rust stage0 compiler for 10.5 is built on newer machines, it bakes `__bzero` into librustc_driver-xxx.dylib.
This may be due to the fact that on newer machines, the `_bzero` symbol is an indirect reference to another symbol.
*/
void __bzero(void *s, size_t n) { bzero(s, n); }
#endif /* __MP_LEGACY_SUPPORT_SYMBOL_ALIASES__ */

#if __MP_LEGACY_SUPPORT_SYMBOL__dirfd__
#include <dirent.h>
#include <errno.h>
#include <stddef.h>
/*
The need for this function is highly limited.
Prior to 10.8, `dirfd` was a macro`.
The Rust compiler requires `dirfd` to be a library symbol.
*/
#undef dirfd
#if __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050
#define __dd_fd dd_fd
#endif /* __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050 */
int dirfd(DIR *dirp)
{
    if (dirp == NULL || dirp->__dd_fd < 0)
    {
        errno = EINVAL;
        return -1;
    }
    else
        return dirp->__dd_fd;
}
#endif /* __MP_LEGACY_SUPPORT_SYMBOL__dirfd__ */

#if __MP_LEGACY_SUPPORT_ATCALLS__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 1050
#include <sys/stat.h>
/*
The need for this function is highly limited.
The Rust compiler requires `fstatat$INODE64` to be a library symbol.
*/
int fstatat$INODE64(int dirfd, const char *pathname, struct stat64 *buf, int flags) { return fstatat64(dirfd, pathname, buf, flags); }
#endif /* __MP_LEGACY_SUPPORT_ATCALLS__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 1050 */
