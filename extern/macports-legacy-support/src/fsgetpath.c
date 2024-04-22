/*
 * Copyright (c) 2019
 * from an example posted in Apple Developer Support
 * https://forums.developer.apple.com/thread/103162
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
#if __MP_LEGACY_SUPPORT_FSGETPATH__


#if 1
/* SYS_fsgetpath is only available on 10.6 and up */
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1060
/* implement using a syscall available macOS 10.6 to 10.12 */
/* this should be thoroughly vetted as a syscall, but is private API */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/mount.h>
ssize_t fsgetpath(char * buf, size_t buflen, fsid_t * fsid, uint64_t obj_id) {
    return (ssize_t)syscall(SYS_fsgetpath, buf, (size_t)buflen, fsid, (uint64_t)obj_id);
}
#endif
#endif

#if 0
/* implement with a compatability function that presently compiles on 10.6 and over */
/* this may be better (see linked post above) but it's hard to thoroughly test it. */
/* this may also be able to be expanded to cover 10.4 and 10.5 if we can workaround ATTR_CMN_FULLPATH */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <sys/attr.h>
#include <sys/mount.h>

ssize_t fsgetpath(char * buf, size_t buflen, fsid_t * fsid, uint64_t obj_id) {
    char volfsPath[64];  // 8 for `/.vol//\0`, 10 for `fsid->val[0]`, 20 for `obj_id`, rounded up for paranoia

    snprintf(volfsPath, sizeof(volfsPath), "/.vol/%ld/%llu", (long) fsid->val[0], (unsigned long long) obj_id);

    struct {
        uint32_t            length;
        attrreference_t     pathRef;
        char                buffer[MAXPATHLEN];
    } __attribute__((aligned(4), packed)) attrBuf;

    struct attrlist attrList;
    memset(&attrList, 0, sizeof(attrList));
    attrList.bitmapcount = ATTR_BIT_MAP_COUNT;
    attrList.commonattr = ATTR_CMN_FULLPATH;

    int success = getattrlist(volfsPath, &attrList, &attrBuf, sizeof(attrBuf), 0) == 0;
    if ( ! success ) {
        return -1;
    }
    if (attrBuf.pathRef.attr_length > buflen) {
        errno = ENOSPC;
        return -1;
    }
    strlcpy(buf, ((const char *) &attrBuf.pathRef) + attrBuf.pathRef.attr_dataoffset, buflen);
    return attrBuf.pathRef.attr_length;
}
#endif

#endif /* __MP_LEGACY_SUPPORT_FSGETPATH__ */
