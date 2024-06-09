
/*
   SipHash reference C implementation

   Copyright (c) 2016 Jean-Philippe Aumasson <jeanphilippe.aumasson@gmail.com>

   To the extent possible under law, the author(s) have dedicated all copyright
   and related and neighboring rights to this software to the public domain
   worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along
   with
   this software. If not, see
   <http://creativecommons.org/publicdomain/zero/1.0/>.
 */
// modified for simplicity
#include <stddef.h>
#include <stdint.h>

/* default: SipHash-2-4 */
#ifndef cROUNDS
#define cROUNDS 2
#endif
#ifndef dROUNDS
#define dROUNDS 4
#endif

#define ROTL(x, b) (uint32_t)(((x) << (b)) | ((x) >> (32 - (b))))

#define U32TO8_LE(p, v)                                                        \
    (p)[0] = (uint8_t)((v));                                                   \
    (p)[1] = (uint8_t)((v) >> 8);                                              \
    (p)[2] = (uint8_t)((v) >> 16);                                             \
    (p)[3] = (uint8_t)((v) >> 24);

#define U8TO32_LE(p)                                                           \
    (((uint32_t)((p)[0])) | ((uint32_t)((p)[1]) << 8) |                        \
     ((uint32_t)((p)[2]) << 16) | ((uint32_t)((p)[3]) << 24))

#define SIPROUND                                                               \
    do {                                                                       \
        v0 += v1;                                                              \
        v1 = ROTL(v1, 5);                                                      \
        v1 ^= v0;                                                              \
        v0 = ROTL(v0, 16);                                                     \
        v2 += v3;                                                              \
        v3 = ROTL(v3, 8);                                                      \
        v3 ^= v2;                                                              \
        v0 += v3;                                                              \
        v3 = ROTL(v3, 7);                                                      \
        v3 ^= v0;                                                              \
        v2 += v1;                                                              \
        v1 = ROTL(v1, 13);                                                     \
        v1 ^= v2;                                                              \
        v2 = ROTL(v2, 16);                                                     \
    } while (0)


#ifdef DEBUG_SIPHASH
#include <stdio.h>

#define TRACE                                                                  \
    do {                                                                       \
        printf("(%3zu) v0 %08" PRIx32 "\n", inlen, v0);                        \
        printf("(%3zu) v1 %08" PRIx32 "\n", inlen, v1);                        \
        printf("(%3zu) v2 %08" PRIx32 "\n", inlen, v2);                        \
        printf("(%3zu) v3 %08" PRIx32 "\n", inlen, v3);                        \
    } while (0)
#else
#define TRACE
#endif

/*
    Computes a SipHash value
    *in: pointer to input data (read-only)
    inlen: input data length in bytes (any size_t value)
    *k: pointer to the key data (read-only), must be 8 bytes 
    *out: pointer to output data (write-only), outlen bytes must be allocated
    outlen: length of the output in bytes, must be 4 or 8
*/
static uint32_t halfsiphash(const void *in, const size_t inlen, uint64_t k) {

    const unsigned char *ni = (const unsigned char *)in;

    uint32_t v0 = 0;
    uint32_t v1 = 0;
    uint32_t v2 = UINT32_C(0x6c796765);
    uint32_t v3 = UINT32_C(0x74656462);
    uint32_t k0 = k;
    uint32_t k1 = k >> 32;
    uint32_t m;
    int i;
    const unsigned char *end = ni + inlen - (inlen % sizeof(uint32_t));
    const int left = inlen & 3;
    uint32_t b = ((uint32_t)inlen) << 24;
    v3 ^= k1;
    v2 ^= k0;
    v1 ^= k1;
    v0 ^= k0;

    for (; ni != end; ni += 4) {
        m = U8TO32_LE(ni);
        v3 ^= m;

        TRACE;
        for (i = 0; i < cROUNDS; ++i)
            SIPROUND;

        v0 ^= m;
    }

    switch (left) {
    case 3:
        b |= ((uint32_t)ni[2]) << 16;
        /* FALLTHRU */
    case 2:
        b |= ((uint32_t)ni[1]) << 8;
        /* FALLTHRU */
    case 1:
        b |= ((uint32_t)ni[0]);
        break;
    case 0:
        break;
    }

    v3 ^= b;

    TRACE;
    for (i = 0; i < cROUNDS; ++i)
        SIPROUND;

    v0 ^= b;

    v2 ^= 0xff;

    TRACE;
    for (i = 0; i < dROUNDS; ++i)
        SIPROUND;

    b = v1 ^ v3;
    return b;
}
