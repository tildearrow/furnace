// license:BSD-3-Clause
// copyright-holders:Acho A. Tang,R. Belmont, Valley Bell
/*********************************************************

Irem GA20 PCM Sound Chip
80 pin QFP, label NANAO GA20 (Nanao Corporation was Irem's parent company)

TODO:
- It's not currently known whether this chip is stereo.
- Is sample position base(regs 0,1) used while sample is playing, or
  latched at key on? We've always emulated it the latter way.
  gunforc2 seems to be the only game updating the address regs sometimes
  while a sample is playing, but it doesn't seem intentional.
- What is the 2nd sample address for? Is it end(cut-off) address, or
  loop start address? Every game writes a value that's past sample end.
- All games write either 0 or 2 to reg #6, do other bits have any function?


Revisions:

04-15-2002 Acho A. Tang
- rewrote channel mixing
- added prelimenary volume and sample rate emulation

05-30-2002 Acho A. Tang
- applied hyperbolic gain control to volume and used
  a musical-note style progression in sample rate
  calculation(still very inaccurate)

02-18-2004 R. Belmont
- sample rate calculation reverse-engineered.
  Thanks to Fujix, Yasuhiro Ogawa, the Guru, and Tormod
  for real PCB samples that made this possible.

02-03-2007 R. Belmont
- Cleaned up faux x86 assembly.

09-25-2018 Valley Bell & co
- rewrote channel update to make data 0 act as sample terminator


	DISCLAIMER
	- This file is modified for suitable in furnace.
	- modified by cam900

*********************************************************/

#include "iremga20.h"

#include <string.h>


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  iremga20_device - constructor
//-------------------------------------------------

iremga20_device::iremga20_device(iremga20_intf &intf) :
	m_regs{0},
	m_channel{channel_def(), channel_def(), channel_def(), channel_def()},
	m_intf(intf)
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void iremga20_device::device_reset()
{
	memset(m_regs, 0, 0x20 * sizeof(u8));
	for (int i = 0; i < 4; i++)
	{
		m_channel[i].rate = 0;
		m_channel[i].pos = 0;
		m_channel[i].counter = 0;
		m_channel[i].end = 0;
		m_channel[i].volume = 0;
		m_channel[i].play = false;
	}
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void iremga20_device::sound_stream_update(short** outputs, int len)
{
	for (int i = 0; i < len; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			s32 sampleout = 0;

			channel_def &ch = m_channel[j];
			if (ch.play)
			{
				int sample = m_intf.read_byte(ch.pos);
				if (sample == 0x00) // check for sample end marker
					ch.play = false;
				else
				{
					sampleout = (sample - 0x80) * (s32)ch.volume;
					ch.counter--;
					if (ch.counter <= ch.rate)
					{
						ch.pos++;
						ch.counter = 0x100;
					}
				}
			}
			outputs[j][i] = sampleout;
		}
	}
}

void iremga20_device::write(u32 offset, u8 data)
{
	offset &= 0x1f;
	m_regs[offset] = data;
	int ch = offset >> 3;

	// channel regs:
	// 0,1: start address
	// 2,3: end? address
	// 4: rate
	// 5: volume
	// 6: control
	// 7: voice status (read-only)

	switch (offset & 0x7)
	{
		case 4:
			m_channel[ch].rate = data;
			break;

		case 5:
			m_channel[ch].volume = (data * 256) / (data + 10);
			break;

		case 6:
			// d1: key on/off
			if (data & 2)
			{
				m_channel[ch].play = true;
				m_channel[ch].pos = (m_regs[ch << 3 | 0] | m_regs[ch << 3 | 1] << 8) << 4;
				m_channel[ch].end = (m_regs[ch << 3 | 2] | m_regs[ch << 3 | 3] << 8) << 4;
				m_channel[ch].counter = 0x100;
			}
			else
				m_channel[ch].play = false;

			// other: unknown/unused
			// possibilities are: loop flag, left/right speaker(stereo)
			break;
	}
}

u8 iremga20_device::read(u32 offset)
{
	offset &= 0x1f;
	int ch = offset >> 3;

	switch (offset & 0x7)
	{
		case 7: // voice status. bit 0 is 1 if active. (routine around 0xccc in rtypeleo)
			return m_channel[ch].play ? 1 : 0;
	}

	return 0;
}
