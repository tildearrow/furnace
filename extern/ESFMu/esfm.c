/*
 * ESFMu: emulator for the ESS "ESFM" enhanced OPL3 clone
 * Copyright (C) 2023 Kagamiin~
 *
 * This file includes code and data from the Nuked OPL3 project, copyright (C)
 * 2013-2023 Nuke.YKT. Its usage, modification and redistribution is allowed
 * under the terms of the GNU Lesser General Public License version 2.1 or
 * later.
 *
 * ESFMu is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 2.1
 * of the License, or (at your option) any later version.
 *
 * ESFMu is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ESFMu. If not, see <https://www.gnu.org/licenses/>.
 */

/*
 * ESFMu wouldn't have been possible without the hard work and dedication of
 * the retro computer hardware research and preservation community.
 *
 * I'd like to thank:
 *  - Nuke.YKT
 *        Developer of Nuked OPL3, which was the basis for ESFMu's code and
 *        also a great learning resource on Yamaha FM synthesis for myself.
 *        Nuke.YKT also gives shoutouts on behalf of Nuked OPL3 to:
 *        - MAME Development Team(Jarek Burczynski, Tatsuyuki Satoh):
 *              Feedback and Rhythm part calculation information.
 *        - forums.submarine.org.uk(carbon14, opl3):
 *              Tremolo and phase generator calculation information.
 *        - OPLx decapsulated(Matthew Gambrell, Olli Niemitalo):
 *              OPL2 ROMs.
 *        - siliconpr0n.org(John McMaster, digshadow):
 *              YMF262 and VRC VII decaps and die shots.
 * - rainwarrior
 *       For performing the initial research on ESFM drivers and documenting
 *       ESS's patent on native mode operator organization.
 * - jwt27
 *       For kickstarting the ESFM research project and compiling rainwarrior's
 *       findings and more in an accessible document ("ESFM Demystified").
 * - pachuco/CatButts
 *       For documenting ESS's patent on ESFM's feedback implementation, which
 *       was vital in getting ESFMu's sound output to be accurate.
 * - And everybody who helped out with real hardware testing
 */

#include "esfm.h"
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

/*
 * Log-scale quarter sine table extracted from OPL3 ROM; taken straight from
 * Nuked OPL3 source code.
 * TODO: Extract sine table from ESFM die scans... does ESFM even use a sine
 * table? Patent documents give a hint to a possible method of generating sine
 * waves using some sort of boolean logic wizardry (lol)
 */
static const uint16_t logsinrom[256] = {
	0x859, 0x6c3, 0x607, 0x58b, 0x52e, 0x4e4, 0x4a6, 0x471,
	0x443, 0x41a, 0x3f5, 0x3d3, 0x3b5, 0x398, 0x37e, 0x365,
	0x34e, 0x339, 0x324, 0x311, 0x2ff, 0x2ed, 0x2dc, 0x2cd,
	0x2bd, 0x2af, 0x2a0, 0x293, 0x286, 0x279, 0x26d, 0x261,
	0x256, 0x24b, 0x240, 0x236, 0x22c, 0x222, 0x218, 0x20f,
	0x206, 0x1fd, 0x1f5, 0x1ec, 0x1e4, 0x1dc, 0x1d4, 0x1cd,
	0x1c5, 0x1be, 0x1b7, 0x1b0, 0x1a9, 0x1a2, 0x19b, 0x195,
	0x18f, 0x188, 0x182, 0x17c, 0x177, 0x171, 0x16b, 0x166,
	0x160, 0x15b, 0x155, 0x150, 0x14b, 0x146, 0x141, 0x13c,
	0x137, 0x133, 0x12e, 0x129, 0x125, 0x121, 0x11c, 0x118,
	0x114, 0x10f, 0x10b, 0x107, 0x103, 0x0ff, 0x0fb, 0x0f8,
	0x0f4, 0x0f0, 0x0ec, 0x0e9, 0x0e5, 0x0e2, 0x0de, 0x0db,
	0x0d7, 0x0d4, 0x0d1, 0x0cd, 0x0ca, 0x0c7, 0x0c4, 0x0c1,
	0x0be, 0x0bb, 0x0b8, 0x0b5, 0x0b2, 0x0af, 0x0ac, 0x0a9,
	0x0a7, 0x0a4, 0x0a1, 0x09f, 0x09c, 0x099, 0x097, 0x094,
	0x092, 0x08f, 0x08d, 0x08a, 0x088, 0x086, 0x083, 0x081,
	0x07f, 0x07d, 0x07a, 0x078, 0x076, 0x074, 0x072, 0x070,
	0x06e, 0x06c, 0x06a, 0x068, 0x066, 0x064, 0x062, 0x060,
	0x05e, 0x05c, 0x05b, 0x059, 0x057, 0x055, 0x053, 0x052,
	0x050, 0x04e, 0x04d, 0x04b, 0x04a, 0x048, 0x046, 0x045,
	0x043, 0x042, 0x040, 0x03f, 0x03e, 0x03c, 0x03b, 0x039,
	0x038, 0x037, 0x035, 0x034, 0x033, 0x031, 0x030, 0x02f,
	0x02e, 0x02d, 0x02b, 0x02a, 0x029, 0x028, 0x027, 0x026,
	0x025, 0x024, 0x023, 0x022, 0x021, 0x020, 0x01f, 0x01e,
	0x01d, 0x01c, 0x01b, 0x01a, 0x019, 0x018, 0x017, 0x017,
	0x016, 0x015, 0x014, 0x014, 0x013, 0x012, 0x011, 0x011,
	0x010, 0x00f, 0x00f, 0x00e, 0x00d, 0x00d, 0x00c, 0x00c,
	0x00b, 0x00a, 0x00a, 0x009, 0x009, 0x008, 0x008, 0x007,
	0x007, 0x007, 0x006, 0x006, 0x005, 0x005, 0x005, 0x004,
	0x004, 0x004, 0x003, 0x003, 0x003, 0x002, 0x002, 0x002,
	0x002, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000
};

