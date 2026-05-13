/*
 * scsp_types.h — Type definitions and stubs for standalone SCSP build.
 * Replaces ao.h, cpuintrf.h, osd_cpu.h, sat_hw.h when building for WASM.
 */
#ifndef SCSP_TYPES_H
#define SCSP_TYPES_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* ── Integer types (ao.h / osd_cpu.h) ───────────────────────── */
typedef unsigned char   uint8,  UINT8;
typedef signed char     int8,   INT8;
typedef unsigned short  uint16, UINT16;
typedef signed short    int16,  INT16;
typedef unsigned int    uint32, UINT32;
typedef signed int      int32,  INT32;
typedef unsigned long long uint64, UINT64;
typedef signed long long   int64, INT64;

typedef struct { int16 l; int16 r; } stereo_sample_t;

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef INLINE
#define INLINE static inline
#endif

/* ── Endian helpers ──────────────────────────────────────────── */
#define LSB_FIRST 1

INLINE uint16 SWAP16(uint16 x) { return ((x & 0xFF00) >> 8) | ((x & 0x00FF) << 8); }
INLINE uint32 SWAP32(uint32 x) {
    return ((x & 0xFF000000) >> 24) | ((x & 0x00FF0000) >> 8) |
           ((x & 0x0000FF00) << 8)  | ((x & 0x000000FF) << 24);
}

#define BE16(x) (SWAP16(x))
#define BE32(x) (SWAP32(x))
#define LE16(x) (x)
#define LE32(x) (x)

#define AUDIO_RATE 44100

/* ── cpuintrf.h stubs ────────────────────────────────────────── */
#define COMBINE_DATA(varptr) (*(varptr) = (*(varptr) & mem_mask) | (data & ~mem_mask))

/* ── sat_hw.h stubs ──────────────────────────────────────────── */
extern uint8 sat_ram[512 * 1024];

INLINE unsigned short mem_readword_swap(unsigned short *addr) {
    return *addr;
}

INLINE void mem_writeword_swap(unsigned short *addr, unsigned short value) {
    *addr = value;
}

/* stv_scu referenced by scsp.c */
extern UINT32 *stv_scu;

/* ao.h misc */
#define AUDIO_RATE 44100
typedef unsigned char ao_bool;

/* Suppress ao.h function declarations */
#define fopen fopen
#define mkdir mkdir

#endif /* SCSP_TYPES_H */
