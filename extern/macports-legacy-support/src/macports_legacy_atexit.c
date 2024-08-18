/*
 * Copyright (c) 2022
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
#include "MacportsLegacySupport.h"

/* _tlv_atexit wrap */
#if __MP_LEGACY_SUPPORT_ATEXIT_WRAP__

#include <stdlib.h>
#include <dlfcn.h>

/* signature from https://opensource.apple.com/source/dyld/dyld-852.2/src/threadLocalVariables.c.auto.html */
void _tlv_atexit(void (*func)(void*), void* objAddr) {
    void (*real__tlv_atexit)(void (*func)(void*), void* objAddr);
    int (*real___cxa_thread_atexit)(void (*dtor)(void*), void* obj, void* dso_symbol);

    real__tlv_atexit = dlsym(RTLD_NEXT, "_tlv_atexit");
    if (real__tlv_atexit != NULL) {
        /* _tlv_atexit exists in libSystem.B.dylib on 10.7 and later */
        real__tlv_atexit(func, objAddr);
	return;
    }

    real___cxa_thread_atexit = dlsym(RTLD_DEFAULT, "__cxa_thread_atexit");
    if (real___cxa_thread_atexit != NULL) {
        /* __cxa_thread_atexit exists in
               MacPorts provided libc++ on 10.6 and earlier
               libSystem.B.dylib on 10.8 and later
               MacPorts provided libstdc++

           in all cases, third parameter (dso_handle) seems to be ignored
        */
	real___cxa_thread_atexit(func, objAddr, NULL);
	return;
    }

    exit(EXIT_FAILURE);
}

/* signature from https://github.com/llvm-mirror/libcxxabi/blob/master/src/cxa_thread_atexit.cpp */
int __cxa_thread_atexit(void (*dtor)(void*), void* obj, void* dso_symbol) {
    void (*real__tlv_atexit)(void (*func)(void*), void* objAddr);
    int (*real___cxa_thread_atexit)(void (*dtor)(void*), void* obj, void* dso_symbol);

    real___cxa_thread_atexit = dlsym(RTLD_NEXT, "__cxa_thread_atexit");
    if (real___cxa_thread_atexit != NULL) {
        /* __cxa_thread_atexit exists in
               MacPorts provided libc++ on 10.6 and earlier
               libSystem.B.dylib on 10.8 and later
               MacPorts provided libstdc++
	*/
        return real___cxa_thread_atexit(dtor, obj, dso_symbol);
    }

   real__tlv_atexit = dlsym(RTLD_DEFAULT, "_tlv_atexit");
    if (real__tlv_atexit != NULL) {
        /* _tlv_atexit exists in libSystem.B.dylib on 10.7 and later

           it seems to be common practice to ignore dso_symbol
        */
        real__tlv_atexit(dtor, obj);
	return 0;
    }

    exit(EXIT_FAILURE);
}

#endif /*__MP_LEGACY_SUPPORT_ATEXIT_WRAP__*/
