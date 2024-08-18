
/*
 * Copyright (c) 2020
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

#ifndef _MACPORTS_SYS_AIO_H_
#define _MACPORTS_SYS_AIO_H_

/* MP support header */
#include "MacportsLegacySupport.h"


/*  in the MacOSX10.4.sdk, several definitions are missing from this file that are included
    in later SDKs. This fix includes the headers in the order used in this file
    later SDKs, and adds the two missing definitions prior to calling the
    usual header.
    
    The alternate method here would be to copy in the header from the 
    MacOSX10.4.sdk and patch it in place, as is done in gcc. We may do that
    in the end, if it turns out to be less intrusive.
    
*/

#if __MP_LEGACY_SUPPORT_SYSAIOTIGERFIX__

#include <sys/signal.h>
#include <sys/_types.h>
#include <sys/cdefs.h>
 
#ifndef _OFF_T
typedef __darwin_off_t  off_t;
#define _OFF_T
#endif

#ifndef _SSIZE_T
#define _SSIZE_T
typedef __darwin_ssize_t    ssize_t;
#endif

#endif /* __MP_LEGACY_SUPPORT_SYSAIOTIGERFIX__ */

/* Include the primary system sys/aio.h */
#include_next <sys/aio.h>

#endif /* _MACPORTS_SYS_AIO_H_ */
