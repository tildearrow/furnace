
/*
 * Copyright (c) 2022
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

#ifndef _MACPORTS_CoreFoundationCoreFoundation_H_
#define _MACPORTS_CoreFoundationCoreFoundation_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Include the primary system CoreFoundation/CoreFoundation.h */
#include_next <CoreFoundation/CoreFoundation.h>

#if __MP_LEGACY_SUPPORT_CoreFoundation__

#define CFPropertyListCreateWithStream(A,B,C,D,E,F) CFPropertyListCreateFromStream(A,B,C,D,E,F)

#endif /* __MP_LEGACY_SUPPORT_CoreFoundation__ */

#endif /* _MACPORTS_CoreFoundationCoreFoundation_H_ */