/*
 * Inverse exponent table extracted from OPL3 ROM; taken straight from
 * Nuked OPL3 source code.
 * TODO: Verify if ESFM uses an exponent table or if it possibly uses another
 * method to skirt around Yamaha's patents?
 */
static const uint16_t exprom[256] = {
	0x7fa, 0x7f5, 0x7ef, 0x7ea, 0x7e4, 0x7df, 0x7da, 0x7d4,
	0x7cf, 0x7c9, 0x7c4, 0x7bf, 0x7b9, 0x7b4, 0x7ae, 0x7a9,
	0x7a4, 0x79f, 0x799, 0x794, 0x78f, 0x78a, 0x784, 0x77f,
	0x77a, 0x775, 0x770, 0x76a, 0x765, 0x760, 0x75b, 0x756,
	0x751, 0x74c, 0x747, 0x742, 0x73d, 0x738, 0x733, 0x72e,
	0x729, 0x724, 0x71f, 0x71a, 0x715, 0x710, 0x70b, 0x706,
	0x702, 0x6fd, 0x6f8, 0x6f3, 0x6ee, 0x6e9, 0x6e5, 0x6e0,
	0x6db, 0x6d6, 0x6d2, 0x6cd, 0x6c8, 0x6c4, 0x6bf, 0x6ba,
	0x6b5, 0x6b1, 0x6ac, 0x6a8, 0x6a3, 0x69e, 0x69a, 0x695,
	0x691, 0x68c, 0x688, 0x683, 0x67f, 0x67a, 0x676, 0x671,
	0x66d, 0x668, 0x664, 0x65f, 0x65b, 0x657, 0x652, 0x64e,
	0x649, 0x645, 0x641, 0x63c, 0x638, 0x634, 0x630, 0x62b,
	0x627, 0x623, 0x61e, 0x61a, 0x616, 0x612, 0x60e, 0x609,
	0x605, 0x601, 0x5fd, 0x5f9, 0x5f5, 0x5f0, 0x5ec, 0x5e8,
	0x5e4, 0x5e0, 0x5dc, 0x5d8, 0x5d4, 0x5d0, 0x5cc, 0x5c8,
	0x5c4, 0x5c0, 0x5bc, 0x5b8, 0x5b4, 0x5b0, 0x5ac, 0x5a8,
	0x5a4, 0x5a0, 0x59c, 0x599, 0x595, 0x591, 0x58d, 0x589,
	0x585, 0x581, 0x57e, 0x57a, 0x576, 0x572, 0x56f, 0x56b,
	0x567, 0x563, 0x560, 0x55c, 0x558, 0x554, 0x551, 0x54d,
	0x549, 0x546, 0x542, 0x53e, 0x53b, 0x537, 0x534, 0x530,
	0x52c, 0x529, 0x525, 0x522, 0x51e, 0x51b, 0x517, 0x514,
	0x510, 0x50c, 0x509, 0x506, 0x502, 0x4ff, 0x4fb, 0x4f8,
	0x4f4, 0x4f1, 0x4ed, 0x4ea, 0x4e7, 0x4e3, 0x4e0, 0x4dc,
	0x4d9, 0x4d6, 0x4d2, 0x4cf, 0x4cc, 0x4c8, 0x4c5, 0x4c2,
	0x4be, 0x4bb, 0x4b8, 0x4b5, 0x4b1, 0x4ae, 0x4ab, 0x4a8,
	0x4a4, 0x4a1, 0x49e, 0x49b, 0x498, 0x494, 0x491, 0x48e,
	0x48b, 0x488, 0x485, 0x482, 0x47e, 0x47b, 0x478, 0x475,
	0x472, 0x46f, 0x46c, 0x469, 0x466, 0x463, 0x460, 0x45d,
	0x45a, 0x457, 0x454, 0x451, 0x44e, 0x44b, 0x448, 0x445,
	0x442, 0x43f, 0x43c, 0x439, 0x436, 0x433, 0x430, 0x42d,
	0x42a, 0x428, 0x425, 0x422, 0x41f, 0x41c, 0x419, 0x416,
	0x414, 0x411, 0x40e, 0x40b, 0x408, 0x406, 0x403, 0x400
};

/*
 * Frequency multiplier table multiplied by 2; taken straight from Nuked OPL3
 * source code.
 */
static const uint8_t mt[16] = {
	1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 20, 24, 24, 30, 30
};

/*
 * This is used during the envelope generation to apply KSL to the envelope by
 * determining how much to shift right the keyscale attenuation value before
 * adding it to the envelope level.
 */
static const uint8_t kslshift[4] = {
	8, 1, 2, 0
};

/*
 * This encodes which emulation mode channels are the secondary channel in a
 * 4-op channel pair (where the entry is non-negative), and which is the
 * corresponding primary channel for that secondary channel.
 */
