/*

============================================================================

Namco C140 sound emulator
by cam900

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

#include "c140.h"

static int c140_max(int a, int b) { return (a > b) ? a : b; }
static int c140_min(int a, int b) { return (a < b) ? a : b; }
static int c140_clamp(int v, int min, int max) { return c140_min(c140_max(v,min),max); }
static int c140_bit(int val, int bit) { return (val >> bit) & 1; }
static int c140_bitfield(int val, int bit, int len) { return (val >> bit) & ((1 << len) - 1);}

void c140_tick(struct c140_t *c140, const int cycle)
{
	c140->lout = 0;
	c140->rout = 0;
	for (int i = 0; i < 24; i++)
	{
		c140_voice_tick(c140, i, cycle);
		c140->lout += c140->voice[i].lout;
		c140->rout += c140->voice[i].rout;
	}
	// scale as 16bit
	c140->lout >>= 10;
	c140->rout >>= 10;
}

void c140_voice_tick(struct c140_t *c140, const unsigned char voice, const int cycle)
{
	struct c140_voice_t *c140_voice = &c140->voice[voice];
	if (c140_voice->busy && c140_voice->keyon)
	{
		for (int c = 0; c < cycle; c++)
		{
			c140_voice->frac += c140_voice->freq;
			if (c140_voice->frac > 0xffff)
			{
				c140_voice->addr += c140_voice->frac >> 16;
				if (c140_voice->addr > c140_voice->end_addr)
				{
					if (c140_voice->loop)
					{
						c140_voice->addr = (c140_voice->addr + c140_voice->end_addr) - c140_voice->loop_addr;
					}
					else
					{
						c140_voice->keyon = false;
					}
				}
				c140_voice->frac &= 0xffff;
			}
			// fetch 12 bit sample
			signed short s1 = c140->sample_mem[((unsigned int)(c140_voice->bank) << 16) | c140_voice->addr] & ~0xf;
			signed short s2 = c140->sample_mem[((unsigned int)(c140_voice->bank) << 16) | ((c140_voice->addr + 1) & 0xffff)] & ~0xf;
			if (c140_voice->compressed)
			{
				s1 = c140->mulaw[(s1 >> 8) & 0xff];
				s2 = c140->mulaw[(s2 >> 8) & 0xff];
			}
			// interpolate
			signed int sample = s1 + (((c140_voice->frac) * (s2 - s1)) >> 16);
			c140_voice->lout = sample * c140_voice->lvol;
			c140_voice->rout = sample * c140_voice->rvol;
		}
	}
	else
	{
		c140_voice->lout = 0;
		c140_voice->rout = 0;
	}
}

void c140_keyon(struct c140_voice_t *c140_voice)
{
	c140_voice->busy = true;
	c140_voice->keyon = true;
	c140_voice->frac = 0;
	c140_voice->addr = c140_voice->start_addr;
}

void c140_init(struct c140_t *c140)
{
	// u-law table verified from Wii Virtual Console Arcade Starblade
	for (int i = 0; i < 256; i++)
	{
		const unsigned char exponent = c140_bitfield(i, 0, 3);
		const unsigned char mantissa = c140_bitfield(i, 3, 4);
		if (c140_bit(i, 7))
		{
			c140->mulaw[i] = (signed short)(((exponent ? 0xfe00 : 0xff00) | (mantissa << 4))
										<< (exponent ? exponent - 1 : 0));
		}
		else
		{
			c140->mulaw[i] = (signed short)(((exponent ? 0x100 : 0) | (mantissa << 4))
										<< (exponent ? exponent - 1 : 0));
		}
	}
}

void c140_reset(struct c140_t *c140)
{
	for (int i = 0; i < 24; i++)
	{
		c140->voice[i].busy = false;
		c140->voice[i].keyon = false;
		c140->voice[i].freq = 0;
		c140->voice[i].bank = 0;
		c140->voice[i].start_addr = 0;
		c140->voice[i].loop_addr = 0;
		c140->voice[i].end_addr = 0;
		c140->voice[i].lvol = 0;
		c140->voice[i].rvol = 0;
		c140->voice[i].compressed = false;
		c140->voice[i].loop = false;
		c140->voice[i].addr = 0;
		c140->voice[i].frac = 0;
		c140->voice[i].lout = 0;
		c140->voice[i].rout = 0;
	}
}

void c140_write(struct c140_t *c140, const unsigned short addr, const unsigned char data)
{
	// voice register
	if (addr < 0x180)
	{
		struct c140_voice_t *voice = &c140->voice[addr >> 4];
		switch (addr & 0xf)
		{
			case 0x0: voice->rvol = data; break;
			case 0x1: voice->lvol = data; break;
			case 0x2: voice->freq = (voice->freq & ~0xff00) | (unsigned int)(data << 8); break;
			case 0x3: voice->freq = (voice->freq & ~0x00ff) | data; break;
			case 0x4: voice->bank = data; break;
			case 0x5:
				voice->compressed = c140_bit(data, 3);
				voice->loop = c140_bit(data, 4);
				if (data & 0x80)
					c140_keyon(voice);
				else
					voice->busy = false;
				break;
			case 0x6: voice->start_addr = (voice->start_addr & ~0xff00) | (unsigned int)(data << 8); break;
			case 0x7: voice->start_addr = (voice->start_addr & ~0x00ff) | data; break;
			case 0x8: voice->end_addr = (voice->end_addr & ~0xff00) | (unsigned int)(data << 8); break;
			case 0x9: voice->end_addr = (voice->end_addr & ~0x00ff) | data; break;
			case 0xa: voice->loop_addr = (voice->loop_addr & ~0xff00) | (unsigned int)(data << 8); break;
			case 0xb: voice->loop_addr = (voice->loop_addr & ~0x00ff) | data; break;
			default: break;
		}
	}
	// Timer
}