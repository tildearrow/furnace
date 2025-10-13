// BSD 3-Clause License
//
// Copyright (c) 2021, Aaron Giles
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "ymfm_opnx.h"
#include "ymfm_fm.ipp"

namespace ymfm
{

//*********************************************************
//  OPN/OPNA REGISTERS
//*********************************************************

//-------------------------------------------------
//  opnx_registers - constructor
//-------------------------------------------------

opnx_registers::opnx_registers() :
	m_pclfo{ pclfo_t(), pclfo_t() },
	m_lfo_counter(0),
	m_lfo_am(0)
{
	// create these pointers to appease overzealous compilers checking array
	// bounds in unreachable code (looking at you, clang)
	uint16_t *wf0 = &m_waveform[0][0];
	uint16_t *wf1 = &m_waveform[1 % WAVEFORMS][0];
	uint16_t *wf2 = &m_waveform[2 % WAVEFORMS][0];
	uint16_t *wf3 = &m_waveform[3 % WAVEFORMS][0];
	uint16_t *wf4 = &m_waveform[4 % WAVEFORMS][0];
	uint16_t *wf5 = &m_waveform[5 % WAVEFORMS][0];
	uint16_t *wf6 = &m_waveform[6 % WAVEFORMS][0];
	uint16_t *wf7 = &m_waveform[7 % WAVEFORMS][0];
	uint16_t *wf8 = &m_waveform[8 % WAVEFORMS][0];
	uint16_t *wf9 = &m_waveform[9 % WAVEFORMS][0];
	uint16_t *wfa = &m_waveform[10 % WAVEFORMS][0];
	uint16_t *wfb = &m_waveform[11 % WAVEFORMS][0];
	uint16_t *wfc = &m_waveform[12 % WAVEFORMS][0];
	uint16_t *wfd = &m_waveform[13 % WAVEFORMS][0];
	uint16_t *wfe = &m_waveform[14 % WAVEFORMS][0];
	uint16_t *wff = &m_waveform[15 % WAVEFORMS][0];

	// create the waveforms
	for (uint32_t index = 0; index < WAVEFORM_LENGTH; index++)
		wf0[index] = abs_sin_attenuation(index) | (bitfield(index, 9) << 15);

	if (WAVEFORMS >= 4)
	{
		uint16_t zeroval = wf0[0];

		if (WAVEFORMS >= 16)
		{
			// we only have the diagrams to judge from, but suspecting waveform 1 (and
			// derived waveforms) are sin^2, based on OPX description of similar wave-
			// forms; since our sin table is logarithmic, this ends up just being
			// 2*existing value
			for (uint32_t index = 0; index < WAVEFORM_LENGTH; index++)
				wf8[index] = std::min<uint16_t>(2 * (wf0[index] & 0x7fff), zeroval) | (bitfield(index, 9) << 15);
		}

		for (uint32_t index = 0; index < WAVEFORM_LENGTH; index++)
		{
			wf1[index] = bitfield(index, 9) ? zeroval : wf0[index];
			wf2[index] = wf0[index] & 0x7fff;
			wf3[index] = bitfield(index, 8) ? zeroval : (wf0[index] & 0x7fff);
			if (WAVEFORMS >= 8)
			{
				wf4[index] = bitfield(index, 9) ? zeroval : wf0[index * 2];
				wf5[index] = bitfield(index, 9) ? zeroval : wf0[(index * 2) & 0x1ff];
				wf6[index] = bitfield(index, 9) << 15;
				wf7[index] = (bitfield(index, 9) ? (index ^ 0x13ff) : index) << 3;
			}
			if (WAVEFORMS >= 16)
			{
				wf9[index] = bitfield(index, 9) ? zeroval : wf8[index];
				wfa[index] = wf8[index] & 0x7fff;
				wfb[index] = bitfield(index, 8) ? zeroval : (wf8[index] & 0x7fff);
				wfc[index] = bitfield(index, 9) ? zeroval : wf8[index * 2];
				wfd[index] = bitfield(index, 9) ? zeroval : wf8[(index * 2) & 0x1ff];
				wfe[index] = bitfield(index, 9) << 15;
				wff[index] = (bitfield(index, 9) ? (index ^ 0x13ff) : index) << 3;
			}
		}
	}

	// create the LFO waveforms; AM in the low 8 bits, PM in the upper 8
	// waveforms are adjusted to match the pictures in the application manual
	for (uint32_t index = 0; index < LFO_WAVEFORM_LENGTH; index++)
	{
		// waveform 0 is a sawtooth
		uint8_t am = index ^ 0xff;
		int8_t pm = int8_t(index);
		m_lfo_waveform[0][index] = am | (pm << 8);

		// waveform 1 is a square wave
		am = bitfield(index, 7) ? 0 : 0xff;
		pm = int8_t(am ^ 0x80);
		m_lfo_waveform[1][index] = am | (pm << 8);

		// waveform 2 is a triangle wave
		am = bitfield(index, 7) ? (index << 1) : ((index ^ 0xff) << 1);
		pm = int8_t(bitfield(index, 6) ? am : ~am);
		m_lfo_waveform[2][index] = am | (pm << 8);

		// waveform 3 is noise; it is filled in dynamically
	}
}


//-------------------------------------------------
//  reset - reset to initial state
//-------------------------------------------------

void opnx_registers::reset()
{
	std::fill_n(&m_regdata[0], REGISTERS, 0);
	// enable output on both channels by default
	m_regdata[0xb4] = m_regdata[0xb5] = m_regdata[0xb6] = m_regdata[0xb7] = 0xc0;
	m_regdata[0x1b4] = m_regdata[0x1b5] = m_regdata[0x1b6] = m_regdata[0x1b7] = 0xc0;
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void opnx_registers::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_lfo_counter);
	state.save_restore(m_lfo_am);
	state.save_restore(m_regdata);
}


