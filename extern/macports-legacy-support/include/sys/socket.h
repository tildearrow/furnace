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

#ifndef _MACPORTS_SYS_SOCKET_H_
#define _MACPORTS_SYS_SOCKET_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Include the primary system sys/socket.h */
#include_next <sys/socket.h>

/*
 * OSX prior to 10.6 defines CMSG_DATA without properly considering 64-bit
 * builds, due to bad alignment assumptions, though it happens to work in
 * the 10.4 case and only actually fails in the 10.5 64-bit case.
 *
 * In those OS versions we substitute a version of the definition from 10.6.
 */

#if __MP_LEGACY_SUPPORT_CMSG_DATA_FIX__

#define	__DARWIN_ALIGNBYTES32 (sizeof(__uint32_t) - 1)
#define	__DARWIN_ALIGN32(p) \
	((size_t)((char *)(size_t)(p) \
	 + __DARWIN_ALIGNBYTES32) &~ __DARWIN_ALIGNBYTES32)

/* given pointer to struct cmsghdr, return pointer to data */
#undef CMSG_DATA
#define	CMSG_DATA(cmsg) ((unsigned char *)(cmsg) + \
	__DARWIN_ALIGN32(sizeof(struct cmsghdr)))

#endif /* __MP_LEGACY_SUPPORT_CMSG_DATA_FIX__ */

#endif /* _MACPORTS_SYS_SOCKET_H_ */
