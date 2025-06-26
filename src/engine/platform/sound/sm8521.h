/*

============================================================================

SM8521 sound emulator
by cam900

MODIFIED BY TILDEARROW
THIS IS NOT THE ORIGINAL VERSION

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
- needs hardware test

*/

#ifndef _SM8521_EMU_H
#define _SM8521_EMU_H

#ifdef __cplusplus
extern "C"
{
#endif

struct sm8521_sg_t
{
	unsigned short t; // Time constant register
	unsigned char level; // Output level control register
	signed short out; // output
	int counter; // clock counter
};

struct sm8521_wave_t
{
	struct sm8521_sg_t base;
	unsigned char addr; // waveform address
	unsigned char wave[16]; // 4 bit waveform (32 nybbles)
};

struct sm8521_noise_t
{
	struct sm8521_sg_t base;
	unsigned int lfsr, oldLFSR; // LFSR
        unsigned char out;
};

struct sm8521_t
{
	struct sm8521_wave_t sg[2];
	struct sm8521_noise_t noise;
	signed short out; // output
	signed char sgda; // D/A direct output register (write only)
	unsigned char sgc; // Control register
};

void sm8521_sg_wave_tick(struct sm8521_wave_t *sg, const int cycle);

void sm8521_noise_tick(struct sm8521_noise_t *noise, const int cycle);

void sm8521_sound_tick(struct sm8521_t *sm8521, const int cycle);

void sm8521_reset(struct sm8521_t *sm8521);

unsigned char sm8521_read(struct sm8521_t *sm8521, const unsigned char a);
void sm8521_write(struct sm8521_t *sm8521, const unsigned char a, const unsigned char d);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _SM8521_EMU_H