//-------------------------------------------------
//  operator_map - return an array of operator
//  indices for each channel; for OPN this is fixed
//-------------------------------------------------

void opnx_registers::operator_map(operator_mapping &dest) const
{
	// Note that the channel index order is 0,2,1,3, so we bitswap the index.
	//
	// This is because the order in the map is:
	//    carrier 1, carrier 2, modulator 1, modulator 2
	//
	// But when wiring up the connections, the more natural order is:
	//    carrier 1, modulator 1, carrier 2, modulator 2
	static const operator_mapping s_fixed_map =
	{ {
		operator_list(  0,  8,  4, 12 ),  // Channel 0 operators
		operator_list(  1,  9,  5, 13 ),  // Channel 1 operators
		operator_list(  2, 10,  6, 14 ),  // Channel 2 operators
		operator_list(  3, 11,  7, 15 ),  // Channel 3 operators
		operator_list( 16, 24, 20, 28 ),  // Channel 4 operators
		operator_list( 17, 25, 21, 29 ),  // Channel 5 operators
		operator_list( 18, 26, 22, 30 ),  // Channel 6 operators
		operator_list( 19, 27, 23, 31 ),  // Channel 7 operators
	} };
	dest = s_fixed_map;
}


//-------------------------------------------------
//  write - handle writes to the register array
//-------------------------------------------------

bool opnx_registers::write(uint16_t index, uint8_t data, uint32_t &channel, uint32_t &opmask)
{
	assert(index < REGISTERS);
	if (index >= REGISTERS) return false;

	// writes in the 0xa0-af/0x1a0-af region are handled as latched pairs
	// borrow unused registers 0xb8-bf/0x1b8-bf as temporary holding locations
	if ((index & 0xf0) == 0xa0)
	{
		if (is_native_mode())
		{
			if (((index & 0xf8) == 0xa8) && (bitfield(index, 0, 2) == 3))
				return false;
		}
		else
		{
			if ((bitfield(index, 0, 2) == 3) || (!is_ym2610b() && (bitfield(index, 0, 2) == 0)))
				return false;
		}

		uint32_t latchindex = 0xb8 | bitfield(index, 3);
		latchindex |= index & 0x100;

		// writes to the upper half just latch (only low 6 bits matter)
		if (bitfield(index, 2))
			m_regdata[latchindex] = data | 0x80;

		// writes to the lower half only commit if the latch is there
		else if (bitfield(m_regdata[latchindex], 7))
		{
			m_regdata[index] = data;
			m_regdata[index | 4] = m_regdata[latchindex] & 0x3f;
			m_regdata[latchindex] = 0;
		}
		return false;
	}
	else if ((index & 0xf8) == 0xb8)
	{
		// registers 0xb8-0xbf are used internally
		return false;
	}

	// everything else is normal
	m_regdata[index] = data;

	// handle writes to the key on index
	if (index == 0x28)
	{
		channel = bitfield(data, 0, 3);
		if (!is_native_mode() && (((channel & 3) == 3) || (!is_ym2610b() && ((channel & 3) == 0))))
			return false;

		opmask = bitfield(data, 4, 4);
		// according to the TX81Z manual, the sync option causes the LFOs
		// to reset at each note on
		if (opmask != 0)
		{
			for (int param = 0; param < 2; param++)
			{
				if (ch_pclfo_sync(channel, param))
					m_pclfo[channel].m_params[param].m_lfo_counter = 0;
			}
		}
		return true;
	}
	return false;
}


//-------------------------------------------------
//  clock_noise_and_lfo - clock the noise and LFO,
//  handling clock division, depth, and waveform
//  computations
//-------------------------------------------------

