
/*
 * Copyright (c) 2018
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

#ifndef _MACPORTS_PTHREAD_H_
#define _MACPORTS_PTHREAD_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Include the primary system pthread.h */
#include_next <pthread.h>

#if __MP_LEGACY_SUPPORT_PTHREAD_RWLOCK__
/* PTHREAD_RWLOCK_INITIALIZER is not defined on Tiger */
#ifndef PTHREAD_RWLOCK_INITIALIZER
#define PTHREAD_RWLOCK_INITIALIZER {_PTHREAD_RWLOCK_SIG_init, {0}}
#endif
#endif /* __MP_LEGACY_SUPPORT_PTHREAD_RWLOCK__  */


__MP__BEGIN_DECLS

#if __MP_LEGACY_SUPPORT_PTHREAD_SETNAME_NP__
int pthread_setname_np(const char *);
#endif /* __MP_LEGACY_SUPPORT_PTHREAD_SETNAME_NP__ */

__MP__END_DECLS

#endif /* _MACPORTS_PTHREAD_H_ */
