
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

#ifndef _MACPORTS_LEGACYSUPPORTDEFS_H_
#define _MACPORTS_LEGACYSUPPORTDEFS_H_

/* Not needed -- #include "AvailabilityMacros.h" */

/* C++ extern definitions */
#if defined(__cplusplus)
#define	__MP__BEGIN_DECLS extern "C" {
#define	__MP__END_DECLS	  }
#else
#define	__MP__BEGIN_DECLS
#define	__MP__END_DECLS
#endif

/* foundational defs, used later */

#if defined(__i386)
#define __MP_LEGACY_SUPPORT_I386__            1
#else
#define __MP_LEGACY_SUPPORT_I386__            0
#endif

/* defines for when legacy support is required for various functions */

/* fsgetpath */
#define __MP_LEGACY_SUPPORT_FSGETPATH__       (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101300)

/* **setattrlistat */
#define __MP_LEGACY_SUPPORT_SETATTRLISTAT__   (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101300)

/* ** utimensat, futimens, UTIME_NOW, UTIME_OMIT */
#define __MP_LEGACY_SUPPORT_UTIMENSAT__       (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101300)

/* clock_gettime */
#define __MP_LEGACY_SUPPORT_GETTIME__         (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101200)

/* timespec_get */
#define __MP_LEGACY_SUPPORT_TIMESPEC_GET__    (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101500)

/* **at calls */
#define __MP_LEGACY_SUPPORT_ATCALLS__         (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101000)

/* fdopendir */
#define __MP_LEGACY_SUPPORT_FDOPENDIR__       (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101000)

/* this header is automatically included by <net/if.h> on systems 10.9 and up.
   It is therefore expected to be included by most current software. */
/* <net/if.h> include <sys/socket.h> */
#define __MP_LEGACY_SUPPORT_NETIF_SOCKET_FIX__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1090)

/* CMSG_DATA definition in <sys/socket.h> */
#define __MP_LEGACY_SUPPORT_CMSG_DATA_FIX__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1060)

/* stpncpy */
#define __MP_LEGACY_SUPPORT_STPNCPY__         (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* strnlen */
#define __MP_LEGACY_SUPPORT_STRNLEN__         (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* strndup */
#define __MP_LEGACY_SUPPORT_STRNDUP__         (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* dprintf */
#define __MP_LEGACY_SUPPORT_DPRINTF__         (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* getline */
#define __MP_LEGACY_SUPPORT_GETLINE__         (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* memmem */
#define __MP_LEGACY_SUPPORT_MEMMEM__          (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* wcsdup */
#define __MP_LEGACY_SUPPORT_WCSDUP__          (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* wcsnlen */
#define __MP_LEGACY_SUPPORT_WCSNLEN__         (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* wcpcpy, wcpncpy */
#define __MP_LEGACY_SUPPORT_WCPCPY__          (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* wcsncasecmp_l, wcscasecmp_l, wcsncasecmp, wcscasecmp */
#define __MP_LEGACY_SUPPORT_WCSCASECMP__      (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* llround */
#define __MP_LEGACY_SUPPORT_LLROUND__         (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* arc4random */
#define __MP_LEGACY_SUPPORT_ARC4RANDOM__      (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* getentropy */
#define __MP_LEGACY_SUPPORT_GETENTROPY__      (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101200)

/* posix_memalign does not exist on < 1060 */
#define __MP_LEGACY_SUPPORT_POSIX_MEMALIGN__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1060)

/* AI_NUMERICSERV does not exist on < 1060 */
#define __MP_LEGACY_SUPPORT_AI_NUMERICSERV__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1060)

/*  realpath() on < 1060 does not support modern NULL buffer usage */
#define __MP_LEGACY_SUPPORT_REALPATH_WRAP__   (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1060)

/* setattrlistat */
#define __MP_LEGACY_SUPPORT_FSETATTRLIST__    (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1060)

/* localtime_r, gmtime_r, etc only declared on Tiger when _ANSI_SOURCE and _POSIX_C_SOURCE are undefined */
#define __MP_LEGACY_SUPPORT_TIME_THREAD_SAFE_FUNCTIONS__     (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050)

