/*
	License: BSD-3-Clause
	see https://github.com/cam900/vgsound_emu/blob/vgsound_emu_v1/LICENSE for more details

	Copyright holder(s): cam900
	Seta/Allumer X1-010 Emulation core

	the chip has 16 voices, all voices can be switchable to Wavetable or PCM sample playback mode.
	It has also 2 output channels, but no known hardware using this feature for stereo sound.

	Wavetable needs to paired with envelope, it's always enabled and similar as AY PSG's one
	but its shape is stored at RAM.

	PCM volume is stored by each register.

	Both volume is 4bit per output.

	Everything except PCM sample is stored at paired 8 bit RAM.

	RAM layout (common case: Address bit 12 is swapped when RAM is shared with CPU)

	-----------------------------
	0000...007f Voice Registers

	0000...0007 Voice 0 Register

	Address Bits      Description
	        7654 3210
	0       x--- ---- Frequency divider*
	        ---- -x-- Envelope one-shot mode
	        ---- --x- Sound format
	        ---- --0- PCM
	        ---- --1- Wavetable
	        ---- ---x Keyon/off
	PCM case:
	1       xxxx xxxx Volume (Each nibble is for each output)

	2       xxxx xxxx Frequency*

	4       xxxx xxxx Start address / 4096

	5       xxxx xxxx 0x100 - (End address / 4096)
	Wavetable case:
	1       ---x xxxx Wavetable data select

	2       xxxx xxxx Frequency LSB*
	3       xxxx xxxx "" MSB

	4       xxxx xxxx Envelope period (.10 fixed point, Low 8 bit)

	5       ---x xxxx Envelope shape select (!= 0 : Reserved for Voice registers)

	0008...000f Voice 1 Register
	...
	0078...007f Voice 15 Register
	-----------------------------
	0080...0fff Envelope shape data (Same as volume; Each nibble is for each output)

	0080...00ff Envelope shape data 1
	0100...017f Envelope shape data 2
	...
	0f80...0fff Envelope shape data 31
	-----------------------------
	1000...1fff Wavetable data

	1000...107f Wavetable data 0
	1080...10ff Wavetable data 1
	...
	1f80...1fff Wavetable data 31
	-----------------------------

	* Frequency is 4.4 fixed point for PCM,
	  6.10 for Wavetable.
	  Frequency divider is higher precision or just right shift?
	  needs verification.
*/

#include "x1_010.hpp"

void x1_010_core::tick()
{
	// reset output
	m_out[0] = m_out[1] = 0;
	for (int i = 0; i < 16; i++)
	{
		voice_t &v = m_voice[i];
		v.tick();
		m_out[0] += v.data * v.vol_out[0];
		m_out[1] += v.data * v.vol_out[1];
	}
}

void x1_010_core::voice_t::tick()
{
	data = vol_out[0] = vol_out[1] = 0;
	if (flag.keyon)
	{
		if (flag.wavetable) // Wavetable
		{
			// envelope, each nibble is for each output
			u8 vol = m_host.m_envelope[(bitfield(end_envshape, 0, 5) << 7) | bitfield(env_acc, 10, 7)];
			vol_out[0] = bitfield(vol, 4, 4);
			vol_out[1] = bitfield(vol, 0, 4);
			env_acc += start_envfreq;
			if (flag.env_oneshot && bitfield(env_acc, 17))
				flag.keyon = false;
			else
				env_acc = bitfield(env_acc, 0, 17);
			// get wavetable data
			data = m_host.m_wave[(bitfield(vol_wave, 0, 5) << 7) | bitfield(acc, 10, 7)];
			acc = bitfield(acc + (freq >> flag.div), 0, 17);
		}
		else // PCM sample
		{
			// volume register, each nibble is for each output
			vol_out[0] = bitfield(vol_wave, 4, 4);
			vol_out[1] = bitfield(vol_wave, 0, 4);
			// get PCM sample
			data = m_host.m_intf.read_byte(bitfield(acc, 4, 20));
			acc += bitfield(freq, 0, 8) >> flag.div;
			if ((acc >> 16) > (0xff ^ end_envshape))
				flag.keyon = false;
		}
	}
}

u8 x1_010_core::ram_r(u16 offset)
{
	if (offset & 0x1000) // wavetable data
		return m_wave[offset & 0xfff];
	else if (offset & 0xf80) // envelope shape data
		return m_envelope[offset & 0xfff];
	else // channel register
		return m_voice[bitfield(offset, 3, 4)].reg_r(offset & 0x7);
}

void x1_010_core::ram_w(u16 offset, u8 data)
{
	if (offset & 0x1000) // wavetable data
		m_wave[offset & 0xfff] = data;
	else if (offset & 0xf80) // envelope shape data
		m_envelope[offset & 0xfff] = data;
	else // channel register
		m_voice[bitfield(offset, 3, 4)].reg_w(offset & 0x7, data);
}

u8 x1_010_core::voice_t::reg_r(u8 offset)
{
	switch (offset & 0x7)
	{
		case 0x00: return (flag.div << 7)
			        | (flag.env_oneshot << 2)
			        | (flag.wavetable << 1)
			        | (flag.keyon << 0);
		case 0x01: return vol_wave;
		case 0x02: return bitfield(freq, 0, 8);
		case 0x03: return bitfield(freq, 8, 8);
		case 0x04: return start_envfreq;
		case 0x05: return end_envshape;
		default: break;
	}
	return 0;
}

void x1_010_core::voice_t::reg_w(u8 offset, u8 data)
{
	switch (offset & 0x7)
	{
		case 0x00:
		{
			const bool prev_keyon = flag.keyon;
			flag.div = bitfield(data, 7);
			flag.env_oneshot = bitfield(data, 2);
			flag.wavetable = bitfield(data, 1);
			flag.keyon = bitfield(data, 0);
			if (!prev_keyon && flag.keyon) // Key on
			{
				acc = flag.wavetable ? 0 : (u32(start_envfreq) << 16);
				env_acc = 0;
			}
			break;
		}
		case 0x01:
			vol_wave = data;
			break;
		case 0x02:
			freq = (freq & 0xff00) | data;
			break;
		case 0x03:
			freq = (freq & 0x00ff) | (u16(data) << 8);
			break;
		case 0x04:
			start_envfreq = data;
			break;
		case 0x05:
			end_envshape = data;
			break;
		default:
			break;
	}
}

void x1_010_core::voice_t::reset()
{
	flag.reset();
	vol_wave = 0;
	freq = 0;
	start_envfreq = 0;
	end_envshape = 0;
	acc = 0;
	env_acc = 0;
	data = 0;
	vol_out[0] = vol_out[1] = 0;
}

void x1_010_core::reset()
{
	for (auto & elem : m_voice)
		elem.reset();

	std::fill_n(&m_envelope[0], 0x1000, 0);
	std::fill_n(&m_wave[0], 0x1000, 0);
	m_out[0] = m_out[1] = 0;
}