int32_t opnx_registers::clock_noise_and_lfo()
{
	for (uint32_t chan = 0; chan < CHANNELS; chan++)
	{
		pclfo_t &pclfo = m_pclfo[chan];
		for (int param = 0; param < 2; param++)
		{
			pclfo_t::pclfo_params_t &params = pclfo.m_params[param];
			// base noise frequency is measured at 2x 1/2 FM frequency; this
			// means each tick counts as two steps against the noise counter
			uint32_t freq = ch_pclfo_noise(chan, param) ^ 0xff;
			for (int rep = 0; rep < 2; rep++)
			{
				// evidence seems to suggest the LFSR is clocked continually and just
				// sampled at the noise frequency for output purposes; note that the
				// low 8 bits are the most recent 8 bits of history while bits 8-24
				// contain the 17 bit LFSR state
				params.m_noise_lfsr <<= 1;
				params.m_noise_lfsr |= bitfield(params.m_noise_lfsr, 17) ^ bitfield(params.m_noise_lfsr, 14) ^ 1;

				// compare against the frequency and latch when we exceed it
				if (params.m_noise_counter++ >= freq)
				{
					params.m_noise_counter = 0;
					params.m_noise_state = bitfield(params.m_noise_lfsr, 17);
				}
			}

			// treat the rate as a 4.4 floating-point step value with implied
			// leading 1; this matches exactly the frequencies in the application
			// manual, though it might not be implemented exactly this way on chip
			uint32_t rate = ch_pclfo_rate(chan, param);
			if (rate != 0)
			{
				params.m_lfo_counter += (0x10 | bitfield(rate, 0, 4)) << bitfield(rate, 4, 4);
			}
			uint32_t lfo = bitfield(params.m_lfo_counter, 22, 8);

			// fill in the noise entry 1 ahead of our current position; this
			// ensures the current value remains stable for a full LFO clock
			// and effectively latches the running value when the LFO advances
			uint32_t lfo_noise = bitfield(params.m_noise_lfsr, 17, 8);
			params.m_noise_waveform[(lfo + 1) & 0xff] = lfo_noise | (lfo_noise << 8);

			// fetch the AM/PM values based on the waveform; AM is unsigned and
			// encoded in the low 8 bits, while PM signed and encoded in the upper
			// 8 bits
			int32_t ampm = ch_pclfo_waveform(chan, param) == 3 ? params.m_noise_waveform[lfo] : m_lfo_waveform[ch_pclfo_waveform(chan, param)][lfo];

			// apply depth to the AM values and store for later
			params.m_lfo_am = ((ampm & 0xff) * ch_pclfo_am_depth(chan, param)) >> 8;

			// apply depth to the PM values and return them combined into two
			params.m_lfo_pm = ((ampm >> 8) * int32_t(ch_pclfo_pm_depth(chan, param))) >> 8;
		}
	}

	// if LFO not enabled (not present on OPN), quick exit with 0s
	if (!lfo_enable())
	{
		m_lfo_counter = 0;

		// special case: if LFO is disabled on OPNA, it basically just keeps the counter
		// at 0; since position 0 gives an AM value of 0x3f, it is important to reflect
		// that here; for example, MegaDrive Venom plays some notes with LFO globally
		// disabled but enabling LFO on the operators, and it expects this added attenutation
		m_lfo_am = 0x3f;
		return 0;
	}

	// this table is based on converting the frequencies in the applications
	// manual to clock dividers, based on the assumption of a 7-bit LFO value
	static uint8_t const lfo_max_count[8] = { 109, 78, 72, 68, 63, 45, 9, 6 };
	uint32_t subcount = uint8_t(m_lfo_counter++);

	// when we cross the divider count, add enough to zero it and cause an
	// increment at bit 8; the 7-bit value lives from bits 8-14
	if (subcount >= lfo_max_count[lfo_rate()])
	{
		// note: to match the published values this should be 0x100 - subcount;
		// however, tests on the hardware and nuked bear out an off-by-one
		// error exists that causes the max LFO rate to be faster than published
		m_lfo_counter += 0x101 - subcount;
	}

	// AM value is 7 bits, staring at bit 8; grab the low 6 directly
	m_lfo_am = bitfield(m_lfo_counter, 8, 6);

	// first half of the AM period (bit 6 == 0) is inverted
	if (bitfield(m_lfo_counter, 8+6) == 0)
		m_lfo_am ^= 0x3f;

	// PM value is 5 bits, starting at bit 10; grab the low 3 directly
	int32_t pm = bitfield(m_lfo_counter, 10, 3);

	// PM is reflected based on bit 3
	if (bitfield(m_lfo_counter, 10+3))
		pm ^= 7;

	// PM is negated based on bit 4
	return bitfield(m_lfo_counter, 10+4) ? -pm : pm;
}


//-------------------------------------------------
//  lfo_am_offset - return the AM offset from LFO
//  for the given channel
//-------------------------------------------------

uint32_t opnx_registers::lfo_am_offset(uint32_t choffs) const
{
	uint32_t result = 0;
	for (int param = 0; param < 2; param++)
	{
		// shift value for AM sensitivity is [*, 0, 1, 2],
		// mapping to values of [0, 23.9, 47.8, and 95.6dB]
		uint32_t am_sensitivity = ch_pclfo_am_sens(choffs, param);
		if (am_sensitivity != 0)
			result += m_pclfo[choffs].m_params[param].m_lfo_am << (am_sensitivity - 1);
	}

	// shift value for AM sensitivity is [7, 3, 1, 0],
	// mapping to values of [0, 1.4, 5.9, and 11.8dB]
	uint32_t am_shift = (1 << (ch_lfo_am_sens(choffs) ^ 3)) - 1;

	// QUESTION: max sensitivity should give 11.8dB range, but this value
	// is directly added to an x.8 attenuation value, which will only give
	// 126/256 or ~4.9dB range -- what am I missing? The calculation below
	// matches several other emulators, including the Nuked implemenation.

	// raw LFO AM value on OPN is 0-3F, scale that up by a factor of 2
	// (giving 7 bits) before applying the final shift
	return result + ((m_lfo_am << 1) >> am_shift);
}