static const int emu_4op_secondary_to_primary[18] =
{
	-1, -1, -1, 0,  1,  2, -1, -1, -1,
	-1, -1, -1, 9, 10, 11, -1, -1, -1
};

/*
 * Envelope generator dither table, taken straight from Nuked OPL3 source code.
 */
static const uint8_t eg_incstep[4][4] = {
	{ 0, 0, 0, 0 },
	{ 1, 0, 0, 0 },
	{ 1, 0, 1, 0 },
	{ 1, 1, 1, 0 }
};

typedef int13(*envelope_sinfunc)(uint10 phase, uint10 envelope);

/* ------------------------------------------------------------------------- */
static uint12
ESFM_envelope_calc_exp(uint16 level)
{
	if (level > 0x1fff)
	{
		level = 0x1fff;
	}
	return (exprom[level & 0xff] << 1) >> (level >> 8);
}

/* ------------------------------------------------------------------------- */
static int13
ESFM_envelope_calc_sin0(uint10 phase, uint10 envelope)
{
	uint16 out = 0;
	int13 neg = 1;
	phase &= 0x3ff;
	if (phase & 0x200)
	{
		neg = -1;
	}
	if (phase & 0x100)
	{
		out = logsinrom[(phase & 0xff) ^ 0xff];
	}
	else
	{
		out = logsinrom[phase & 0xff];
	}
	return ESFM_envelope_calc_exp(out + (envelope << 3)) * neg;
}

/* ------------------------------------------------------------------------- */
static int13
ESFM_envelope_calc_sin1(uint10 phase, uint10 envelope)
{
	uint16 out = 0;
	phase &= 0x3ff;
	if (phase & 0x200)
	{
		out = 0x1000;
	}
	else if (phase & 0x100)
	{
		out = logsinrom[(phase & 0xff) ^ 0xff];
	}
	else
	{
		out = logsinrom[phase & 0xff];
	}
	return ESFM_envelope_calc_exp(out + (envelope << 3));
}

/* ------------------------------------------------------------------------- */
static int13
ESFM_envelope_calc_sin2(uint10 phase, uint10 envelope)
{
	uint16 out = 0;
	phase &= 0x3ff;
	if (phase & 0x100)
	{
		out = logsinrom[(phase & 0xff) ^ 0xff];
	}
	else
	{
		out = logsinrom[phase & 0xff];
	}
	return ESFM_envelope_calc_exp(out + (envelope << 3));
}

/* ------------------------------------------------------------------------- */
static int13
ESFM_envelope_calc_sin3(uint10 phase, uint10 envelope)
{
	uint16 out = 0;
	phase &= 0x3ff;
	if (phase & 0x100)
	{
		out = 0x1000;
	}
	else
	{
		out = logsinrom[phase & 0xff];
	}
	return ESFM_envelope_calc_exp(out + (envelope << 3));
}

/* ------------------------------------------------------------------------- */
static int13
ESFM_envelope_calc_sin4(uint10 phase, uint10 envelope)
{
	uint16 out = 0;
	int13 neg = 1;
	phase &= 0x3ff;
	if ((phase & 0x300) == 0x100)
	{
		neg = -1;
	}
	if (phase & 0x200)
	{
		out = 0x1000;
	}
	else if (phase & 0x80)
	{
		out = logsinrom[((phase ^ 0xff) << 1) & 0xff];
	}
	else
	{
		out = logsinrom[(phase << 1) & 0xff];
	}
	return ESFM_envelope_calc_exp(out + (envelope << 3)) * neg;
}

/* ------------------------------------------------------------------------- */
static int13
ESFM_envelope_calc_sin5(uint10 phase, uint10 envelope)
{
	uint16 out = 0;
	phase &= 0x3ff;
	if (phase & 0x200)
	{
		out = 0x1000;
	}
	else if (phase & 0x80)
	{
		out = logsinrom[((phase ^ 0xff) << 1) & 0xff];
	}
	else
	{
		out = logsinrom[(phase << 1) & 0xff];
	}
	return ESFM_envelope_calc_exp(out + (envelope << 3));
}

/* ------------------------------------------------------------------------- */
static int13
ESFM_envelope_calc_sin6(uint10 phase, uint10 envelope)
{
	int13 neg = 1;
	phase &= 0x3ff;
	if (phase & 0x200)
	{
		neg = -1;
	}
	return ESFM_envelope_calc_exp(envelope << 3) * neg;
}

/* ------------------------------------------------------------------------- */
static int13
ESFM_envelope_calc_sin7(uint10 phase, uint10 envelope)
{
	uint16 out = 0;
	int13 neg = 1;
	phase &= 0x3ff;
	if (phase & 0x200)
	{
		neg = -1;
		phase = (phase & 0x1ff) ^ 0x1ff;
	}
	out = phase << 3;
	return ESFM_envelope_calc_exp(out + (envelope << 3)) * neg;
}

/* ------------------------------------------------------------------------- */
static const envelope_sinfunc envelope_sin[8] =
{
	ESFM_envelope_calc_sin0,
	ESFM_envelope_calc_sin1,
	ESFM_envelope_calc_sin2,
	ESFM_envelope_calc_sin3,
	ESFM_envelope_calc_sin4,
	ESFM_envelope_calc_sin5,
	ESFM_envelope_calc_sin6,
	ESFM_envelope_calc_sin7
};

