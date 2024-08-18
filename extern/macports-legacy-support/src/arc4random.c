/*
 * Copyright (c) 1996, David Mazieres <dm@uun.org>
 * Copyright (c) 2008, Damien Miller <djm@openbsd.org>
 * Copyright (c) 2013, Markus Friedl <markus@openbsd.org>
 * Copyright (c) 2014, Theo de Raadt <deraadt@openbsd.org>
 * Copyright (c) 2015, Sudhi Herle   <sudhi@herle.net>
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
#if __MP_LEGACY_SUPPORT_ARC4RANDOM__

/*
 * ChaCha based random number generator from OpenBSD.
 *
 * Made fully portable and thread-safe by Sudhi Herle.
 */

#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/random.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>


#define ARC4R_KEYSZ     32
#define ARC4R_IVSZ      8
#define ARC4R_BLOCKSZ   64
#define ARC4R_RSBUFSZ   (16*ARC4R_BLOCKSZ)

typedef struct
{
    uint32_t input[16]; /* could be compressed */
} chacha_ctx;

struct rand_state
{
    size_t          rs_have;    /* valid bytes at end of rs_buf */
    size_t          rs_count;   /* bytes till reseed */
    pid_t           rs_pid;     /* My PID */
    chacha_ctx      rs_chacha;  /* chacha context for random keystream */
    u_char          rs_buf[ARC4R_RSBUFSZ];  /* keystream blocks */
};
typedef struct rand_state rand_state;

#define KEYSTREAM_ONLY

typedef unsigned char u8;
typedef uint32_t      u32;


#define U8C(v) (v##U)
#define U32C(v) (v##U)

#define U8V(v) ((u8)(v) & U8C(0xFF))
#define U32V(v) ((u32)(v) & U32C(0xFFFFFFFF))

#define ROTL32(v, n) \
  (U32V((v) << (n)) | ((v) >> (32 - (n))))

#define U8TO32_LITTLE(p) \
  (((u32)((p)[0])      ) | \
   ((u32)((p)[1]) <<  8) | \
   ((u32)((p)[2]) << 16) | \
   ((u32)((p)[3]) << 24))

#define U32TO8_LITTLE(p, v) \
  do { \
    (p)[0] = U8V((v)      ); \
    (p)[1] = U8V((v) >>  8); \
    (p)[2] = U8V((v) >> 16); \
    (p)[3] = U8V((v) >> 24); \
  } while (0)

#define ROTATE(v,c) (ROTL32(v,c))
#define XOR(v,w) ((v) ^ (w))
#define PLUS(v,w) (U32V((v) + (w)))
#define PLUSONE(v) (PLUS((v),1))

#define QUARTERROUND(a,b,c,d) \
  a = PLUS(a,b); d = ROTATE(XOR(d,a),16); \
  c = PLUS(c,d); b = ROTATE(XOR(b,c),12); \
  a = PLUS(a,b); d = ROTATE(XOR(d,a), 8); \
  c = PLUS(c,d); b = ROTATE(XOR(b,c), 7);

static const char sigma[16] = "expand 32-byte k";
static const char tau[16] = "expand 16-byte k";

static void
_chacha_keysetup(chacha_ctx *x,const u8 *k,u32 kbits,u32 ivbits)
{
    const char *constants;

    (void)ivbits;

    x->input[4] = U8TO32_LITTLE(k + 0);
    x->input[5] = U8TO32_LITTLE(k + 4);
    x->input[6] = U8TO32_LITTLE(k + 8);
    x->input[7] = U8TO32_LITTLE(k + 12);
    if (kbits == 256) { /* recommended */
        k += 16;
        constants = sigma;
    } else { /* kbits == 128 */
        constants = tau;
    }
    x->input[8] = U8TO32_LITTLE(k + 0);
    x->input[9] = U8TO32_LITTLE(k + 4);
    x->input[10] = U8TO32_LITTLE(k + 8);
    x->input[11] = U8TO32_LITTLE(k + 12);
    x->input[0] = U8TO32_LITTLE(constants + 0);
    x->input[1] = U8TO32_LITTLE(constants + 4);
    x->input[2] = U8TO32_LITTLE(constants + 8);
    x->input[3] = U8TO32_LITTLE(constants + 12);
}