//-------------------------------------------------
//  cache_operator_data - fill the operator cache
//  with prefetched data
//-------------------------------------------------

void opnx_registers::cache_operator_data(uint32_t choffs, uint32_t opoffs, opdata_cache &cache)
{
	// set up the easy stuff
	cache.waveform = &m_waveform[op_waveform(opoffs)][0];

	// get frequency from the channel
	uint32_t block_freq = cache.block_freq = ch_block_freq(choffs);

	// if multi-frequency mode is enabled and this is channel 2,
	// fetch one of the special frequencies
	if (multi_freq() && choffs == 2)
	{
		if (opoffs == 2)
			block_freq = cache.block_freq = multi_block_freq(1);
		else if (opoffs == 10)
			block_freq = cache.block_freq = multi_block_freq(2);
		else if (opoffs == 6)
			block_freq = cache.block_freq = multi_block_freq(0);
	}

	// compute the keycode: block_freq is:
	//
	//     BBBFFFFFFFFFFF
	//     ^^^^???
	//
	// the 5-bit keycode uses the top 4 bits plus a magic formula
	// for the final bit
	uint32_t keycode = bitfield(block_freq, 10, 4) << 1;

	// lowest bit is determined by a mix of next lower FNUM bits
	// according to this equation from the YM2608 manual:
	//
	//   (F11 & (F10 | F9 | F8)) | (!F11 & F10 & F9 & F8)
	//
	// for speed, we just look it up in a 16-bit constant
	keycode |= bitfield(0xfe80, bitfield(block_freq, 7, 4));

	// detune adjustment
	cache.detune = detune_adjustment(op_detune(opoffs), keycode);

	// multiple value, as an x.1 value (0 means 0.5)
	cache.multiple = op_multiple(opoffs) * 2;
	if (cache.multiple == 0)
		cache.multiple = 1;

	// phase step, or PHASE_STEP_DYNAMIC if PM is active; this depends on
	// block_freq, detune, and multiple, so compute it after we've done those
	if ((lfo_enable() == 0 || ch_lfo_pm_sens(choffs) == 0))
		cache.phase_step = compute_phase_step(choffs, opoffs, cache, 0);
	else
		cache.phase_step = opdata_cache::PHASE_STEP_DYNAMIC;

	// total level, scaled by 8
	cache.total_level = op_total_level(opoffs) << 3;

	// 4-bit sustain level, but 15 means 31 so effectively 5 bits
	cache.eg_sustain = op_sustain_level(opoffs);
	cache.eg_sustain |= (cache.eg_sustain + 1) & 0x10;
	cache.eg_sustain <<= 5;

	// determine KSR adjustment for enevlope rates
	uint32_t ksrval = keycode >> (op_ksr(opoffs) ^ 3);
	cache.eg_rate[EG_ATTACK] = effective_rate(op_attack_rate(opoffs) * 2, ksrval);
	cache.eg_rate[EG_DECAY] = effective_rate(op_decay_rate(opoffs) * 2, ksrval);
	cache.eg_rate[EG_SUSTAIN] = effective_rate(op_sustain_rate(opoffs) * 2, ksrval);
	cache.eg_rate[EG_RELEASE] = effective_rate(op_release_rate(opoffs) * 4 + 2, ksrval);
}


//-------------------------------------------------
//  compute_phase_step - compute the phase step
//-------------------------------------------------

uint32_t opnx_registers::compute_phase_step(uint32_t choffs, uint32_t opoffs, opdata_cache const &cache, int32_t lfo_raw_pm)
{
	// OPN phase calculation has only a single detune parameter
	// and uses FNUMs instead of keycodes

	// extract frequency number (low 11 bits of block_freq)
	uint32_t fnum = bitfield(cache.block_freq, 0, 11) << 1;

	// if there's a non-zero PM sensitivity, compute the adjustment
	uint32_t pm_sensitivity = ch_lfo_pm_sens(choffs);
	if (pm_sensitivity != 0)
	{
		// apply the phase adjustment based on the upper 7 bits
		// of FNUM and the PM depth parameters
		fnum += opn_lfo_pm_phase_adjustment(bitfield(cache.block_freq, 4, 7), pm_sensitivity, lfo_raw_pm);

		// keep fnum to 12 bits
		fnum &= 0xfff;
	}

	for (int param = 0; param < 2; param++)
	{
		pclfo_t::pclfo_params_t &params = m_pclfo[choffs].m_params[param];

		uint32_t pclfo_pm_sensitivity = ch_pclfo_pm_sens(choffs, param);
		if (pclfo_pm_sensitivity != 0)
		{
			// raw PM value is -127..128 which is +/- 200 cents
			// manual gives these magnitudes in cents:
			//    0, +/-5, +/-10, +/-20, +/-50, +/-100, +/-400, +/-700
			// this roughly corresponds to shifting the 200-cent value:
			//    0  >> 5,  >> 4,  >> 3,  >> 2,  >> 1,   << 1,   << 2
			if (pm_sensitivity < 6)
				fnum += int8_t(params.m_lfo_pm) >> (6 - pclfo_pm_sensitivity);
			else
				fnum += int8_t(params.m_lfo_pm) << (pclfo_pm_sensitivity - 5);

			// keep fnum to 12 bits
			fnum &= 0xfff;
		}
	}

	// apply block shift to compute phase step
	uint32_t block = bitfield(cache.block_freq, 11, 3);
	uint32_t phase_step = (fnum << block) >> 2;

	// apply detune based on the keycode
	phase_step += cache.detune;

	// clamp to 17 bits in case detune overflows
	// QUESTION: is this specific to the YM2612/3438?
	phase_step &= 0x1ffff;

	// apply frequency multiplier (which is cached as an x.1 value)
	return (phase_step * cache.multiple) >> 1;
}


