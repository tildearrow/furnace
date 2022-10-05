//	Altirra - Atari 800/800XL/5200 emulator
//	Copyright (C) 2008-2018 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef f_AT_POKEYTABLES_H
#define f_AT_POKEYTABLES_H

#include <stdint.h>

struct ATPokeyTables {
	// Rate of decay per sample (28 cycles) for the first stage amplifier output. This
	// affects POKEY output but not GTIA (CONSOL) output.
	float	mReferenceDecayPerSample;

	// Min/max clamps for first stage amplifier output. These are in terms of final output
	// polarity, so negated from volume values.
	float	mReferenceClampLo;
	float	mReferenceClampHi;

	float	mSpeakerLevel;

	// Volume table for sums of all four audio channels.
	//
	// The index for this table is constructed as follows:
	//
	//   index = sum(v[i] & 1) + 5*sum((v[i] & 2)/2) + 25*sum((v[i] & 12) / 4)
	//
	// The packed fields are thus: the number of channels with volume bit 0 set, the
	// number of channels with volume bit 1 set, and the number of volume bit 2 equivalent
	// steps for all channels. The last part takes advantage of the measurement for bit 3
	// being close enough to twice bit 2 that we can combine the two to reduce table size.
	// With this formulation, the POKEY channel volume levels can be translated to index
	// deltas that are just added together.
	//
	// Also in this table is the saturation curve, which kicks in roughly at volume level
	// 12 of a single channel, and negation, so the output is the same polarity as on
	// the actual hardware (POKEY pulls down from ~4.80V).
	//
	// Finally, the result is divided by 56 to account for output being integrated across
	// 56 half-cycles for each sample.
	//
	float	mMixTable[325];

	// Bit 0 = 17-bit polynomial
	// Bit 1 = 9-bit polynomial
	// Bit 2 = 5-bit polynomial
	// Bit 3 = 4-bit polynomial
	uint8_t	mPolyBuffer[131071 * 2];
	uint8_t	mInitModeBuffer[131071 * 2];

	ATPokeyTables();
};

// High-resolution filter table.
//
// This is used to resample from the 2x rate that audio events are generated
// at (3.54/3.58MHz) to the 1/28 mixing rate of 64KHz. It is a filter bank of
// 8-tap filter kernels at 56 sub-sample offsets, giving the response of the
// filter for a single half-tick impulse. Each transition in POKEY's output
// is converted to a half-tick edge pulse and the resampled through one of
// the 8-tap filters into the edge buffer.

struct ATPokeyHiFilterTable {
	// This table is anti-symmetric, so we only need half of the kernel -- the
	// other half is generated on the fly via reflection. Size: 928 bytes.
	alignas(64) float mFilter[29][8];
};

extern const ATPokeyHiFilterTable g_ATPokeyHiFilterTable;

#endif	// f_AT_POKEYTABLES_H
