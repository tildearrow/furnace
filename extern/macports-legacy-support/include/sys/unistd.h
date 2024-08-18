
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

#ifndef _MACPORTS_SYSUNISTD_H_
#define _MACPORTS_SYSUNISTD_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Include the primary system unistd.h */
#include_next <sys/unistd.h>

/* For types such as uint32_t. */
#include <stdint.h>

#if __MP_LEGACY_SUPPORT_ATCALLS__

/*
 * [XSI] The ssize_t and size_t types shall be defined as described
 * in <sys/types.h>.
 */
#ifndef _SIZE_T
#define _SIZE_T
typedef __darwin_size_t		size_t;
#endif

#ifndef	_SSIZE_T
#define	_SSIZE_T
typedef	__darwin_ssize_t	ssize_t;
#endif

#ifndef _UID_T
#define _UID_T
typedef __darwin_uid_t		uid_t;	/* user id */
#endif

#ifndef _GID_T
#define _GID_T
typedef __darwin_gid_t		gid_t;
#endif

__MP__BEGIN_DECLS

extern int getattrlistat(int dirfd, const char *pathname, void *a,
			 void *buf, size_t size, unsigned long flags);
extern ssize_t readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz);
extern int faccessat(int dirfd, const char *pathname, int mode, int flags);
extern int fchownat(int dirfd, const char *pathname, uid_t owner, gid_t group, int flags);
extern int linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags);
extern int symlinkat(const char *oldpath, int newdirfd, const char *newpath);
extern int unlinkat(int dirfd, const char *pathname, int flags);

__MP__END_DECLS

#endif /* __MP_LEGACY_SUPPORT_ATCALLS__ */


#if __MP_LEGACY_SUPPORT_SETATTRLISTAT__

/*
 * [XSI] The ssize_t and size_t types shall be defined as described
 * in <sys/types.h>.
 */
#ifndef _SIZE_T
#define _SIZE_T
typedef __darwin_size_t		size_t;
#endif


__MP__BEGIN_DECLS

extern int setattrlistat(int dirfd, const char *pathname, void *a,
			 void *buf, size_t size, uint32_t flags);

__MP__END_DECLS

#endif /* __MP_LEGACY_SUPPORT_SETATTRLISTAT__ */

#endif /* _MACPORTS_SYSUNISTD_H_ */