/* ------------------------------------------------------------------------- */
static void
ESFM_envelope_calc(esfm_slot *slot)
{
	uint8 nonzero;
	uint8 rate;
	uint5 rate_hi;
	uint2 rate_lo;
	uint4 reg_rate = 0;
	uint4 ks;
	uint8 eg_shift, shift;
	bool eg_off;
	uint9 eg_rout;
	int16 eg_inc;
	bool reset = 0;
	bool key_on;
	bool key_on_signal;

	key_on = *slot->in.key_on;
	if (!slot->chip->native_mode)
	{
		int pair_primary_idx = emu_4op_secondary_to_primary[slot->channel->channel_idx];
		if (pair_primary_idx >= 0)
		{
			esfm_channel *pair_primary = &slot->channel->chip->channels[pair_primary_idx];
			if (pair_primary->emu_mode_4op_enable)
			{
				key_on = *pair_primary->slots[0].in.key_on;
			}
		}
		else if ((slot->channel->channel_idx == 7 || slot->channel->channel_idx == 8)
			&& slot->slot_idx == 1)
		{
			key_on = slot->channel->key_on_2;
		}
	}

	slot->in.eg_output = slot->in.eg_position + (slot->t_level << 2)
		+ (slot->in.eg_ksl_offset >> kslshift[slot->ksl]);
	if (slot->tremolo_en)
	{
		uint8 tremolo;
		if (slot->chip->native_mode)
		{
			tremolo = slot->channel->chip->tremolo >> ((!slot->tremolo_deep << 1) + 2);
		}
		else
		{
			tremolo = slot->channel->chip->tremolo >> ((!slot->chip->emu_tremolo_deep << 1) + 2);
		}
		slot->in.eg_output += tremolo;
	}
	
	if (slot->in.eg_delay_run && slot->in.eg_delay_counter < 32768)
	{
		slot->in.eg_delay_counter++;
	}
	
	// triggers on key-on edge
	if (key_on && !slot->in.key_on_gate)
	{
		slot->in.eg_delay_run = 1;
		slot->in.eg_delay_counter = 0;
		slot->in.eg_delay_transitioned_01 = 0;
		slot->in.eg_delay_transitioned_01_gate = 0;
		slot->in.eg_delay_transitioned_10 = 0;
		slot->in.eg_delay_transitioned_10_gate = 0;
		slot->in.eg_delay_counter_compare = 0;
		if (slot->env_delay > 0)
		{
			slot->in.eg_delay_counter_compare = 256 << slot->env_delay;
		}
	}
	else if (!key_on)
	{
		slot->in.eg_delay_run = 0;
	}
	
	// TODO: is this really how the chip behaves? Can it only transition the envelope delay once? Am I implementing this in a sane way? I feel like this is a roundabout hack.
	if ((slot->in.eg_delay_transitioned_10 && !slot->in.eg_delay_transitioned_10_gate) ||
		(slot->in.eg_delay_transitioned_01 && !slot->in.eg_delay_transitioned_01_gate)
	)
	{
		slot->in.eg_delay_counter_compare = 0;
		if (slot->env_delay > 0)
		{
			slot->in.eg_delay_counter_compare = 256 << slot->env_delay;
		}
		if (slot->in.eg_delay_transitioned_10)
		{
			slot->in.eg_delay_transitioned_10_gate = 1;
		}
		if (slot->in.eg_delay_transitioned_01)
		{
			slot->in.eg_delay_transitioned_01_gate = 1;
		}
	}
	
	if (key_on && ((slot->in.eg_delay_counter >= slot->in.eg_delay_counter_compare) || !slot->chip->native_mode))
	{
		key_on_signal = 1;
	} else {
		key_on_signal = 0;
	}
	
	if (key_on && slot->in.eg_state == EG_RELEASE)
	{

		if ((slot->in.eg_delay_counter >= slot->in.eg_delay_counter_compare) || !slot->chip->native_mode)
		{
			reset = 1;
			reg_rate = slot->attack_rate;
		}
		else
		{
			reg_rate = slot->release_rate;
		}
	}
	else
	{
		switch (slot->in.eg_state)
		{
			case EG_ATTACK:
				reg_rate = slot->attack_rate;
				break;
			case EG_DECAY:
				reg_rate = slot->decay_rate;
				break;
			case EG_SUSTAIN:
				if (!slot->env_sustaining)
				{
					reg_rate = slot->release_rate;
				}
				break;
			case EG_RELEASE:
				reg_rate = slot->release_rate;
				break;
		}
	}
	slot->in.key_on_gate = key_on;
	slot->in.phase_reset = reset;
	ks = slot->in.keyscale >> ((!slot->ksr) << 1);
	nonzero = (reg_rate != 0);
	rate = ks + (reg_rate << 2);
	rate_hi = rate >> 2;
	rate_lo = rate & 0x03;
	if (rate_hi & 0x10)
	{
		rate_hi = 0x0f;
	}
	eg_shift = rate_hi + slot->chip->eg_clocks;
	shift = 0;
	if (nonzero)
	{
		if (rate_hi < 12)
		{
			if (slot->chip->eg_tick)
			{
				switch (eg_shift)
				{
					case 12:
						shift = 1;
						break;
					case 13:
						shift = (rate_lo >> 1) & 0x01;
						break;
					case 14:
						shift = rate_lo & 0x01;
						break;
					default:
						break;
				}
			}
		}
		else
		{
			shift = (rate_hi & 0x03)
				+ eg_incstep[rate_lo][slot->chip->global_timer & 0x03];
			if (shift & 0x04)
			{
				shift = 0x03;
			}
			if (!shift)
			{
				shift = slot->chip->eg_tick;
			}
		}
	}
	eg_rout = slot->in.eg_position;
	eg_inc = 0;
	eg_off = 0;
	/* Instant attack */
	if (reset && rate_hi == 0x0f)
	{
		eg_rout = 0x00;
	}
	/* Envelope off */
	if ((slot->in.eg_position & 0x1f8) == 0x1f8)
	{
		eg_off = 1;
	}
	if (slot->in.eg_state != EG_ATTACK && !reset && eg_off)
	{
		eg_rout = 0x1ff;
	}
	switch (slot->in.eg_state)
	{
		case EG_ATTACK:
			if (slot->in.eg_position == 0)
			{
				slot->in.eg_state = EG_DECAY;
			}
			else if (key_on_signal && shift > 0 && rate_hi != 0x0f)
			{
				eg_inc = ~slot->in.eg_position >> (4 - shift);
			}
			break;
		case EG_DECAY:
			if ((slot->in.eg_position >> 4) == slot->sustain_lvl)
			{
				slot->in.eg_state = EG_SUSTAIN;
			}
			else if (!eg_off && !reset && shift > 0)
			{
				eg_inc = 1 << (shift - 1);
			}
			break;
		case EG_SUSTAIN:
		case EG_RELEASE:
			if (!eg_off && !reset && shift > 0)
			{
				eg_inc = 1 << (shift - 1);
			}
			break;
	}
	slot->in.eg_position = (eg_rout + eg_inc) & 0x1ff;
	/* Key off */
	if (reset)
	{
		slot->in.eg_state = EG_ATTACK;
	}
	if (!key_on_signal)
	{
		slot->in.eg_state = EG_RELEASE;
	}
}

