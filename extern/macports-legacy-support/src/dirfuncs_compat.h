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

/* Prototypes for directory function compatibility wrappers */

#ifndef __MP_LEGACY_SUPPORT_DIRFUNCS_COMPAT_H
#define __MP_LEGACY_SUPPORT_DIRFUNCS_COMPAT_H

#include <dirent.h>

DIR *
__mpls_opendir(const char *filename);

struct dirent *
__mpls_readdir(DIR *dirp);

int
__mpls_readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);

long
__mpls_telldir(DIR *dirp);

void
__mpls_seekdir(DIR *dirp, long loc);

void
__mpls_rewinddir(DIR *dirp);

int
__mpls_closedir(DIR *dirp);

int
__mpls_dirfd(DIR *dirp);

#endif /* __MP_LEGACY_SUPPORT_DIRFUNCS_COMPAT_H */
