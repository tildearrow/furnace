// license:BSD-3-Clause
// copyright-holders:Olivier Galibert,Aaron Giles
/*********************************************************/
/*    ricoh RF5C68(or clone) PCM controller              */
/*                                                       */
/*    TODO: Verify RF5C105,164 (Sega CD/Mega CD)         */
/*           differences                                 */
/*********************************************************/

#include "rf5c68.h"
#include <algorithm>


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  rf5c68_device - constructor
//-------------------------------------------------

rf5c68_device::rf5c68_device(int output_bits)
	: m_cbank(0)
	, m_wbank(0)
	, m_enable(0)
	, m_output_bits(output_bits)
	, m_ext_mem(nullptr)
{
}

rf5c68_device::rf5c68_device()
	: rf5c68_device(10)
{
}


//-------------------------------------------------
//  rf5c164_device - constructor
//-------------------------------------------------

rf5c164_device::rf5c164_device()
	: rf5c68_device(16)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void rf5c68_device::device_start(u8 *ext_mem)
{
	m_ext_mem = ext_mem;
}

void rf5c68_device::device_reset()
{
	m_cbank = 0;
	m_wbank = 0;
	m_enable = 0;

	for (pcm_channel &chan : m_chan) {
		chan.enable = 0;
		chan.env = 0;
		chan.pan = 0;
		chan.start = 0;
		chan.addr = 0;
		chan.step = 0;
		chan.loopst = 0;
	}
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void rf5c68_device::sound_stream_update(s16 **outputs, s16 **channel_outputs, u32 samples)
{
	/* bail if not enabled */
	if (!m_enable)
	{
		std::fill_n(outputs[0], samples, 0);
		std::fill_n(outputs[1], samples, 0);
		for (unsigned int i = 0; i < NUM_CHANNELS; i++) {
			std::fill_n(channel_outputs[i*2], samples, 0);
			std::fill_n(channel_outputs[i*2+1], samples, 0);
		}
		return;
	}

	if (m_mixleft.size() < samples)
		m_mixleft.resize(samples);
	if (m_mixright.size() < samples)
		m_mixright.resize(samples);

	std::fill_n(&m_mixleft[0], samples, 0);
	std::fill_n(&m_mixright[0], samples, 0);

	/* loop over channels */
	for (unsigned int i = 0; i < NUM_CHANNELS; i++)
	{
		pcm_channel &chan = m_chan[i];
		/* if this channel is active, accumulate samples */
		if (chan.enable)
		{
			int lv = (chan.pan & 0x0f) * chan.env;
			int rv = ((chan.pan >> 4) & 0x0f) * chan.env;

			/* loop over the sample buffer */
			for (u32 j = 0; j < samples; j++)
			{
				int sample;

				/* fetch the sample and handle looping */
				sample = m_ext_mem[(chan.addr >> 11) & 0xffff];
				if (sample == 0xff)
				{
					chan.addr = chan.loopst << 11;
					sample = m_ext_mem[(chan.addr >> 11) & 0xffff];

					/* if we loop to a loop point, we're effectively dead */
					if (sample == 0xff)
						break;
				}
				chan.addr += chan.step;

				/* add to the buffer */
				s32 left_out = ((sample & 0x7f) * lv) >> 5;
				s32 right_out = ((sample & 0x7f) * rv) >> 5;
				if ((sample & 0x80) == 0)
				{
					left_out = -left_out;
					right_out = -right_out;
				}
				channel_outputs[i*2][j] = (s16)left_out;
				channel_outputs[i*2+1][j] = (s16)right_out;
				m_mixleft[j] += left_out;
				m_mixright[j] += right_out;
			}
		}
		else
		{
			std::fill_n(channel_outputs[i*2], samples, 0);
			std::fill_n(channel_outputs[i*2+1], samples, 0);
		}
	}

	/*
	now clamp and shift the result (output is only 10 bits for RF5C68, 16 bits for RF5C164)
	reference: Mega CD hardware manual, RF5C68 datasheet
	*/
	const u8 output_shift = (m_output_bits > 16) ? 0 : (16 - m_output_bits);
	const s32 output_nandmask = (1 << output_shift) - 1;
	for (u32 j = 0; j < samples; j++)
	{
		s32 outleft = std::min(std::max(m_mixleft[j], -32768), 32767);
		s32 outright = std::min(std::max(m_mixright[j], -32768), 32767);
		outputs[0][j] = outleft & ~output_nandmask;
		outputs[1][j] = outright & ~output_nandmask;
	}
}


//-------------------------------------------------
//    RF5C68 write register
//-------------------------------------------------

// TODO: RF5C164 only?
u8 rf5c68_device::rf5c68_r(offs_t offset)
{
	u8 shift;

	shift = (offset & 1) ? 11 + 8 : 11;

//  printf("%08x\n",(m_chan[(offset & 0x0e) >> 1].addr));

	return (m_chan[(offset & 0x0e) >> 1].addr) >> (shift);
}

void rf5c68_device::rf5c68_w(offs_t offset, u8 data)
{
	pcm_channel &chan = m_chan[m_cbank];
	int i;

	/* switch off the address */
	switch (offset)
	{
		case 0x00:  /* envelope */
			chan.env = data;
			break;

		case 0x01:  /* pan */
			chan.pan = data;
			break;

		case 0x02:  /* FDL */
			chan.step = (chan.step & 0xff00) | (data & 0x00ff);
			break;

		case 0x03:  /* FDH */
			chan.step = (chan.step & 0x00ff) | ((data << 8) & 0xff00);
			break;

		case 0x04:  /* LSL */
			chan.loopst = (chan.loopst & 0xff00) | (data & 0x00ff);
			break;

		case 0x05:  /* LSH */
			chan.loopst = (chan.loopst & 0x00ff) | ((data << 8) & 0xff00);
			break;

		case 0x06:  /* ST */
			chan.start = data;
			if (!chan.enable)
				chan.addr = chan.start << (8 + 11);
			break;

		case 0x07:  /* control reg */
			m_enable = (data >> 7) & 1;
			if (data & 0x40)
				m_cbank = data & 7;
			else
				m_wbank = (data & 0xf) << 12;
			break;

		case 0x08:  /* channel on/off reg */
			for (i = 0; i < 8; i++)
			{
				m_chan[i].enable = (~data >> i) & 1;
				if (!m_chan[i].enable)
					m_chan[i].addr = m_chan[i].start << (8 + 11);
			}
			break;
	}
}


//-------------------------------------------------
//    RF5C68 read memory
//-------------------------------------------------

u8 rf5c68_device::rf5c68_mem_r(offs_t offset)
{
	return m_ext_mem[m_wbank | offset];
}


//-------------------------------------------------
//    RF5C68 write memory
//-------------------------------------------------

void rf5c68_device::rf5c68_mem_w(offs_t offset, u8 data)
{
	m_ext_mem[m_wbank | offset] = data;
}