/* ------------------------------------------------------------------------- */
static void
ESFM_phase_generate(esfm_slot *slot)
{
	esfm_chip *chip;
	uint10 f_num;
	uint32 basefreq;
	bool rm_xor, n_bit;
	uint23 noise;
	uint10 phase;

	chip = slot->chip;
	f_num = slot->f_num;
	if (slot->vibrato_en)
	{
		int8_t range;
		uint8_t vibpos;

		range = (f_num >> 7) & 7;
		vibpos = chip->vibrato_pos;

		if (!(vibpos & 3))
		{
			range = 0;
		}
		else if (vibpos & 1)
		{
			range >>= 1;
		}
		range >>= !slot->vibrato_deep;

		if (vibpos & 4)
		{
			range = -range;
		}
		f_num += range;
	}
	basefreq = (f_num << slot->block) >> 1;
	phase = (uint10)(slot->in.phase_acc >> 9);
	if (slot->in.phase_reset)
	{
		slot->in.phase_acc = 0;
	}
	slot->in.phase_acc += (basefreq * mt[slot->mult]) >> 1;
	slot->in.phase_acc &= (1 << 19) - 1;
	slot->in.phase_out = phase;
	/* Noise mode (rhythm) sounds */
	noise = chip->lfsr;
	if (slot->slot_idx == 3 && slot->rhy_noise)
	{
		esfm_slot *prev_slot = &slot->channel->slots[2];

		chip->rm_hh_bit2 = (phase >> 2) & 1;
		chip->rm_hh_bit3 = (phase >> 3) & 1;
		chip->rm_hh_bit7 = (phase >> 7) & 1;
		chip->rm_hh_bit8 = (phase >> 8) & 1;

		chip->rm_tc_bit3 = (prev_slot->in.phase_out >> 3) & 1;
		chip->rm_tc_bit5 = (prev_slot->in.phase_out >> 5) & 1;

		rm_xor = (chip->rm_hh_bit2 ^ chip->rm_hh_bit7)
		       | (chip->rm_hh_bit3 ^ chip->rm_tc_bit5)
		       | (chip->rm_tc_bit3 ^ chip->rm_tc_bit5);

		switch(slot->rhy_noise)
		{
			case 1:
				// SD
				slot->in.phase_out = (chip->rm_hh_bit8 << 9)
					| ((chip->rm_hh_bit8 ^ (noise & 1)) << 8);
				break;
			case 2:
				// HH
				slot->in.phase_out = rm_xor << 9;
				if (rm_xor ^ (noise & 1))
				{
					slot->in.phase_out |= 0xd0;
				}
				else
				{
					slot->in.phase_out |= 0x34;
				}
				break;
			case 3:
				// TC
				slot->in.phase_out = (rm_xor << 9) | 0x80;
				break;
		}
	}

	n_bit = ((noise >> 14) ^ noise) & 0x01;
	chip->lfsr = (noise >> 1) | (n_bit << 22);
}

