/*
 * Copyright (c) 2018 Chris Jones <jonesc@macports.org>
 * Copyright (c) 2019 Michael Dickens <michaelld@macports.org>
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

#ifndef _MACPORTS_MATH_H_
#define _MACPORTS_MATH_H_

/* MP support header */
#include "MacportsLegacySupport.h"

#if __MP_LEGACY_SUPPORT_LLROUND__

__MP__BEGIN_DECLS

/*
 * These functions are present in the system math library but their
 * prototypes might not be declared under some circumstances. Declare
 * them here anyway.
 */

/*
 * this is the same condition that defines the function prototypes in
 * the GCC <math.h>.
 */
#if !(__DARWIN_NO_LONG_LONG)
extern long long int llrint   ( double );
extern long long int llrintf  ( float );
extern long long int llrintl  ( long double );

extern long long int llround  ( double );
extern long long int llroundf ( float );
extern long long int llroundl ( long double );
#endif

__MP__END_DECLS

/*
 * If the GCC <math.h> header exists, then tell it: (1) to include the
 * next <math.h>, which should be from the system; and (2) to not use
 * it's <math.h> yet, because it basically wraps <cmath> and we need
 * to keep everything herein focused on just <math.h>. If the user
 * wants <cmath>, they should #include that specific header.
 */

#undef L_GLIBCXX_MATH_H
#ifndef _GLIBCXX_MATH_H
#define L_GLIBCXX_MATH_H 1
#define _GLIBCXX_MATH_H 1
#endif

#undef L_GLIBCXX_INCLUDE_NEXT_C_HEADERS
#ifndef _GLIBCXX_INCLUDE_NEXT_C_HEADERS
#define L_GLIBCXX_INCLUDE_NEXT_C_HEADERS 1
#define _GLIBCXX_INCLUDE_NEXT_C_HEADERS 1
#endif

#endif /* __MP_LEGACY_SUPPORT_LLROUND__ */

/*
 * Include the next math.h, which might be from the primary system or
 * it might be within GCC's c or c++ (yup!) headers
 */

#include_next <math.h>

#if __MP_LEGACY_SUPPORT_COSSIN__

/* Following is borrowed from math.h on macOS 10.9+ */

/*  __sincos and __sincosf were introduced in OSX 10.9 and iOS 7.0.  When
    targeting an older system, we simply split them up into discrete calls
    to sin( ) and cos( ).  */

__MP__BEGIN_DECLS
extern void __sincosf(float __x, float *__sinp, float *__cosp);
extern void __sincos(double __x, double *__sinp, double *__cosp);
__MP__END_DECLS

#endif /* __MP_LEGACY_SUPPORT_COSSIN__ */

#if __MP_LEGACY_SUPPORT_LLROUND__

#ifdef L_GLIBCXX_MATH_H
#undef L_GLIBCXX_MATH_H
#undef _GLIBCXX_MATH_H
#endif

#ifdef L_GLIBCXX_INCLUDE_NEXT_C_HEADERS
#undef L_GLIBCXX_INCLUDE_NEXT_C_HEADERS
#undef _GLIBCXX_INCLUDE_NEXT_C_HEADERS
#endif

#endif /* __MP_LEGACY_SUPPORT_LLROUND__ */

#endif /* _MACPORTS_MATH_H_ */