/* lsmod does not exist on Tiger */
#define __MP_LEGACY_SUPPORT_LSMOD__           (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050)

/* lutimes does not exist on Tiger */
#define __MP_LEGACY_SUPPORT_LUTIMES__         (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050)

/* sys/aio.h header needs adjustment to match newer SDKs */
#define __MP_LEGACY_SUPPORT_SYSAIOTIGERFIX__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050)

/*  sysconf() is missing some functions on some systems, and may misbehave on i386 */
#define __MP_LEGACY_SUPPORT_SYSCONF_WRAP__    (__APPLE__ && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101100 \
                                                             || __MP_LEGACY_SUPPORT_I386__))

/* pthread_rwlock_initializer is not defined on Tiger */
#define __MP_LEGACY_SUPPORT_PTHREAD_RWLOCK__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050)

/* STAILQ_FOREACH is not defined on Tiger*/
#define __MP_LEGACY_SUPPORT_STAILQ_FOREACH__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050)

/* c++11 <cmath> PPC 10.[45] and Intel 10.[4-6], GNU g++ 4.6 through 8. */
#if (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070 \
               && defined(__GNUC__) && (__GNUC__ <= 8)                 \
               && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6))))
#define __MP_LEGACY_SUPPORT_CXX11_CMATH__ 1
#else
#define __MP_LEGACY_SUPPORT_CXX11_CMATH__ 0
#endif

/* cossin */
#define __MP_LEGACY_SUPPORT_COSSIN__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1090)

/* ffsl */
#define __MP_LEGACY_SUPPORT_FFSL__    (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050)
/* ffsll */
#define __MP_LEGACY_SUPPORT_FFSLL__   (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1090)

/* fls */
#define __MP_LEGACY_SUPPORT_FLS__     (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050)
/* flsl */
#define __MP_LEGACY_SUPPORT_FLSL__    (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1050)
/* flsll */
#define __MP_LEGACY_SUPPORT_FLSLL__   (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1090)

/* open_memstream */
#define __MP_LEGACY_SUPPORT_OPEN_MEMSTREAM__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101300)

/* fmemopen */
#define __MP_LEGACY_SUPPORT_FMEMOPEN__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101300)

/* pthread_setname_np */
#define __MP_LEGACY_SUPPORT_PTHREAD_SETNAME_NP__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1060)

/* Compound macros, bundling functionality needed by multiple single features. */
#define __MP_LEGACY_SUPPORT_NEED_ATCALL_MACROS__  (__MP_LEGACY_SUPPORT_ATCALLS__ || __MP_LEGACY_SUPPORT_SETATTRLISTAT__)

#define __MP_LEGACY_SUPPORT_NEED_BEST_FCHDIR__    (__MP_LEGACY_SUPPORT_FDOPENDIR__ || __MP_LEGACY_SUPPORT_ATCALLS__ || __MP_LEGACY_SUPPORT_SETATTRLISTAT__)

/* for now, just add missing typedef statements */
#define __MP_LEGACY_SUPPORT_UUID__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1060)

/* for now, just forward call to CFPropertyListCreateWithStream */
#define __MP_LEGACY_SUPPORT_CoreFoundation__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1060)

/* copyfile and its associated functions have gained functionality over the years */
#define __MP_LEGACY_SUPPORT_COPYFILE_WRAP__ (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1060)

/* _tlv_atexit and __cxa_thread_atexit */
#define __MP_LEGACY_SUPPORT_ATEXIT_WRAP__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

/* os_unfair_lock structure and its associated functions */
#define __MP_LEGACY_SUPPORT_OS_UNFAIR_LOCK__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 101200)

/* library symbol ___bzero */
#define __MP_LEGACY_SUPPORT_SYMBOL____bzero__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1060)

/* library symbol _dirfd */
#define __MP_LEGACY_SUPPORT_SYMBOL__dirfd__  (__APPLE__ && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1080)

#endif /* _MACPORTS_LEGACYSUPPORTDEFS_H_ */