static void
_chacha_ivsetup(chacha_ctx *x,const u8 *iv)
{
  x->input[12] = 0;
  x->input[13] = 0;
  x->input[14] = U8TO32_LITTLE(iv + 0);
  x->input[15] = U8TO32_LITTLE(iv + 4);
}

static void
_chacha_encrypt_bytes(chacha_ctx *x,const u8 *m,u8 *c,u32 bytes)
{
  u32 x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15;
  u32 j0, j1, j2, j3, j4, j5, j6, j7, j8, j9, j10, j11, j12, j13, j14, j15;
  u8 *ctarget = NULL;
  u8 tmp[64];
  u_int i;

  if (!bytes) return;

  j0 = x->input[0];
  j1 = x->input[1];
  j2 = x->input[2];
  j3 = x->input[3];
  j4 = x->input[4];
  j5 = x->input[5];
  j6 = x->input[6];
  j7 = x->input[7];
  j8 = x->input[8];
  j9 = x->input[9];
  j10 = x->input[10];
  j11 = x->input[11];
  j12 = x->input[12];
  j13 = x->input[13];
  j14 = x->input[14];
  j15 = x->input[15];

  for (;;) {
    if (bytes < 64) {
      for (i = 0;i < bytes;++i) tmp[i] = m[i];
      m = tmp;
      ctarget = c;
      c = tmp;
    }
    x0 = j0;
    x1 = j1;
    x2 = j2;
    x3 = j3;
    x4 = j4;
    x5 = j5;
    x6 = j6;
    x7 = j7;
    x8 = j8;
    x9 = j9;
    x10 = j10;
    x11 = j11;
    x12 = j12;
    x13 = j13;
    x14 = j14;
    x15 = j15;
    for (i = 20;i > 0;i -= 2) {
      QUARTERROUND( x0, x4, x8,x12)
      QUARTERROUND( x1, x5, x9,x13)
      QUARTERROUND( x2, x6,x10,x14)
      QUARTERROUND( x3, x7,x11,x15)
      QUARTERROUND( x0, x5,x10,x15)
      QUARTERROUND( x1, x6,x11,x12)
      QUARTERROUND( x2, x7, x8,x13)
      QUARTERROUND( x3, x4, x9,x14)
    }
    x0 = PLUS(x0,j0);
    x1 = PLUS(x1,j1);
    x2 = PLUS(x2,j2);
    x3 = PLUS(x3,j3);
    x4 = PLUS(x4,j4);
    x5 = PLUS(x5,j5);
    x6 = PLUS(x6,j6);
    x7 = PLUS(x7,j7);
    x8 = PLUS(x8,j8);
    x9 = PLUS(x9,j9);
    x10 = PLUS(x10,j10);
    x11 = PLUS(x11,j11);
    x12 = PLUS(x12,j12);
    x13 = PLUS(x13,j13);
    x14 = PLUS(x14,j14);
    x15 = PLUS(x15,j15);

#ifndef KEYSTREAM_ONLY
    x0 = XOR(x0,U8TO32_LITTLE(m + 0));
    x1 = XOR(x1,U8TO32_LITTLE(m + 4));
    x2 = XOR(x2,U8TO32_LITTLE(m + 8));
    x3 = XOR(x3,U8TO32_LITTLE(m + 12));
    x4 = XOR(x4,U8TO32_LITTLE(m + 16));
    x5 = XOR(x5,U8TO32_LITTLE(m + 20));
    x6 = XOR(x6,U8TO32_LITTLE(m + 24));
    x7 = XOR(x7,U8TO32_LITTLE(m + 28));
    x8 = XOR(x8,U8TO32_LITTLE(m + 32));
    x9 = XOR(x9,U8TO32_LITTLE(m + 36));
    x10 = XOR(x10,U8TO32_LITTLE(m + 40));
    x11 = XOR(x11,U8TO32_LITTLE(m + 44));
    x12 = XOR(x12,U8TO32_LITTLE(m + 48));
    x13 = XOR(x13,U8TO32_LITTLE(m + 52));
    x14 = XOR(x14,U8TO32_LITTLE(m + 56));
    x15 = XOR(x15,U8TO32_LITTLE(m + 60));
#endif

    j12 = PLUSONE(j12);
    if (!j12) {
      j13 = PLUSONE(j13);
      /* stopping at 2^70 bytes per nonce is user's responsibility */
    }

    U32TO8_LITTLE(c + 0,x0);
    U32TO8_LITTLE(c + 4,x1);
    U32TO8_LITTLE(c + 8,x2);
    U32TO8_LITTLE(c + 12,x3);
    U32TO8_LITTLE(c + 16,x4);
    U32TO8_LITTLE(c + 20,x5);
    U32TO8_LITTLE(c + 24,x6);
    U32TO8_LITTLE(c + 28,x7);
    U32TO8_LITTLE(c + 32,x8);
    U32TO8_LITTLE(c + 36,x9);
    U32TO8_LITTLE(c + 40,x10);
    U32TO8_LITTLE(c + 44,x11);
    U32TO8_LITTLE(c + 48,x12);
    U32TO8_LITTLE(c + 52,x13);
    U32TO8_LITTLE(c + 56,x14);
    U32TO8_LITTLE(c + 60,x15);

    if (bytes <= 64) {
      if (bytes < 64) {
        for (i = 0;i < bytes;++i) ctarget[i] = c[i];
      }
      x->input[12] = j12;
      x->input[13] = j13;
      return;
    }
    bytes -= 64;
    c += 64;
#ifndef KEYSTREAM_ONLY
    m += 64;
#endif
  }
}