/* ------------------------------------------------------------------------- */
static void
ESFM_phase_generate_emu(esfm_slot *slot)
{
	esfm_chip *chip;
	uint3 block;
	uint10 f_num;
	uint32 basefreq;
	bool rm_xor, n_bit;
	uint23 noise;
	uint10 phase;
	int pair_primary_idx;

	chip = slot->chip;
	block = slot->channel->slots[0].block;
	f_num = slot->channel->slots[0].f_num;

	pair_primary_idx = emu_4op_secondary_to_primary[slot->channel->channel_idx];
	if (pair_primary_idx >= 0)
	{
		esfm_channel *pair_primary = &slot->channel->chip->channels[pair_primary_idx];
		if (pair_primary->emu_mode_4op_enable)
		{
			block = pair_primary->slots[0].block;
			f_num = pair_primary->slots[0].f_num;
		}
	}

	if (slot->vibrato_en)
	{
		int8_t range;
		uint8_t vibpos;

		range = (f_num >> 7) & 7;
		vibpos = chip->vibrato_pos;

		if (!(vibpos & 3))
		{
			range = 0;
		}
		else if (vibpos & 1)
		{
			range >>= 1;
		}
		range >>= !chip->emu_vibrato_deep;

		if (vibpos & 4)
		{
			range = -range;
		}
		f_num += range;
	}
	basefreq = (f_num << block) >> 1;
	phase = (uint10)(slot->in.phase_acc >> 9);
	if (slot->in.phase_reset)
	{
		slot->in.phase_acc = 0;
	}
	slot->in.phase_acc += (basefreq * mt[slot->mult]) >> 1;
	slot->in.phase_acc &= (1 << 19) - 1;
	slot->in.phase_out = phase;

	/* Noise mode (rhythm) sounds */
	noise = chip->lfsr;
	// HH
	if (slot->channel->channel_idx == 7 && slot->slot_idx == 0)
	{
		chip->rm_hh_bit2 = (phase >> 2) & 1;
		chip->rm_hh_bit3 = (phase >> 3) & 1;
		chip->rm_hh_bit7 = (phase >> 7) & 1;
		chip->rm_hh_bit8 = (phase >> 8) & 1;
	}
	// TC
	if (slot->channel->channel_idx == 8 && slot->slot_idx == 1)
	{
		chip->rm_tc_bit3 = (phase >> 3) & 1;
		chip->rm_tc_bit5 = (phase >> 5) & 1;
	}
	if (chip->emu_rhy_mode_flags & 0x20)
	{
		rm_xor = (chip->rm_hh_bit2 ^ chip->rm_hh_bit7)
		       | (chip->rm_hh_bit3 ^ chip->rm_tc_bit5)
		       | (chip->rm_tc_bit3 ^ chip->rm_tc_bit5);
		if (slot->channel->channel_idx == 7)
		{
			if (slot->slot_idx == 0) {
				// HH
				slot->in.phase_out = rm_xor << 9;
				if (rm_xor ^ (noise & 1))
				{
					slot->in.phase_out |= 0xd0;
				}
				else
				{
					slot->in.phase_out |= 0x34;
				}
			}
			else if (slot->slot_idx == 1)
			{
				// SD
				slot->in.phase_out = (chip->rm_hh_bit8 << 9)
					| ((chip->rm_hh_bit8 ^ (noise & 1)) << 8);
			}
		}
		else if (slot->channel->channel_idx == 8 && slot->slot_idx == 1)
		{
			// TC
			slot->in.phase_out = (rm_xor << 9) | 0x80;
		}
	}

	n_bit = ((noise >> 14) ^ noise) & 0x01;
	chip->lfsr = (noise >> 1) | (n_bit << 22);
}

/**
 * TODO: Figure out what's ACTUALLY going on inside the real chip!
 * This is not accurate at all, but it's the closest I was able to get with
 * empirical testing (and it's closer than nothing).
 */
/* ------------------------------------------------------------------------- */
static int16
ESFM_slot3_noise3_mod_input_calc(esfm_slot *slot)
{
	esfm_channel *channel = slot->channel;
	envelope_sinfunc wavegen = envelope_sin[channel->slots[2].waveform];
	int16 phase;
	int13 output_buf = *channel->slots[1].in.mod_input;
	int i;
	
	// Go through previous slots' partial results and recalculate outputs
	// (we skip slot 0 because its calculation happens at the end, not at the beginning)
	for (i = 1; i < 3; i++)
	{
		// double the pitch
		phase = channel->slots[i].in.phase_acc >> 8;
		if (channel->slots[i].mod_in_level)
		{
			phase += output_buf >> (7 - channel->slots[i].mod_in_level);
		}
		output_buf = wavegen((uint10)(phase & 0x3ff), channel->slots[i].in.eg_output);
	}

	return output_buf >> (8 - slot->mod_in_level);
}

/* ------------------------------------------------------------------------- */
static void
ESFM_slot_generate(esfm_slot *slot)
{
	envelope_sinfunc wavegen = envelope_sin[slot->waveform];
	int16 phase = slot->in.phase_out;
	if (slot->mod_in_level)
	{
		if (slot->slot_idx == 3 && slot->rhy_noise == 3)
		{
			phase += ESFM_slot3_noise3_mod_input_calc(slot);
		}
		else
		{
			phase += *slot->in.mod_input >> (7 - slot->mod_in_level);
		}
	}
	slot->in.output = wavegen((uint10)(phase & 0x3ff), slot->in.eg_output);
	if (slot->output_level)
	{
		int13 output_value = slot->in.output >> (7 - slot->output_level);
		slot->channel->output[0] += output_value & slot->out_enable[0];
		slot->channel->output[1] += output_value & slot->out_enable[1];
	}
}