//-------------------------------------------------
//  log_keyon - log a key-on event
//-------------------------------------------------

std::string opnx_registers::log_keyon(uint32_t choffs, uint32_t opoffs)
{
	uint32_t chnum = (choffs & 3) + 4 * bitfield(choffs, 8);
	uint32_t opnum = (opoffs & 15) + 16 * bitfield(opoffs, 8);

	uint32_t block_freq = ch_block_freq(choffs);
	if (multi_freq() && choffs == 2)
	{
		if (opoffs == 2)
			block_freq = multi_block_freq(1);
		else if (opoffs == 10)
			block_freq = multi_block_freq(2);
		else if (opoffs == 6)
			block_freq = multi_block_freq(0);
	}

	char buffer[256];
	char *end = &buffer[0];

	end += snprintf(end, 256-(end-buffer), "%u.%02u freq=%04X dt=%u fb=%u alg=%X mul=%X tl=%02X ksr=%u adsr=%02X/%02X/%02X/%X sl=%X",
		chnum, opnum,
		block_freq,
		op_detune(opoffs),
		ch_feedback(choffs),
		ch_algorithm(choffs),
		op_multiple(opoffs),
		op_total_level(opoffs),
		op_ksr(opoffs),
		op_attack_rate(opoffs),
		op_decay_rate(opoffs),
		op_sustain_rate(opoffs),
		op_release_rate(opoffs),
		op_sustain_level(opoffs));

	if (OUTPUTS > 1)
		end += snprintf(end, 256-(end-buffer), " out=%c%c",
			ch_output_0(choffs) ? 'L' : '-',
			ch_output_1(choffs) ? 'R' : '-');
	if (op_ssg_eg_enable(opoffs))
		end += snprintf(end, 256-(end-buffer), " ssg=%X", op_ssg_eg_mode(opoffs));
	bool am = (op_lfo_am_enable(opoffs) && ch_lfo_am_sens(choffs) != 0);
	if (am)
		end += snprintf(end, 256-(end-buffer), " am=%u", ch_lfo_am_sens(choffs));
	bool pm = (ch_lfo_pm_sens(choffs) != 0);
	if (pm)
		end += snprintf(end, 256-(end-buffer), " pm=%u", ch_lfo_pm_sens(choffs));
	if (am || pm)
		end += snprintf(end, 256-(end-buffer), " lfo=%02X", lfo_rate());
	if (multi_freq() && choffs == 2)
		end += snprintf(end, 256-(end-buffer), " multi=1");

	return buffer;
}



//*********************************************************
//  SSG RESAMPLER
//*********************************************************

//-------------------------------------------------
//  add_last - helper to add the last computed
//  value to the sums, applying the given scale
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
void ssgx_resampler<OutputType, FirstOutput, MixTo1>::add_last(int32_t &sum0, int32_t &sum1, int32_t &sum2, int32_t &sum3, int32_t &sum4, int32_t &sum5, int32_t scale)
{
	sum0 += m_last.data[0] * scale;
	sum1 += m_last.data[1] * scale;
	sum2 += m_last.data[2] * scale;
	sum3 += m_last.data[3] * scale;
	sum4 += m_last.data[4] * scale;
	sum5 += m_last.data[5] * scale;
}


//-------------------------------------------------
//  clock_and_add - helper to clock a new value
//  and then add it to the sums, applying the
//  given scale
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
void ssgx_resampler<OutputType, FirstOutput, MixTo1>::clock_and_add(int32_t &sum0, int32_t &sum1, int32_t &sum2, int32_t &sum3, int32_t &sum4, int32_t &sum5, int32_t scale, uint32_t tick)
{
	m_ssgx.clock(tick);
	m_ssgx.output(m_last);
	add_last(sum0, sum1, sum2, sum3, sum4, sum5, scale);
}


