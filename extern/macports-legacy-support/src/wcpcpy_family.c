
/*
 * Copyright (c) 2018 Christian Cornelssen
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

// MP support header
#include "MacportsLegacySupport.h"
#if __MP_LEGACY_SUPPORT_WCPCPY__

#include <wchar.h>

wchar_t *wcpncpy(wchar_t *__restrict d, const wchar_t *__restrict s, size_t n)
{
    wint_t c;
    while (n && (c = *s)) --n, *d++ = c, ++s;
    return wmemset(d, 0, n);
}

wchar_t *wcpcpy(wchar_t *__restrict d, const wchar_t *__restrict s)
{
    while ((*d = *s)) ++d, ++s;
    return d;
}

#endif /*  __MP_LEGACY_SUPPORT_WCPCPY__ */
