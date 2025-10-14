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

#include "ymfm_ssgx.h"

namespace ymfm
{

//*********************************************************
// SSG REGISTERS
//*********************************************************

//-------------------------------------------------
//  reset - reset the register state
//-------------------------------------------------

void ssgx_registers::reset()
{
	std::fill_n(&m_regdata[0], REGISTERS, 0);
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void ssgx_registers::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_regdata);
}



//*********************************************************
// SSG ENGINE
//*********************************************************

//-------------------------------------------------
//  ssgx_engine - constructor
//-------------------------------------------------

ssgx_engine::ssgx_engine(ymfm_interface &intf) :
	m_intf(intf),
	m_tone{tone_t(), tone_t(), tone_t()},
	m_envelope{envelope_t(), envelope_t(), envelope_t()},
	m_noise{noise_t(), noise_t(), noise_t()},
	m_override(nullptr)
{
}


//-------------------------------------------------
//  reset - reset the engine state
//-------------------------------------------------

void ssgx_engine::reset()
{
	// defer to the override if present
	if (m_override != nullptr)
		return m_override->ssgx_reset();

	// reset register state
	m_regs.reset();

	// reset engine state
	for (int chan = 0; chan < 3; chan++)
	{
		m_tone[chan].reset();
		m_envelope[chan].reset();
		m_noise[chan].reset();
	}
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void ssgx_engine::save_restore(ymfm_saved_state &state)
{
	// save register state
	m_regs.save_restore(state);

	// save engine state
}


//-------------------------------------------------
//  clock - master clocking function
//-------------------------------------------------

void ssgx_engine::clock(uint32_t tick)
{
	// clock tones; tone period units are clock/16 but since we run at clock/8
	// that works out for us to toggle the state (50% duty cycle) at twice the
	// programmed period
	for (int chan = 0; chan < 3; chan++)
	{
		tone_t &tone = m_tone[chan];
		uint32_t tone_period = std::max<uint32_t>(1, m_regs.ch_tone_period(chan));
		tone.m_tone_count += tick;
		while (tone.m_tone_count >= tone_period)
		{
			tone.m_tone_duty = (tone.m_tone_duty + 1) & 0x1f;
			tone.m_tone_state = m_regs.is_native_mode() ? ((tone.m_tone_duty >= m_regs.ch_pulse_duty(chan)) ? 0 : 1) : bitfield(tone.m_tone_duty, 4);
			tone.m_tone_count -= tone_period;
		}

		// clock noise; noise period units are clock/16 but since we run at clock/8,
		// our counter needs a right shift prior to compare; note that a period of 0
		// should produce an indentical result to a period of 1, so add a special
		// check against that case
		noise_t &noise = m_noise[chan];
		uint32_t noise_period = std::max<uint32_t>(1, m_regs.ch_noise_period(chan)) << (m_regs.is_native_mode() ? 4 : 6);
		noise.m_noise_count += tick;
		while (noise.m_noise_count >= noise_period)
		{
			if (m_regs.is_native_mode())
			{
				noise.m_noise_compare++;
				if (noise.m_noise_compare >= (((noise.m_noise_state & 0xff) & m_regs.noise_and_mask()) | m_regs.noise_or_mask()))
				{
					noise.m_noise_output ^= 1;
					noise.m_noise_compare = 0;
				}
			}
			noise.m_noise_state ^= (bitfield(noise.m_noise_state, 0) ^ bitfield(noise.m_noise_state, 2)) << 17;
			noise.m_noise_state >>= 1;
			noise.m_noise_count -= noise_period;
			if (!m_regs.is_native_mode())
			{
				noise.m_noise_output = noise.m_noise_state & 1;
			}
		}

		// clock envelope; envelope period units are clock/8 (manual says clock/256
		// but that's for all 32 steps)
		envelope_t &envelope = m_envelope[chan];
		uint32_t envelope_period = std::max<uint32_t>(1, m_regs.ch_envelope_period(chan)) << 5;
		envelope.m_envelope_count += tick;
		while (envelope.m_envelope_count >= envelope_period)
		{
			envelope.m_envelope_state++;
			envelope.m_envelope_count -= envelope_period;
		}
	}
}


//-------------------------------------------------
//  output - output the current state
//-------------------------------------------------

void ssgx_engine::output(output_data &output)
{
	// volume to amplitude table, taken from MAME's implementation but biased
	// so that 0 == 0
	static int16_t const s_amplitudes[32] =
	{
		     0,   32,   78,  141,  178,  222,  262,  306,
		   369,  441,  509,  585,  701,  836,  965, 1112,
		  1334, 1595, 1853, 2146, 2576, 3081, 3576, 4135,
		  5000, 6006, 7023, 8155, 9963,11976,14132,16382
	};

	// iterate over channels
	for (int chan = 0; chan < 3; chan++)
	{
		envelope_t &envelope = m_envelope[chan];
		// compute the envelope volume
		uint32_t envelope_volume;
		if ((m_regs.ch_envelope_hold(chan) | (m_regs.ch_envelope_continue(chan) ^ 1)) && envelope.m_envelope_state >= 32)
		{
			envelope.m_envelope_state = 32;
			envelope_volume = ((m_regs.ch_envelope_attack(chan) ^ m_regs.ch_envelope_alternate(chan)) & m_regs.ch_envelope_continue(chan)) ? 31 : 0;
		}
		else
		{
			uint32_t attack = m_regs.ch_envelope_attack(chan);
			if (m_regs.ch_envelope_alternate(chan))
				attack ^= bitfield(envelope.m_envelope_state, 5);
			envelope_volume = (envelope.m_envelope_state & 31) ^ (attack ? 0 : 31);
		}

		// noise depends on the noise state, which is the LSB of m_noise_state
		uint32_t noise_on = m_regs.ch_noise_enable_n(chan) | m_noise[chan].m_noise_output;

		// tone depends on the current tone state
		uint32_t tone_on = m_regs.ch_tone_enable_n(chan) | m_tone[chan].m_tone_state;

		// if neither tone nor noise enabled, return 0
		uint32_t volume;
		if ((noise_on & tone_on) == 0)
			volume = 0;

		// if the envelope is enabled, use its amplitude
		else if (m_regs.ch_envelope_enable(chan))
			volume = envelope_volume >> m_regs.ch_envelope_volume(chan);

		// otherwise, scale the tone amplitude up to match envelope values
		// according to the datasheet, amplitude 15 maps to envelope 31
		else if (m_regs.is_native_mode())
		{
			volume = m_regs.ch_amplitude(chan);
		}
		else
		{
			volume = m_regs.ch_amplitude(chan) * 2;
			if (volume != 0)
				volume |= 1;
		}

		// convert to amplitude
		if ((!m_regs.is_native_mode()) || (!m_regs.ch_left_disable(chan)))
			output.data[chan] = s_amplitudes[volume];
		if ((!m_regs.is_native_mode()) || (!m_regs.ch_right_disable(chan)))
			output.data[chan + 3] = s_amplitudes[volume];
	}

	m_last_out = output;
}


//-------------------------------------------------
//  read - handle reads from the SSG registers
//-------------------------------------------------

uint8_t ssgx_engine::read(uint32_t regnum)
{
	// defer to the override if present
	if (m_override != nullptr)
		return m_override->ssgx_read(regnum);

	if (regnum != 0x0f)
		regnum += m_regs.register_bank() * 0x10;

	// otherwise just return the register value
	return m_regs.read(regnum);
}


//-------------------------------------------------
//  write - handle writes to the SSG registers
//-------------------------------------------------

void ssgx_engine::write(uint32_t regnum, uint8_t data)
{
	// defer to the override if present
	if (m_override != nullptr)
		return m_override->ssgx_write(regnum, data);

	if (regnum != 0x0f)
		regnum += m_regs.register_bank() * 0x10;

	// store the raw value to the register array;
	// most writes are passive, consumed only when needed
	m_regs.write(regnum, data);

	// writes to the envelope shape register reset the state
	if (regnum == 0x0d)
	{
		m_envelope[0].m_envelope_state = 0;
		if (!m_regs.is_native_mode())
		{
			m_envelope[1].m_envelope_state = 0;
			m_envelope[2].m_envelope_state = 0;
		}
	}
	if ((m_regs.is_native_mode()) && ((regnum >= 0x14) && (regnum <= 0x15)))
	{
		m_envelope[1 + (regnum - 0x14)].m_envelope_state = 0;
	}
}

}