//-------------------------------------------------
//  write_to_output - helper to write the sums to
//  the appropriate outputs, applying the given
//  divisor to the final result
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
void ssgx_resampler<OutputType, FirstOutput, MixTo1>::write_to_output(OutputType *output, int32_t sum0, int32_t sum1, int32_t sum2, int32_t sum3, int32_t sum4, int32_t sum5, int32_t divisor)
{
	if (MixTo1)
	{
		// mixing to one, apply a 2/3 factor to prevent overflow
		output->data[FirstOutput + 0] = (sum0 + sum1 + sum2) * 2 / (3 * divisor);
		output->data[FirstOutput + 1] = (sum3 + sum4 + sum5) * 2 / (3 * divisor);
	}
	else
	{
		// write three outputs in a row
		output->data[FirstOutput + 0] = sum0 / divisor;
		output->data[FirstOutput + 1] = sum1 / divisor;
		output->data[FirstOutput + 2] = sum2 / divisor;
		output->data[FirstOutput + 3] = sum3 / divisor;
		output->data[FirstOutput + 4] = sum4 / divisor;
		output->data[FirstOutput + 5] = sum5 / divisor;
	}

	// track the sample index here
	m_sampindex++;
}


//-------------------------------------------------
//  ssgx_resampler - constructor
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
ssgx_resampler<OutputType, FirstOutput, MixTo1>::ssgx_resampler(ssgx_engine &ssg) :
	m_ssgx(ssg),
	m_sampindex(0),
	m_resampler(&ssgx_resampler::resample_nop)
{
	m_last.clear();
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
void ssgx_resampler<OutputType, FirstOutput, MixTo1>::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_sampindex);
	state.save_restore(m_last.data);
}


//-------------------------------------------------
//  configure - configure a new ratio
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
void ssgx_resampler<OutputType, FirstOutput, MixTo1>::configure(uint32_t outsamples, uint32_t srcsamples)
{
	switch (outsamples * 1000 + srcsamples)
	{
		case 144*1000 + 1:  /* 288:1 */	m_resampler = &ssgx_resampler::resample_n_1<144>; break;
		case 288*1000 + 1:  /* 288:1 */	m_resampler = &ssgx_resampler::resample_n_1<288>; break;
		case 576*1000 + 1:  /* 576:1 */	m_resampler = &ssgx_resampler::resample_n_1<576>; break;
		case 1*1000 + 1:	/* 1:1 */	m_resampler = &ssgx_resampler::resample_n_1<1>; break;
		case 1*1000 + 144:	/* 1:144 */	m_resampler = &ssgx_resampler::resample_1_n<144>; break;
		case 1*1000 + 288:	/* 1:288 */	m_resampler = &ssgx_resampler::resample_1_n<288>; break;
		case 1*1000 + 576:	/* 1:576 */	m_resampler = &ssgx_resampler::resample_1_n<576>; break;
		case 0*1000 + 0:	/* 0:0 */	m_resampler = &ssgx_resampler::resample_nop;    break;
		default: assert(false); break;
	}
}


//-------------------------------------------------
//  resample_n_1 - resample SSG output to the
//  target at a rate of 1 SSG sample to every
//  n output sample
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
template<int Multiplier>
void ssgx_resampler<OutputType, FirstOutput, MixTo1>::resample_n_1(OutputType *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		if (m_sampindex % Multiplier == 0)
		{
			m_ssgx.clock();
			m_ssgx.output(m_last);
		}
		write_to_output(output, m_last.data[0], m_last.data[1], m_last.data[2], m_last.data[3], m_last.data[4], m_last.data[5]);
	}
}


//-------------------------------------------------
//  resample_1_n - resample SSG output to the
//  target at a rate of n SSG samples to every
//  1 output sample
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
template<int Divisor>
void ssgx_resampler<OutputType, FirstOutput, MixTo1>::resample_1_n(OutputType *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		int32_t sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0, sum5 = 0;;
		clock_and_add(sum0, sum1, sum2, sum3, sum4, sum5, 1, Divisor);
		write_to_output(output, sum0, sum1, sum2, sum3, sum4, sum5, 1);
	}
}


//-------------------------------------------------
//  resample_2_9 - resample SSG output to the
//  target at a rate of 9 SSG samples to every
//  2 output samples
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
void ssgx_resampler<OutputType, FirstOutput, MixTo1>::resample_2_9(OutputType *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		int32_t sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0, sum5 = 0;;
		if (bitfield(m_sampindex, 0) != 0)
			add_last(sum0, sum1, sum2, sum3, sum4, sum5, 1);
		clock_and_add(sum0, sum1, sum2, sum3, sum4, sum5, 2);
		clock_and_add(sum0, sum1, sum2, sum3, sum4, sum5, 2);
		clock_and_add(sum0, sum1, sum2, sum3, sum4, sum5, 2);
		clock_and_add(sum0, sum1, sum2, sum3, sum4, sum5, 2);
		if (bitfield(m_sampindex, 0) == 0)
			clock_and_add(sum0, sum1, sum2, sum3, sum4, sum5, 1);
		write_to_output(output, sum0, sum1, sum2, sum3, sum4, sum5, 9);
	}
}


