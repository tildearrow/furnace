
/*
 * Copyright (c) 2020
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

#ifndef _MACPORTS_SYS_SPAWN_H_
#define _MACPORTS_SYS_SPAWN_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* spawn.h exists on Leopard or later. Use this block method at present
 * because gcc versions < 5 don't recognize __have_include()
 */
#if (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 1050)

/* Include the primary system sys/queue.h */
#include_next <sys/spawn.h>

/* replace if missing */
#ifndef POSIX_SPAWN_CLOEXEC_DEFAULT
#define	POSIX_SPAWN_CLOEXEC_DEFAULT	0x4000
#endif

#endif

#endif /* _MACPORTS_SYS_SPAWN_H_ */
