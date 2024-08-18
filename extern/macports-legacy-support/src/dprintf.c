/*
 * Copyright (c) 2021, Evan Miller   <emmiller@gmail.com>
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
#if __MP_LEGACY_SUPPORT_DPRINTF__

#include <stdarg.h>
#include <stdio.h>
#include <sys/errno.h>

int dprintf(int fildes, const char * __restrict format, ...) {
    FILE *stream = fdopen(fildes, "w");
    if (stream == NULL) {
        errno = EBADF;
        return -1;
    }
    va_list ap;
    va_start(ap, format);
    int result = vfprintf(stream, format, ap);
    va_end(ap);
    fclose(stream);
    return result;
}

#endif /* __MP_LEGACY_SUPPORT_DPRINTF__ */
