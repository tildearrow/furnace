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

#include "sm8521.h"
#include <stdlib.h>

enum sm8521_sgc
{
	SM8521_SGC_SONDOUT = (1 << 7),
	SM8521_SGC_DIROUT = (1 << 3),
	SM8521_SGC_SG2OUT = (1 << 2),
	SM8521_SGC_SG1OUT = (1 << 1),
	SM8521_SGC_SG0OUT = (1 << 0)
};

void sm8521_sg_wave_tick(struct sm8521_wave_t *sg, const int cycle)
{
	sg->base.counter += cycle;
	while (sg->base.counter >= (sg->base.t + 1))
	{
		sg->addr++;
		sg->base.counter -= (sg->base.t + 1);
	}
	int wave = (sg->wave[(sg->addr >> 1) & 0xf] >> ((sg->addr & 1) << 2)) & 0xf;
	if (wave & 0x8)
	{
		wave = -(0x8 - (wave & 0x7));
	}
	sg->base.out = (wave * sg->base.level) >> 1; // scale out to 8bit
}

void sm8521_noise_tick(struct sm8521_noise_t *noise, const int cycle)
{
	noise->base.counter += cycle;
	while (noise->base.counter >= (noise->base.t + 1))
	{
                // https://github.com/tildearrow/furnace/issues/2567
                // unknown algorithm, but don't use rand()
                //
                // some research suggests VIC-like noise, although
                // that remains to be confirmed
                noise->oldLFSR = noise->lfsr & 1;
		noise->lfsr = ( noise->lfsr>>1|(((noise->lfsr) ^ (noise->lfsr >> 5) ^ (noise->lfsr >> 8) ^ (noise->lfsr >> 13) ) & 1)<<31);
		noise->base.counter -= (noise->base.t + 1);
                if (noise->oldLFSR^(noise->lfsr&1)) {
                  noise->out ^= 1;
                }
	}
	noise->base.out = ((noise->out ? 7 : -8) * noise->base.level) >> 1; // scale out to 8bit
}

void sm8521_sound_tick(struct sm8521_t *sm8521, const int cycle)
{
	int out = 0;
	if (sm8521->sgc & SM8521_SGC_SONDOUT)
	{
		if (sm8521->sgc & SM8521_SGC_DIROUT)
		{
			out = sm8521->sgda - 0x80;
		}
		else
		{
			if (sm8521->sgc & SM8521_SGC_SG0OUT)
			{
				sm8521_sg_wave_tick(&sm8521->sg[0], cycle);
				out += sm8521->sg[0].base.out;
			}
			if (sm8521->sgc & SM8521_SGC_SG1OUT)
			{
				sm8521_sg_wave_tick(&sm8521->sg[1], cycle);
				out += sm8521->sg[1].base.out;
			}
			if (sm8521->sgc & SM8521_SGC_SG2OUT)
			{
				sm8521_noise_tick(&sm8521->noise, cycle);
				out += sm8521->noise.base.out;
			}
			out = (out < -0x80) ? -0x80 : ((out > 0x7f) ? 0x7f : out); // clamp
		}
	}
	sm8521->out = out;
}

void sm8521_reset(struct sm8521_t *sm8521)
{
	for (int i = 0; i < 2; i++)
	{
		sm8521->sg[i].base.t = 0;
		sm8521->sg[i].base.level = 0;
		sm8521->sg[i].base.out = 0;
		sm8521->sg[i].base.counter = 0;
		sm8521->sg[i].addr = 0;
		for (int j = 0; j < 16; j++)
		{
			sm8521->sg[i].wave[j] = 0;
		}
	}
	sm8521->noise.base.t = 0;
	sm8521->noise.base.level = 0;
	sm8521->noise.base.out = 0;
	sm8521->noise.base.counter = 0;
	sm8521->noise.lfsr = 0x89abcdef;
	sm8521->noise.oldLFSR = 1;
	sm8521->noise.out = 0;
	sm8521->out = 0;
	sm8521->sgda = 0;
	sm8521->sgc = 0;
}