#define minimum(a, b) ((a) < (b) ? (a) : (b))

#include "arc4random.h"


static inline void
_rs_init(rand_state* st, u8 *buf, size_t n)
{
    assert(n >= (ARC4R_KEYSZ + ARC4R_IVSZ));

    _chacha_keysetup(&st->rs_chacha, buf, ARC4R_KEYSZ * 8, 0);
    _chacha_ivsetup(&st->rs_chacha,  buf + ARC4R_KEYSZ);
}



static inline void
_rs_rekey(rand_state* st, u8 *dat, size_t datlen)
{
    /* fill rs_buf with the keystream */
    _chacha_encrypt_bytes(&st->rs_chacha, st->rs_buf, st->rs_buf, sizeof st->rs_buf);

    /* mix in optional user provided data */
    if (dat) {
        size_t i, m;

        m = minimum(datlen, ARC4R_KEYSZ + ARC4R_IVSZ);
        for (i = 0; i < m; i++)
            st->rs_buf[i] ^= dat[i];

        memset(dat, 0, datlen);
    }

    /* immediately reinit for backtracking resistance */
    _rs_init(st, st->rs_buf, ARC4R_KEYSZ + ARC4R_IVSZ);
    memset(st->rs_buf, 0, ARC4R_KEYSZ + ARC4R_IVSZ);
    st->rs_have = (sizeof st->rs_buf) - ARC4R_KEYSZ - ARC4R_IVSZ;
}


static void
_rs_stir(rand_state* st)
{
    u8 rnd[ARC4R_KEYSZ + ARC4R_IVSZ];


    int r = getentropy(rnd, sizeof rnd);
    assert(r == 0);

    _rs_rekey(st, rnd, sizeof(rnd));

    /* invalidate rs_buf */
    st->rs_have = 0;
    memset(st->rs_buf, 0, sizeof st->rs_buf);

    st->rs_count = 1600000;
}


static inline void
_rs_stir_if_needed(rand_state* st, size_t len)
{
    if (st->rs_count <= len)
        _rs_stir(st);

    st->rs_count -= len;
}


static inline void
_rs_random_buf(rand_state* rs, void *_buf, size_t n)
{
    u8 *buf = (u8 *)_buf;
    u8 *keystream;
    size_t m;

    _rs_stir_if_needed(rs, n);
    while (n > 0) {
        if (rs->rs_have > 0) {
            m = minimum(n, rs->rs_have);
            keystream = rs->rs_buf + sizeof(rs->rs_buf) - rs->rs_have;
            memcpy(buf, keystream, m);
            memset(keystream, 0, m);
            buf += m;
            n   -= m;
            rs->rs_have -= m;
        } else 
            _rs_rekey(rs, NULL, 0);
    }
}

