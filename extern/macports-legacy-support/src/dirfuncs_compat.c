/*
 * Copyright (c) 2024 Frederick H. G. Wright II <fw@fwright.net>
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

/*
 * Earlier versions of legacy-support needed wrappers around several OS
 * calls related to directories, in order to implement fdopendir().  That
 * is no longer the case, but existing dependents still reference those
 * wrapper calls.  For compatibility, we continue to provide those functions,
 * but just as transparent wrappers around the OS calls.
 *
 * These wrappers can eventually be removed once all dependents have been
 * rebuilt with the current headers.  But since there would be significant
 * work in determining when this is the case, and since they only add 752
 * bytes to the library size, they should probably be left in place for a
 * long time.
 *
 * This is only relevant for OS versions where our fdopendir() is needed,
 * hence the conditional (which is the same conditional as was used for
 * the earlier implementations).
 */

/* MP support header */
#include "MacportsLegacySupport.h"
#if __MP_LEGACY_SUPPORT_FDOPENDIR__

#include "dirfuncs_compat.h"

DIR *
__mpls_opendir(const char *filename) {
    return opendir(filename);
}

struct dirent *
__mpls_readdir(DIR *dirp) {
    return readdir(dirp);
}

int
__mpls_readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result) {
    return readdir_r(dirp, entry, result);
}

long
__mpls_telldir(DIR *dirp) {
    return telldir(dirp);
}

void
__mpls_seekdir(DIR *dirp, long loc) {
    seekdir(dirp, loc);
}

void
__mpls_rewinddir(DIR *dirp) {
    rewinddir(dirp);
}

int
__mpls_closedir(DIR *dirp) {
    return closedir(dirp);
}

int
__mpls_dirfd(DIR *dirp) {
    return dirfd(dirp);
}

#endif /* __MP_LEGACY_SUPPORT_FDOPENDIR__ */
