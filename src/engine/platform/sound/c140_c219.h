/*

============================================================================

MODIFIED Namco C140/C219 sound emulator - MODIFIED VERSION
by cam900

MODIFICATION by tildearrow - adds muting function AND VGM banking
THIS IS NOT THE ORIGINAL VERSION - you can find the original one in
commit 72d04777c013988ed8cf6da27c62a9d784a59dff

This file is licensed under zlib license.

============================================================================

zlib License

(C) 2023-present cam900 and contributors

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

============================================================================

TODO:
- unknown registers (Bit 6 of control register, etc)
- Internal timer

*/

#ifndef _C140_C219_EMU_H
#define _C140_C219_EMU_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct c140_voice_t
{
   bool muted;                // muted - can be set by user
   bool busy;                 // busy flag
   bool keyon;                // key on flag
   unsigned short freq;       // sample frequency
   unsigned char bank;        // sample bank
   unsigned int start_addr;   // sample start address
   unsigned int loop_addr;    // sample loop address
   unsigned int end_addr;     // sample end address
   int lvol, rvol;            // left/right volume
   bool noise;                // noise flag
   bool inv_lout;             // invert left output flag
   bool inv_sign;             // invert sign bit flag
   bool compressed;           // compressed sample flag
   bool loop;                 // loop flag
   unsigned int addr;         // sample address
   int frac;                  // frequency counter (.16 fixed point)
   int lout, rout;            // left/right output
};

struct c140_t
{
   struct c140_voice_t voice[24];
   signed int lout, rout;
   signed short mulaw[256];
   signed short *sample_mem;
   unsigned char bank_type;
};

struct c219_t
{
   struct c140_voice_t voice[16];
   signed int lout, rout;
   signed short mulaw[256];
   unsigned short lfsr;
   unsigned char bank[4];
   signed char *sample_mem;
};

void c140_tick(struct c140_t *c140, const int cycle);

void c219_tick(struct c219_t *c219, const int cycle);

void c140_voice_tick(struct c140_t *c140, const unsigned char v, const int cycle);

void c219_voice_tick(struct c219_t *c219, const unsigned char v, const int cycle);

void c140_keyon(struct c140_voice_t *c140_voice);

void c219_keyon(struct c140_voice_t *c140_voice);

void c140_write(struct c140_t *c140, const unsigned short addr, const unsigned char data);

void c219_write(struct c219_t *c219, const unsigned short addr, const unsigned char data);

void c140_bank_type(struct c140_t *c140, unsigned char type);

void c140_init(struct c140_t *c140);

void c219_init(struct c219_t *c219);

void c140_reset(struct c140_t *c140);

void c219_reset(struct c219_t *c219);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _C140_C219_EMU_H
