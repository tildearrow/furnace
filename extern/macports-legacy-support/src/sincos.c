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

/* MP support header */
#include <math.h>

#if __MP_LEGACY_SUPPORT_COSSIN__

/* Following is borrowed from math.h on macOS 10.9+ */

/*  __sincos and __sincosf were introduced in OSX 10.9 and iOS 7.0.  When
    targeting an older system, we simply split them up into discrete calls
    to sin( ) and cos( ).  */
void __sincosf(float __x, float *__sinp, float *__cosp) {
  *__sinp = sinf(__x);
  *__cosp = cosf(__x);
}
void __sincos(double __x, double *__sinp, double *__cosp) {
  *__sinp = sin(__x);
  *__cosp = cos(__x);
}

#endif /* __MP_LEGACY_SUPPORT_COSSIN__ */