/* ------------------------------------------------------------------------- */
static void
ESFM_slot_generate_emu(esfm_slot *slot)
{
	esfm_chip *chip = slot->chip;
	envelope_sinfunc wavegen = envelope_sin[
		slot->waveform & (chip->emu_newmode != 0 ? 0x07 : 0x03)];
	bool rhythm_slot_double_volume = (slot->chip->emu_rhy_mode_flags & 0x20) != 0
		&& slot->channel->channel_idx >= 6 && slot->channel->channel_idx < 9;
	int16 phase = slot->in.phase_out;
	int14 output_value;

	phase += *slot->in.mod_input & slot->in.emu_mod_enable;
	slot->in.output = wavegen((uint10)(phase & 0x3ff), slot->in.eg_output);
	output_value = (slot->in.output & slot->in.emu_output_enable) << rhythm_slot_double_volume;
	if (chip->emu_newmode)
	{
		slot->channel->output[0] += output_value & slot->channel->slots[0].out_enable[0];
		slot->channel->output[1] += output_value & slot->channel->slots[0].out_enable[1];
	}
	else
	{
		slot->channel->output[0] += output_value;
		slot->channel->output[1] += output_value;
	}
}

/* ------------------------------------------------------------------------- */
static void
ESFM_slot_calc_feedback(esfm_slot *slot)
{
	esfm_chip *chip = slot->chip;
	uint32 basefreq, phase_offset;
	uint3 block;
	uint10 f_num;
	int13 in1 = 0, in2 = 0, wave_out;
	int16 phase, phase_feedback;
	uint19 regressed_phase;
	int iter_counter;
	envelope_sinfunc wavegen;

	if (slot->mod_in_level)
	{
		if (chip->native_mode)
		{
			wavegen = envelope_sin[slot->waveform];
		}
		else
		{
			wavegen = envelope_sin[slot->waveform & (0x03 | (0x02 << (chip->emu_newmode != 0)))];
		}
		f_num = slot->f_num;
		block = slot->block;
		basefreq = (f_num << block) >> 1;
		phase_offset = (basefreq * mt[slot->mult]) >> 1;

		for (iter_counter = 28; iter_counter >= 0; iter_counter--)
		{
			regressed_phase = (uint19)((uint32)slot->in.phase_acc - iter_counter * phase_offset) & ((1 << 19) - 1);
			phase = (int16)(regressed_phase >> 9);
			phase_feedback = (in1 + in2) >> 2;
			phase += phase_feedback >> (7 - slot->mod_in_level);
			wave_out = wavegen((uint10)(phase & 0x3ff), slot->in.eg_output);
			in2 = in1;
			in1 = wave_out;
		}

		// TODO: Figure out - is this how the ESFM chip does it, like the
		// patent literally says? (it's really hacky...)
		//   slot->in.output = wave_out;

		// This would be the more canonical way to do it, reusing the rest of
		// the synthesis pipeline to finish the calculation:
		if (chip->native_mode)
		{
			slot->in.feedback_buf = phase_feedback;
		}
		else
		{
			slot->in.feedback_buf = phase_feedback >> (7 - slot->mod_in_level);
		}
	}
}

/* ------------------------------------------------------------------------- */
static void
ESFM_process_channel(esfm_channel *channel)
{
	int slot_idx;
	channel->output[0] = channel->output[1] = 0;
	for (slot_idx = 0; slot_idx < 4; slot_idx++)
	{
		esfm_slot *slot = &channel->slots[slot_idx];
		ESFM_envelope_calc(slot);
		ESFM_phase_generate(slot);
		if(slot_idx > 0)
		{
			ESFM_slot_generate(slot);
		}
	}
	// ESFM feedback calculation takes a large number of clock cycles, so
	// defer slot 0 generation to the end
	// TODO: verify this behavior on real hardware
	ESFM_slot_calc_feedback(&channel->slots[0]);
	ESFM_slot_generate(&channel->slots[0]);
}

/* ------------------------------------------------------------------------- */
static void
ESFM_process_channel_emu(esfm_channel *channel)
{
	int slot_idx;
	channel->output[0] = channel->output[1] = 0;
	for (slot_idx = 0; slot_idx < 2; slot_idx++)
	{
		esfm_slot *slot = &channel->slots[slot_idx];
		ESFM_envelope_calc(slot);
		ESFM_phase_generate_emu(slot);
		if(slot_idx > 0)
		{
			ESFM_slot_generate_emu(slot);
		}
	}
	// ESFM feedback calculation takes a large number of clock cycles, so
	// defer slot 0 generation to the end
	// TODO: verify this behavior on real hardware
	if (channel->slots[0].in.mod_input == &channel->slots[0].in.feedback_buf)
	{
		ESFM_slot_calc_feedback(&channel->slots[0]);
	}
	ESFM_slot_generate_emu(&channel->slots[0]);
}

/* ------------------------------------------------------------------------- */
static int16_t
ESFM_clip_sample(int32 sample)
{
	// TODO: Supposedly, the real ESFM chip actually overflows rather than
	// clipping. Verify that.
	if (sample > 32767)
	{
		sample = 32767;
	}
	else if (sample < -32768)
	{
		sample = -32768;
	}
	return (int16_t)sample;
}

/* ------------------------------------------------------------------------- */
static void
ESFM_update_timers(esfm_chip *chip)
{
	// Tremolo
	if ((chip->global_timer & 0x3f) == 0x3f)
	{
		chip->tremolo_pos = (chip->tremolo_pos + 1) % 210;
		if (chip->tremolo_pos < 105)
		{
			chip->tremolo = chip->tremolo_pos;
		}
		else
		{
			chip->tremolo = (210 - chip->tremolo_pos);
		}
	}

	// Vibrato
	if ((chip->global_timer & 0x3ff) == 0x3ff)
	{
		chip->vibrato_pos = (chip->vibrato_pos + 1) & 0x07;
	}

	chip->global_timer = (chip->global_timer + 1) & 0x3ff;

	// Envelope generator dither clocks
	chip->eg_clocks = 0;
	if (chip->eg_timer)
	{
		uint8 shift = 0;
		while (shift < 36 && ((chip->eg_timer >> shift) & 1) == 0)
		{
			shift++;
		}

		if (shift <= 12)
		{
			chip->eg_clocks = shift + 1;
		}
	}

	if (chip->eg_tick || chip->eg_timer_overflow)
	{
		if (chip->eg_timer == (1llu << 36) - 1)
		{
			chip->eg_timer = 0;
			chip->eg_timer_overflow = 1;
		}
		else
		{
			chip->eg_timer++;
			chip->eg_timer_overflow = 0;
		}
	}

	chip->eg_tick ^= 1;
}

