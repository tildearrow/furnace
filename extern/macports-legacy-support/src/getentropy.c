
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

// MP support header
#include "MacportsLegacySupport.h"
#if __MP_LEGACY_SUPPORT_GETENTROPY__

#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/random.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

void
_error(int doexit, int err, const char* fmt, ...)
{
    va_list ap;

    fflush(stdout);
    fflush(stderr);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    if (err > 0)
        fprintf(stderr, "\n  %s (Errno %d)\n", strerror(err), err);

    if (doexit) {
        fflush(stderr);
        exit(1);
    }
}

static int
_randopen(const char* name)
{
    int fd = open(name, O_RDONLY);
    if (fd < 0) {
      _error(1, errno, "Cannot open system random number dev %s", name);
    }

    return fd;
}

int
getentropy(void* buf, size_t n)
{
  
    static int fd = -1;
    uint8_t* b    = (uint8_t*)buf;

    if (fd < 0)
        fd = _randopen("/dev/urandom");

    while (n > 0)
    {
        ssize_t m = (read)(fd, b, n);

        if (m < 0) {
            if (errno == EINTR) continue;
            _error(1, errno, "Fatal read error while reading rand dev");
        }
        b += m;
        n -= m;
    }

    return 0;
}

#endif /* __MP_LEGACY_SUPPORT_GETENTROPY__ */
