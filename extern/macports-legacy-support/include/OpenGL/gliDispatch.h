/*
 * Copyright (c) 2021
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


/* in SDKs > 10.6 this Apple header does not include glext.h.
 * Including it causes redefinition errors that are hard to
 * overcome in ports, eg mesa, so we block the loading of
 * glext.h here on older systems for consistent behaviour with newer systems
 * 
 * Note: this header has no specific blocker as it may be called
 * multiple times and should have the same effect each time
 */

#if MAC_OS_X_VERSION_MAX_ALLOWED < 1070
#  ifdef __glext_h_
#    define MACPORTS_LEGACY_SAVED_GLEXT_SET
#  else
#    define __glext_h_
#  endif
#endif

#include_next <OpenGL/gliDispatch.h>

#if MAC_OS_X_VERSION_MAX_ALLOWED < 1070
#  ifdef MACPORTS_LEGACY_SAVED_GLEXT_SET
#    undef MACPORTS_LEGACY_SAVED_GLEXT_SET
#  else
#    undef __glext_h_
#  endif
#endif