#define KEY_ON_REGS_START (18 * 4 * 8)
/* ------------------------------------------------------------------------- */
int
ESFM_reg_write_chan_idx(esfm_chip *chip, uint16_t reg)
{
	int which_reg = -1;
	if (chip->native_mode)
	{
		bool is_key_on_reg = reg >= KEY_ON_REGS_START && reg < (KEY_ON_REGS_START + 20);
		if (is_key_on_reg)
		{
			which_reg = reg - KEY_ON_REGS_START;
		}
	}
	else
	{
		uint8_t reg_low = reg & 0xff;
		bool high = reg & 0x100;
		bool is_key_on_reg = reg_low >= 0xb0 && reg_low < 0xb9;
		if (is_key_on_reg)
		{
			which_reg = (reg_low & 0x0f) + high * 9;
		}
	}

	return which_reg;
}

/* ------------------------------------------------------------------------- */
void
ESFM_update_write_buffer(esfm_chip *chip)
{
	esfm_write_buf *write_buf;
	bool note_off_written[20];
	bool bassdrum_written = false;
	int i;
	for (i = 0; i < 20; i++)
	{
		note_off_written[i] = false;
	}
	while((write_buf = &chip->write_buf[chip->write_buf_start]),
		write_buf->valid && write_buf->timestamp <= chip->write_buf_timestamp)
	{
		int is_which_note_on_reg =
			ESFM_reg_write_chan_idx(chip, write_buf->address);
		if (is_which_note_on_reg >= 0)
		{
			if ((chip->native_mode && (write_buf->data & 0x01) == 0)
				|| (!chip->native_mode && (write_buf->data & 0x20) == 0)
			)
			{
				// this is a note off command; note down that we got note off for this channel
				note_off_written[is_which_note_on_reg] = true;
			}
			else
			{
				// this is a note on command; have we gotten a note off for this channel in this cycle?
				if (note_off_written[is_which_note_on_reg])
				{
					// we have a conflict; let the note off be processed first and defer the
					// rest of the buffer to the next cycle
					break;
				}
			}
		}
		if ((chip->native_mode && write_buf->address == 0x4bd)
			|| (!chip->native_mode && (write_buf->address & 0xff) == 0xbd)
		)
		{
			// bassdrum register write (rhythm mode note-on/off control)
			// have we already written to the bassdrum register in this cycle
			if (bassdrum_written) {
				// we have a conflict
				break;
			}
			bassdrum_written = true;
		}

		write_buf->valid = 0;
		ESFM_write_reg(chip, write_buf->address, write_buf->data);
		chip->write_buf_start = (chip->write_buf_start + 1) % ESFM_WRITEBUF_SIZE;
	}

	chip->write_buf_timestamp++;
}

/* ------------------------------------------------------------------------- */
void
ESFM_generate(esfm_chip *chip, int16_t *buf)
{
	int channel_idx;

	chip->output_accm[0] = chip->output_accm[1] = 0;
	for (channel_idx = 0; channel_idx < 18; channel_idx++)
	{
		esfm_channel *channel = &chip->channels[channel_idx];
		if (chip->native_mode)
		{
			ESFM_process_channel(channel);
		}
		else
		{
			ESFM_process_channel_emu(channel);
		}

		chip->output_accm[0] += channel->output[0];
		chip->output_accm[1] += channel->output[1];
	}

	buf[0] = ESFM_clip_sample(chip->output_accm[0]);
	buf[1] = ESFM_clip_sample(chip->output_accm[1]);

	ESFM_update_timers(chip);
	ESFM_update_write_buffer(chip);
}

/* ------------------------------------------------------------------------- */
int16_t
ESFM_get_channel_output_native(esfm_chip *chip, int channel_idx)
{
	int16_t result;
	int32_t temp_mix = 0;
	int i;
	
	if (channel_idx < 0 || channel_idx >= 18)
	{
		return 0;
	}
	
	for (i = 0; i < 4; i++)
	{
		esfm_slot *slot = &chip->channels[channel_idx].slots[i];
		
		if (slot->output_level)
		{
			int13 output_value = slot->in.output >> (7 - slot->output_level);
			temp_mix += output_value & slot->out_enable[0];
			temp_mix += output_value & slot->out_enable[1];
		}
	}
	
	if (temp_mix > 32767)
	{
		temp_mix = 32767;
	}
	else if (temp_mix < -32768)
	{
		temp_mix = -32768;
	}
	result = temp_mix;
	return result;
}

/* ------------------------------------------------------------------------- */
void
ESFM_generate_stream(esfm_chip *chip, int16_t *sndptr, uint32_t num_samples)
{
	uint32_t i;

	for (i = 0; i < num_samples; i++)
	{
		ESFM_generate(chip, sndptr);
		sndptr += 2;
	}
}
