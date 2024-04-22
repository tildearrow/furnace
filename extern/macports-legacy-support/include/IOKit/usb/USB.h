/*
 * Copyright (c) 2021 Evan Miller <emmiller@gmail.com>
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

/* pack(1) and align=reset don't mix in some versions of GCC.
 * See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=50909 */
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && !defined(__clang__)
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5)
/* 10.4 and up need three invocations */
#pragma options align=power
#pragma options align=power
#pragma options align=power
/* 10.8 and up need two more */
#pragma options align=power
#pragma options align=power
/* 10.15 and up need none (extras won't hurt so SDK branching is unimplemented) */
#endif
#endif

#include_next <IOKit/usb/USB.h>