static inline uint32_t
_rs_random_u32(rand_state* rs)
{
    u8 *keystream;
    uint32_t val;

    _rs_stir_if_needed(rs, sizeof(val));
    if (rs->rs_have < sizeof(val))
        _rs_rekey(rs, NULL, 0);
    keystream = rs->rs_buf + sizeof(rs->rs_buf) - rs->rs_have;
    memcpy(&val, keystream, sizeof(val));
    memset(keystream, 0, sizeof(val));
    rs->rs_have -= sizeof(val);

    return val;
}


#if defined(__Darwin__) || defined(__APPLE__)

/*
 * Multi-threaded support using pthread API. Needed for OS X:
 *
 *   https://www.reddit.com/r/cpp/comments/3bg8jc/anyone_know_if_and_when_applexcode_will_support/
 */
static pthread_key_t     Rkey;
static pthread_once_t    Ronce   = PTHREAD_ONCE_INIT;
static volatile uint32_t Rforked = 0;

/*
 * Fork handler to reset my context
 */
static void
_atfork()
{
    // the pthread_atfork() callbacks called once per process.
    // We set it to be called by the child process.
    Rforked++;
}

/*
 * Run once and only once by pthread lib. We use the opportunity to
 * create the thread-specific key.
 */
static void
_screate()
{
    pthread_key_create(&Rkey, 0);
    pthread_atfork(0, 0, _atfork);

    /*
     * Get entropy once to initialize the fd - for non OpenBSD
     * systems.
     */
    uint8_t buf[8];
    getentropy(buf, sizeof buf);
}


/*
 * Get the per-thread rand state. Initialize if needed.
 */
static rand_state*
_sget()
{
    pthread_once(&Ronce, _screate);

    volatile pthread_key_t* k = &Rkey;
    rand_state * z = (rand_state *)pthread_getspecific(*k);
    if (!z) {
        z = (rand_state*)calloc(sizeof *z, 1);
        assert(z);

        _rs_stir(z);
        z->rs_pid = getpid();

        pthread_setspecific(*k, z);
    }

    /* Detect if a fork has happened */
    if (Rforked > 0 || getpid() != z->rs_pid) {
        Rforked   = 0;
        z->rs_pid = getpid();
        _rs_stir(z);
    }

    return z;
}

#else

/*
 * Use gcc extension to declare a thread-local variable.
 *
 * On most systems (including x86_64), thread-local access is
 * essentially free for non .so use cases.
 *
 */
static __thread rand_state st = { .rs_count = 0, .rs_pid = 0 };
static inline rand_state*
_sget()
{
    rand_state* s = &st;

    if (s->rs_count == 0 || getpid() != s->rs_pid) {
        _rs_stir(s);
        s->rs_pid = getpid();
    }
    return s;
}

#endif /* __Darwin__ */


/*
 * Public API.
 */


void
arc4random_buf(void* b, size_t n)
{
    rand_state* z = _sget();

    _rs_random_buf(z, b, n);
}



/*
 * Calculate a uniformly distributed random number less than upper_bound
 * avoiding "modulo bias".
 *
 * Uniformity is achieved by generating new random numbers until the one
 * returned is outside the range [0, 2**32 % upper_bound).  This
 * guarantees the selected random number will be inside
 * [2**32 % upper_bound, 2**32) which maps back to [0, upper_bound)
 * after reduction modulo upper_bound.
 */
uint32_t
arc4random_uniform(uint32_t upper_bound)
{
    rand_state* z = _sget();
    uint32_t r, min;

    if (upper_bound < 2)
        return 0;

    /* 2**32 % x == (2**32 - x) % x */
    min = -upper_bound % upper_bound;

    /*
     * This could theoretically loop forever but each retry has
     * p > 0.5 (worst case, usually far better) of selecting a
     * number inside the range we need, so it should rarely need
     * to re-roll.
     */
    for (;;) {
        r = _rs_random_u32(z);
        if (r >= min)
            break;
    }

    return r % upper_bound;
}


#endif /* __MP_LEGACY_SUPPORT_ARC4RANDOM__ */