unsigned char sm8521_read(struct sm8521_t *sm8521, const unsigned char a)
{
	if ((a & 0xe0) == 0x60)
	{
		if ((a & 0x10) && (!(sm8521->sgc & SM8521_SGC_SG1OUT))) // 0x70-0x7f SG1W0-15
		{
			return sm8521->sg[1].wave[a & 0xf];
		}
		else if ((!(a & 0x10)) && (!(sm8521->sgc & SM8521_SGC_SG0OUT))) // 0x60-0x6f SG0W0-15
		{
			return sm8521->sg[0].wave[a & 0xf];
		}
		return 0;
	}
	switch (a)
	{
		case 0x40: // SGC
			return sm8521->sgc;
			break;
		case 0x42: // SG0L
			return sm8521->sg[0].base.level & 0x1f;
			break;
		case 0x44: // SG1L
			return sm8521->sg[1].base.level & 0x1f;
			break;
		case 0x46: // SG0TH
			return (sm8521->sg[0].base.t >> 8) & 0xf;
			break;
		case 0x47: // SG0TL
			return sm8521->sg[0].base.t & 0xff;
			break;
		case 0x48: // SG1TH
			return (sm8521->sg[1].base.t >> 8) & 0xf;
			break;
		case 0x49: // SG1TL
			return sm8521->sg[1].base.t & 0x0ff;
			break;
		case 0x4a: // SG2L
			return sm8521->noise.base.level & 0x1f;
			break;
		case 0x4c: // SG2TH
			return (sm8521->noise.base.t >> 8) & 0xf;
			break;
		case 0x4d: // SG2TL
			return sm8521->noise.base.t & 0xff;
			break;
		default:
			return 0;
			break;
	}
}
void sm8521_write(struct sm8521_t *sm8521, const unsigned char a, const unsigned char d)
{
	if ((a & 0xe0) == 0x60)
	{
		if ((a & 0x10) && (!(sm8521->sgc & SM8521_SGC_SG1OUT))) // 0x70-0x7f SG1W0-15
		{
			sm8521->sg[1].wave[a & 0xf] = d;
		}
		else if ((!(a & 0x10)) && (!(sm8521->sgc & SM8521_SGC_SG0OUT))) // 0x60-0x6f SG0W0-15
		{
			sm8521->sg[0].wave[a & 0xf] = d;
		}
		return;
	}
	switch (a)
	{
		case 0x40: // SGC
			sm8521->sgc = d;
			break;
		case 0x42: // SG0L
			sm8521->sg[0].base.level = d & 0x1f;
			break;
		case 0x44: // SG1L
			sm8521->sg[1].base.level = d & 0x1f;
			break;
		case 0x46: // SG0TH
			sm8521->sg[0].base.t = (sm8521->sg[0].base.t & 0x0ff) | ((d << 8) & 0xf00);
			break;
		case 0x47: // SG0TL
			sm8521->sg[0].base.t = (sm8521->sg[0].base.t & 0xf00) | (d & 0x0ff);
			break;
		case 0x48: // SG1TH
			sm8521->sg[1].base.t = (sm8521->sg[1].base.t & 0x0ff) | ((d << 8) & 0xf00);
			break;
		case 0x49: // SG1TL
			sm8521->sg[1].base.t = (sm8521->sg[1].base.t & 0xf00) | (d & 0x0ff);
			break;
		case 0x4a: // SG2L
			sm8521->noise.base.level = d & 0x1f;
			break;
		case 0x4c: // SG2TH
			sm8521->noise.base.t = (sm8521->noise.base.t & 0x0ff) | ((d << 8) & 0xf00);
			break;
		case 0x4d: // SG2TL
			sm8521->noise.base.t = (sm8521->noise.base.t & 0xf00) | (d & 0x0ff);
			break;
		case 0x4e: // SGDA
			sm8521->sgda = d;
			break;
	}
}
