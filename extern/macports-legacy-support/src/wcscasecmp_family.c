
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
#if __MP_LEGACY_SUPPORT_WCSCASECMP__

#include <wchar.h>
#include <wctype.h>
#include <xlocale.h>

#include <limits.h>
#include <stdint.h>
#if WCHAR_MAX > INT_MAX
/* Caution: multiple argument evaluations */
#define _MP_WCDIFFSIGN(a, b)    (((b) < (a)) - ((a) < (b)))
#else
#define _MP_WCDIFFSIGN(a, b)    ((int)(a) - (int)(b))
#endif

int wcsncasecmp_l(const wchar_t *l, const wchar_t *r, size_t n, locale_t locale)
{
    wint_t lc, rc;
    int d = 0;
    while (!d && n) {
        lc = *l++; rc = *r++; --n;
        if (!(lc && rc)) n = 0;
        if (lc == rc) continue;
        lc = towlower_l(lc, locale);
        rc = towlower_l(rc, locale);
        d = _MP_WCDIFFSIGN(lc, rc);
    }
    return d;
}

int wcsncasecmp(const wchar_t *l, const wchar_t *r, size_t n)
{
    return wcsncasecmp_l(l, r, n, 0);
}

int wcscasecmp_l(const wchar_t *l, const wchar_t *r, locale_t locale)
{
    return wcsncasecmp_l(l, r, -1, locale);
}

int wcscasecmp(const wchar_t *l, const wchar_t *r)
{
    return wcsncasecmp_l(l, r, -1, 0);
}

#endif /* __MP_LEGACY_SUPPORT_WCSCASECMP__ */
