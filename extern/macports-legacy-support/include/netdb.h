
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

#ifndef _MACPORTS_NETDB_H_
#define _MACPORTS_NETDB_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Include the primary system netdb.h */
#include_next <netdb.h>

#if __MP_LEGACY_SUPPORT_AI_NUMERICSERV__
#  ifndef AI_NUMERICSERV
#    define	AI_NUMERICSERV	0x00001000 /* prevent service name resolution */
#  endif
#endif

#endif /* _MACPORTS_NETDB_H_ */
