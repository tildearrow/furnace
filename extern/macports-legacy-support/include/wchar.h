
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

#ifndef _MACPORTS_WCHAR_H_
#define _MACPORTS_WCHAR_H_

/* MP support header */
#include "MacportsLegacySupport.h"

/* Include the primary system wchar.h */
#include_next <wchar.h>

__MP__BEGIN_DECLS

/* wcsdup */
#if __MP_LEGACY_SUPPORT_WCSDUP__
  extern wchar_t * wcsdup(const wchar_t *s);
#endif

/* wcsnlen */
#if __MP_LEGACY_SUPPORT_WCSNLEN__
  extern size_t wcsnlen(const wchar_t *, size_t);
#endif

/* wcpcpy, wcpncpy */
#if __MP_LEGACY_SUPPORT_WCPCPY__
  extern wchar_t * wcpcpy(wchar_t *__restrict d, const wchar_t *__restrict s);
  extern wchar_t *wcpncpy(wchar_t *__restrict d, const wchar_t *__restrict s, size_t n);
#endif

/* wcsncasecmp, wcscasecmp */
#if __MP_LEGACY_SUPPORT_WCSCASECMP__
  extern int  wcscasecmp(const wchar_t *l, const wchar_t *r);
  extern int wcsncasecmp(const wchar_t *l, const wchar_t *r, size_t n);
#endif

__MP__END_DECLS

#endif /* _MACPORTS_WCHAR_H_ */