//-------------------------------------------------
//  resample_2_3 - resample SSG output to the
//  target at a rate of 3 SSG samples to every
//  2 output samples
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
void ssgx_resampler<OutputType, FirstOutput, MixTo1>::resample_2_3(OutputType *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		int32_t sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0, sum5 = 0;;
		if (bitfield(m_sampindex, 0) == 0)
		{
			clock_and_add(sum0, sum1, sum2, sum3, sum4, sum5, 2);
			clock_and_add(sum0, sum1, sum2, sum3, sum4, sum5, 1);
		}
		else
		{
			add_last(sum0, sum1, sum2, sum3, sum4, sum5, 1);
			clock_and_add(sum0, sum1, sum2, sum3, sum4, sum5, 2);
		}
		write_to_output(output, sum0, sum1, sum2, sum3, sum4, sum5, 3);
	}
}


//-------------------------------------------------
//  resample_4_3 - resample SSG output to the
//  target at a rate of 3 SSG samples to every
//  4 output samples
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
void ssgx_resampler<OutputType, FirstOutput, MixTo1>::resample_4_3(OutputType *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		int32_t sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0, sum5 = 0;;
		int32_t step = bitfield(m_sampindex, 0, 2);
		add_last(sum0, sum1, sum2, sum3, sum4, sum5, step);
		if (step != 3)
			clock_and_add(sum0, sum1, sum2, sum3, sum4, sum5, 3 - step);
		write_to_output(output, sum0, sum1, sum2, sum3, sum4, sum5, 3);
	}
}


//-------------------------------------------------
//  resample_nop - no-op resampler
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
void ssgx_resampler<OutputType, FirstOutput, MixTo1>::resample_nop(OutputType *output, uint32_t numsamples)
{
	// nothing to do except increment the sample index
	m_sampindex += numsamples;
}



//*********************************************************
//  YM2610X
//*********************************************************

//-------------------------------------------------
//  ym2610x - constructor
//-------------------------------------------------

ym2610x::ym2610x(ymfm_interface &intf) :
	m_fidelity(OPNX_FIDELITY_MAX),
	m_address(0),
	m_eos_status(0x00),
	m_flag_mask(EOS_FLAGS_MASK),
	m_fm(intf),
	m_ssgx(intf),
	m_ssgx_resampler(m_ssgx),
	m_adpcmx_a(intf),
	m_adpcmx_b(intf)
{
	update_prescale();
}


//-------------------------------------------------
//  reset - reset the system
//-------------------------------------------------

void ym2610x::reset()
{
	// reset the engines
	m_fm.reset();
	m_ssgx.reset();
	m_adpcmx_a.reset();
	m_adpcmx_b.reset();

	// initialize our special interrupt states
	m_eos_status = 0x00;
	m_flag_mask = EOS_FLAGS_MASK;
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void ym2610x::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_address);
	state.save_restore(m_eos_status);
	state.save_restore(m_flag_mask);

	m_fm.save_restore(state);
	m_ssgx.save_restore(state);
	m_ssgx_resampler.save_restore(state);
	m_adpcmx_a.save_restore(state);
	m_adpcmx_b.save_restore(state);
}


//-------------------------------------------------
//  read_status - read the status register
//-------------------------------------------------

uint8_t ym2610x::read_status()
{
	uint8_t result = m_fm.status() & (fm_engine::STATUS_TIMERA | fm_engine::STATUS_TIMERB);
	if (m_fm.intf().ymfm_is_busy())
		result |= fm_engine::STATUS_BUSY;
	return result;
}


//-------------------------------------------------
//  read_data - read the data register
//-------------------------------------------------

uint8_t ym2610x::read_data()
{
	uint8_t result = 0;
	if (m_address < 0x10)
	{
		// 00-0D: Read from SSG
		result = m_ssgx.read(m_address & 0x0f);
	}
	else if (m_address == 0xff)
	{
		// FF: ID code
		result = 1;
	}
	return result;
}


//-------------------------------------------------
//  read_status_hi - read the extended status
//  register
//-------------------------------------------------

uint8_t ym2610x::read_status_hi()
{
	return m_eos_status & m_flag_mask;
}


//-------------------------------------------------
//  read_data_hi - read the upper data register
//-------------------------------------------------

