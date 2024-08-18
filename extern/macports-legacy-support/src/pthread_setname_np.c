/*
Until such time as this behavior can be emulated, do nothing and return success.
* This is what the MacPorts SpiderMonkey does:
    https://github.com/macports/macports-ports/commit/50cdf084768436a421e2c6d05e995d122bc93bca
* This is what the MacPorts LLVM does:
    https://github.com/macports/macports-ports/blob/master/lang/llvm-14/files/0007-Threading-Only-call-pthread_setname_np-if-we-have-it.patch
* This is what upstream dav1d does:
    https://code.videolan.org/videolan/dav1d/-/blob/87f9a81cd770e49394a45deca7a3df41243de00b/src/thread.h#L182
    vs
    https://code.videolan.org/videolan/dav1d/-/blob/87f9a81cd770e49394a45deca7a3df41243de00b/src/thread.h#L182
* This is what upstream Rust does:
    https://github.com/rust-lang/rust/blob/100f12d17026fccfc5d80527b5976dd66b228b13/library/std/src/sys/unix/thread.rs#L137
    vs
    https://github.com/rust-lang/rust/blob/100f12d17026fccfc5d80527b5976dd66b228b13/library/std/src/sys/unix/thread.rs#L199
*/

/* MP support header */
#include "MacportsLegacySupport.h"
#if __MP_LEGACY_SUPPORT_PTHREAD_SETNAME_NP__

int pthread_setname_np(const char *name) {
  return 0;
}

#endif /* __MP_LEGACY_SUPPORT_PTHREAD_SETNAME_NP__ */
