
/*
 * Copyright (c) 2018 Chris Jones <jonesc@macports.org>
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

#ifndef _MACPORTS_SYSFCNTL_H_
#define _MACPORTS_SYSFCNTL_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Include the primary system fcntl.h */
#include_next <sys/fcntl.h>

/* replace missing O_CLOEXEC definition with 0, which works
 * but does not replace the full function of that flag
 * this is the commonly done fix in MacPorts (see gtk3, for example)
 * FIXME - this could use a proper fix, if possible
 */

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

/* atcalls */
#if __MP_LEGACY_SUPPORT_ATCALLS__

#ifndef AT_FDCWD
#define AT_FDCWD		-2
#endif
#ifndef AT_EACCESS
#define AT_EACCESS		0x0010	/* Use effective ids in access check */
#endif
#ifndef AT_SYMLINK_NOFOLLOW
#define AT_SYMLINK_NOFOLLOW	0x0020	/* Act on the symlink itself not the target */
#endif
#ifndef AT_SYMLINK_FOLLOW
#define AT_SYMLINK_FOLLOW	0x0040	/* Act on target of symlink */
#endif
#ifndef AT_REMOVEDIR
#define AT_REMOVEDIR		0x0080	/* Path refers to directory */
#endif

__MP__BEGIN_DECLS

extern int openat(int dirfd, const char *pathname, int flags, ...);

__MP__END_DECLS

#endif /* __MP_LEGACY_SUPPORT_ATCALLS__ */

#endif /* _MACPORTS_SYSFCNTL_H_ */
