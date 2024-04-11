
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

#ifndef _MACPORTS_STDLIB_H_
#define _MACPORTS_STDLIB_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Include the primary system stdlib.h */
#include_next <stdlib.h>

/* posix_memalign */
#if __MP_LEGACY_SUPPORT_POSIX_MEMALIGN__

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

__MP__BEGIN_DECLS
extern int posix_memalign(void **memptr, size_t alignment, size_t size);
__MP__END_DECLS

#endif /*  __MP_LEGACY_SUPPORT_POSIX_MEMALIGN__ */

/* arc4random */
#if __MP_LEGACY_SUPPORT_ARC4RANDOM__

#ifndef _SIZE_T
#define _SIZE_T
typedef __darwin_size_t		size_t;
#endif
#ifndef _UINT32_T
#define _UINT32_T
typedef unsigned int         uint32_t;
#endif

/*
 * Generate and return a uniformly random 32-bit quantity with an
 * upper bound of 'upper_bound'
 */
__MP__BEGIN_DECLS
extern uint32_t arc4random_uniform( uint32_t upper_bound );
__MP__END_DECLS

/*
 * Generate 'n' random bytes and put them in 'buf'.
 */
__MP__BEGIN_DECLS
extern void arc4random_buf( void* buf, size_t n );
__MP__END_DECLS

#endif /*  __MP_LEGACY_SUPPORT_ARC4RANDOM__ */

#endif /* _MACPORTS_STDLIB_H_ */
