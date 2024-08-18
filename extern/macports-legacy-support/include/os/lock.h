/*
 * Copyright (c) 2023
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

#ifndef _MACPORTS_LOCK_H_
#define _MACPORTS_LOCK_H_

/* MP support header */
#include "MacportsLegacySupport.h"

#if __MP_LEGACY_SUPPORT_OS_UNFAIR_LOCK__

/*
  os/lock.h does not exist
  use deprecated OSSpinLock instead

  see https://developer.apple.com/documentation/os/os_unfair_lock
*/

#include <libkern/OSAtomic.h>

#define OS_UNFAIR_LOCK_INIT OS_SPINLOCK_INIT

typedef OSSpinLock os_unfair_lock;
typedef OSSpinLock *os_unfair_lock_t;

__MP__BEGIN_DECLS

void os_unfair_lock_lock(os_unfair_lock_t lock);

bool os_unfair_lock_trylock(os_unfair_lock_t lock);

void os_unfair_lock_unlock(os_unfair_lock_t lock);

__MP__END_DECLS

/*
it is not clear how to implement these functions

void os_unfair_lock_assert_owner(const os_unfair_lock *lock);
void os_unfair_lock_assert_not_owner(const os_unfair_lock *lock);
*/

#else /*__MP_LEGACY_SUPPORT_OS_UNFAIR_LOCK__*/

/* Include the primary system os/lock.h */
#include_next <os/lock.h>

#endif /*__MP_LEGACY_SUPPORT_OS_UNFAIR_LOCK__*/

#endif