uint8_t ym2610x::read_data_hi()
{
	uint8_t result = 0;
	return result;
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

uint8_t ym2610x::read(uint32_t offset)
{
	uint8_t result = 0;
	switch (offset & 3)
	{
		case 0: // status port, YM2203 compatible
			result = read_status();
			break;

		case 1: // data port (only SSG)
			result = read_data();
			break;

		case 2: // status port, extended
			result = read_status_hi();
			break;

		case 3: // ADPCM-B data
			result = read_data_hi();
			break;
	}
	return result;
}


//-------------------------------------------------
//  write_address - handle a write to the address
//  register
//-------------------------------------------------

void ym2610x::write_address(uint8_t data)
{
	// just set the address
	m_address = data;
}


//-------------------------------------------------
//  write - handle a write to the data register
//-------------------------------------------------

void ym2610x::write_data(uint8_t data)
{
	// ignore if paired with upper address
	if (bitfield(m_address, 8))
		return;

	if (m_address < 0x10)
	{
		// 00-0D: write to SSG
		m_ssgx.write(m_address & 0x0f, data);
	}
	else if (m_address < 0x20)
	{	
		if (m_address == 0x1c)
		{
			// 1C: EOS flag reset
			m_flag_mask = ~data & EOS_FLAGS_MASK;
			m_eos_status &= ~(data & EOS_FLAGS_MASK);
		}
		else
		{
			// 10-1B: write to ADPCM-B
			// YM2610 effectively forces external mode on, and disables recording
			if (m_address == 0x10)
				data = (data | 0x20) & ~0x40;
			m_adpcmx_b.write(m_address & 0x0f, data);
		}
	}
	else
	{
		// 20-FF: write to FM
		m_fm.write(m_address, data);
	}

	// mark busy for a bit
	m_fm.intf().ymfm_set_busy_end(32 * m_fm.clock_prescale());
}


//-------------------------------------------------
//  write_address_hi - handle a write to the upper
//  address register
//-------------------------------------------------

void ym2610x::write_address_hi(uint8_t data)
{
	// just set the address
	m_address = 0x100 | data;
}


//-------------------------------------------------
//  write_data_hi - handle a write to the upper
//  data register
//-------------------------------------------------

void ym2610x::write_data_hi(uint8_t data)
{
	// ignore if paired with upper address
	if (!bitfield(m_address, 8))
		return;

	if (m_address < 0x130)
	{
		// 100-12F: write to ADPCM-A
		m_adpcmx_a.write(m_address & 0x3f, data);
	}
	else
	{
		// 130-1FF: write to FM
		m_fm.write(m_address, data);
	}

	// mark busy for a bit
	m_fm.intf().ymfm_set_busy_end(32 * m_fm.clock_prescale());
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2610x::write(uint32_t offset, uint8_t data)
{
	switch (offset & 3)
	{
		case 0: // address port
			write_address(data);
			break;

		case 1: // data port
			write_data(data);
			break;

		case 2: // upper address port
			write_address_hi(data);
			break;

		case 3: // upper data port
			write_data_hi(data);
			break;
	}
}


//-------------------------------------------------
//  generate - generate one sample of sound
//-------------------------------------------------

void ym2610x::generate(output_data *output, uint32_t numsamples)
{
	// FM output is just repeated the prescale number of times
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		if ((m_ssgx_resampler.sampindex() + samp) % m_fm_samples_per_output == 0)
			clock_fm_and_adpcm();
		output->data[0] = m_last_fm.data[0];
		output->data[1] = m_last_fm.data[1];
	}

	// resample the SSG as configured
	m_ssgx_resampler.resample(output - numsamples, numsamples);
}


//-------------------------------------------------
//  update_prescale - update the prescale value,
//  recomputing derived values
//-------------------------------------------------

void ym2610x::update_prescale()
{
	// Fidelity:   ---- minimum ----    ---- medium -----    ---- maximum-----
	//              rate = clock/576     rate = clock/576     rate = clock/2
	// Prescale    FM rate  SSG rate    FM rate  SSG rate    FM rate  SSG rate
	//    18          1:1     1:576        1:1     1:576      576:1    1:1

	// compute the number of FM samples per output sample, and select the
	// resampler function
	if (m_fidelity == OPNX_FIDELITY_MIN || m_fidelity == OPNX_FIDELITY_MED)
	{
		m_fm_samples_per_output = 1;
		m_ssgx_resampler.configure(1, 144);
	}
	else
	{
		m_fm_samples_per_output = 144;
		m_ssgx_resampler.configure(1, 1);
	}

	// if overriding the SSG, override the configuration with the nop
	// resampler to at least keep the sample index moving forward
	if (m_ssgx.overridden())
		m_ssgx_resampler.configure(0, 0);
}


//-------------------------------------------------
//  clock_fm_and_adpcm - clock FM and ADPCM state
//-------------------------------------------------

void ym2610x::clock_fm_and_adpcm()
{
	uint32_t fm_mask = m_fm.regs().is_native_mode() ? opnx_registers::ALL_CHANNELS : (m_fm.regs().is_ym2610b() ? 0x77 : 0x66);
	// clock the system
	uint32_t env_counter = m_fm.clock(fm_mask);

	// clock the ADPCM-A engine on every envelope cycle
	if (bitfield(env_counter, 0, 2) == 0)
		m_eos_status |= m_adpcmx_a.clock(0x3f);

	// clock the ADPCM-B engine every cycle
	m_adpcmx_b.clock();

	// we track the last ADPCM-B EOS value in bit 6 (which is hidden from callers);
	// if it changed since the last sample, update the visible EOS state in bit 7
	uint8_t live_eos = ((m_adpcmx_b.status() & adpcmx_b_channel::STATUS_EOS) != 0) ? 0x40 : 0x00;
	if (((live_eos ^ m_eos_status) & 0x40) != 0)
		m_eos_status = (m_eos_status & ~0xc0) | live_eos | (live_eos << 1);

	// update the FM content; OPNB is 13-bit with no intermediate clipping
	m_fm.output(m_last_fm.clear(), 1, 32767, fm_mask);

	// mix in the ADPCM and clamp
	m_adpcmx_a.output(m_last_fm, 0x3f);
	m_adpcmx_b.output(m_last_fm, 1);
	m_last_fm.clamp16();
}

}
