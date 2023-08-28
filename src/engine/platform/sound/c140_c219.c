/*

============================================================================

MODIFIED Namco C140/C219 sound emulator - MODIFIED VERSION
by cam900

MODIFICATION by tildearrow - adds muting function and fixes overflow
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

#include "c140_c219.h"

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
}

void c219_tick(struct c219_t *c219, const int cycle)
{
	c219->lout = 0;
	c219->rout = 0;
	for (int i = 0; i < 16; i++)
	{
		c219_voice_tick(c219, i, cycle);
		c219->lout += c219->voice[i].lout;
		c219->rout += c219->voice[i].rout;
	}
}

void c140_voice_tick(struct c140_t *c140, const unsigned char v, const int cycle)
{
	struct c140_voice_t *voice = &c140->voice[v];
	if (voice->busy && voice->keyon)
	{
		for (int c = 0; c < cycle; c++)
		{
			voice->frac += voice->freq;
			if (voice->frac > 0xffff)
			{
				voice->addr += voice->frac >> 16;
				if (voice->addr > voice->end_addr)
				{
					if (voice->loop)
					{
						voice->addr = (voice->addr + voice->loop_addr) - voice->end_addr;
					}
					else
					{
						voice->keyon = false;
						voice->lout = 0;
						voice->rout = 0;
						return;
					}
				}
				voice->frac &= 0xffff;
			}
		}
		if (!voice->muted)
		{
			// fetch 12 bit sample
			signed short s1 = c140->sample_mem[((unsigned int)(voice->bank) << 16) | voice->addr] & ~0xf;
			signed short s2 = c140->sample_mem[((unsigned int)(voice->bank) << 16) | ((voice->addr + 1) & 0xffff)] & ~0xf;
			if (voice->compressed)
			{
				s1 = c140->mulaw[(s1 >> 8) & 0xff];
				s2 = c140->mulaw[(s2 >> 8) & 0xff];
			}
			// interpolate (originally was >>16, but I had to reduce it to 15 to prevent overflow)
			signed int sample = s1 + (((voice->frac >> 1) * (s2 - s1)) >> 15);
			voice->lout = sample * voice->lvol;
			voice->rout = sample * voice->rvol;
		}
		else
		{
			voice->lout = 0;
			voice->rout = 0;
		}
	}
	else
	{
		voice->lout = 0;
		voice->rout = 0;
	}
}

void c219_voice_tick(struct c219_t *c219, const unsigned char v, const int cycle)
{
	struct c140_voice_t *voice = &c219->voice[v];
	if (voice->busy && voice->keyon)
	{
		for (int c = 0; c < cycle; c++)
		{
			voice->frac += voice->freq;
			if (voice->frac > 0xffff)
			{
				voice->addr += voice->frac >> 16;
				if ((voice->addr >> 1) > voice->end_addr)
				{
					if (voice->loop)
					{
						voice->addr = (voice->addr + (voice->loop_addr << 1)) - (voice->end_addr << 1);
					}
					else
					{
						voice->keyon = false;
						voice->lout = 0;
						voice->rout = 0;
						return;
					}
				}
				if (voice->noise)
				{
					c219->lfsr = (c219->lfsr >> 1) ^ ((-(c219->lfsr & 1)) & 0xfff6);
				}
				voice->frac &= 0xffff;
			}
		}
		if (!voice->muted)
		{
			signed int sample = 0;
			if (voice->noise)
			{
				sample = (signed int)((signed short)(c219->lfsr));
			}
			else
			{
				// fetch 8 bit sample
				signed short s1 = c219->sample_mem[((unsigned int)(c219->bank[(v >> 2) & 3]) << 17) | voice->addr];
				signed short s2 = c219->sample_mem[((unsigned int)(c219->bank[(v >> 2) & 3]) << 17) | ((voice->addr + 1) & 0x1ffff)];
				if (voice->compressed)
				{
					s1 = c219->mulaw[s1];
					s2 = c219->mulaw[s2];
				}
				else
				{
					s1 = (signed short)((signed char)(s1) << 8);
					s2 = (signed short)((signed char)(s2) << 8);
				}
				if (voice->inv_sign)
				{
					s1 = -s1;
					s2 = -s2;
				}
				// interpolate (originally was >>16, but I had to reduce it to 15 to prevent overflow)
				sample = s1 + (((voice->frac >> 1) * (s2 - s1)) >> 15);
			}
			voice->lout = (voice->inv_lout ? (-sample) : sample) * voice->lvol;
			voice->rout = sample * voice->rvol;
		}
		else
		{
			voice->lout = 0;
			voice->rout = 0;
		}
	}
	else
	{
		voice->lout = 0;
		voice->rout = 0;
	}
}

void c140_keyon(struct c140_voice_t *c140_voice)
{
	c140_voice->busy = true;
	c140_voice->keyon = true;
	c140_voice->frac = 0;
	c140_voice->addr = c140_voice->start_addr;
}

void c219_keyon(struct c140_voice_t *c140_voice)
{
	c140_voice->busy = true;
	c140_voice->keyon = true;
	c140_voice->frac = 0;
	c140_voice->addr = c140_voice->start_addr << 1;
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
	for (int i = 0; i < 24; i++)
	{
		c140->voice[i].muted = false;
	}
}

void c219_init(struct c219_t *c219)
{
	// u-law table verified from Wii Virtual Console Arcade Knuckle Heads
	for (int i = 0; i < 128; i++)
	{
		signed int compressed_sample = 0;
		if (i < 16)
		{
			compressed_sample = 0x20 * i;
		}
		else if (i < 24)
		{
			compressed_sample = (0x200 + (0x40 * i)) - 0x400;
		}
		else if (i < 48)
		{
			compressed_sample = (0x400 + (0x80 * i)) - 0xc00;
		}
		else if (i < 100)
		{
			compressed_sample = (0x1000 + (0x100 * i)) - 0x3000;
		}
		else
		{
			compressed_sample = (0x4400 + (0x200 * i)) - 0xc800;
		}
		c219->mulaw[i]       = compressed_sample;
		c219->mulaw[i + 128] = (~compressed_sample) & 0xffe0;
	}
	for (int i = 0; i < 16; i++)
	{
		c219->voice[i].muted = false;
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

void c219_reset(struct c219_t *c219)
{
	for (int i = 0; i < 16; i++)
	{
		c219->voice[i].busy = false;
		c219->voice[i].keyon = false;
		c219->voice[i].freq = 0;
		c219->voice[i].start_addr = 0;
		c219->voice[i].loop_addr = 0;
		c219->voice[i].end_addr = 0;
		c219->voice[i].lvol = 0;
		c219->voice[i].rvol = 0;
		c219->voice[i].noise = false;
		c219->voice[i].inv_lout = false;
		c219->voice[i].inv_sign = false;
		c219->voice[i].compressed = false;
		c219->voice[i].loop = false;
		c219->voice[i].addr = 0;
		c219->voice[i].frac = 0;
		c219->voice[i].lout = 0;
		c219->voice[i].rout = 0;
	}
	c219->lfsr = 0x1234;
	for (int i = 0; i < 4; i++)
	{
		c219->bank[i] = 0;
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

void c219_write(struct c219_t *c219, const unsigned short addr, const unsigned char data)
{
	// voice register
	if (addr < 0x180)
	{
		struct c140_voice_t *voice = &c219->voice[addr >> 4];
		switch (addr & 0xf)
		{
			case 0x0: voice->rvol = data; break;
			case 0x1: voice->lvol = data; break;
			case 0x2: voice->freq = (voice->freq & ~0xff00) | (unsigned int)(data << 8); break;
			case 0x3: voice->freq = (voice->freq & ~0x00ff) | data; break;
			//case 0x4: break; // Unknown
			case 0x5:
				voice->compressed = c140_bit(data, 0);
				voice->noise = c140_bit(data, 2);
				voice->inv_lout = c140_bit(data, 3);
				voice->loop = c140_bit(data, 4);
				voice->inv_sign = c140_bit(data, 6);
				if (data & 0x80)
					c219_keyon(voice);
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
	// bank
	else if (addr >= 0x1f0)
	{
		if (addr & 1)
		{
			const unsigned short bankaddr = (addr >> 1) & 3;
			c219->bank[(bankaddr + 1) & 3] = (data & 3);
		}
	}
}
