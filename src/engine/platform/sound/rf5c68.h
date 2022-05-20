// license:BSD-3-Clause
// copyright-holders:Olivier Galibert,Aaron Giles
/*********************************************************/
/*    ricoh RF5C68(or clone) PCM controller              */
/*********************************************************/

#include <vector>

#ifndef MAME_SOUND_RF5C68_H
#define MAME_SOUND_RF5C68_H

#pragma once

namespace rf5c68
{
	typedef unsigned char       u8;
	typedef signed char         s8;
	typedef unsigned short     u16;
	typedef signed short       s16;
	typedef unsigned int       u32;
	typedef signed int         s32;
	typedef signed int      offs_t;
}


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

#define RF5C68_SAMPLE_END_CB_MEMBER(_name)   void _name(int channel)


// ======================> rf5c68_device

using namespace rf5c68;
class rf5c68_device
{
public:
	rf5c68_device();

	u8 rf5c68_r(offs_t offset);
	void rf5c68_w(offs_t offset, u8 data);

	u8 rf5c68_mem_r(offs_t offset);
	void rf5c68_mem_w(offs_t offset, u8 data);

	void device_start(u8 *ext_mem);
	void device_reset();

	void sound_stream_update(s16 **outputs, s16 **channel_outputs, u32 samples);

protected:
	rf5c68_device(int output_bits);

private:
	static constexpr unsigned NUM_CHANNELS = 8;

	struct pcm_channel
	{
		pcm_channel() { }

		u8       enable;
		u8       env;
		u8       pan;
		u8       start;
		u32      addr;
		u16      step;
		u16      loopst;
	};

	pcm_channel      m_chan[NUM_CHANNELS];
	u8               m_cbank;
	u16              m_wbank;
	u8               m_enable;
	int              m_output_bits;
	std::vector<s32> m_mixleft;
	std::vector<s32> m_mixright;
	u8 			     *m_ext_mem;
};

class rf5c164_device : public rf5c68_device
{
public:
	rf5c164_device();
};

#endif // MAME_SOUND_RF5C68_H
