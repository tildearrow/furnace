// license:BSD-3-Clause
// copyright-holders:Acho A. Tang,R. Belmont
/*********************************************************

    Irem GA20 PCM Sound Chip

	DISCLAIMER
	- This file is modified for suitable in furnace.
	- modified by cam900

*********************************************************/
#ifndef MAME_SOUND_IREMGA20_H
#define MAME_SOUND_IREMGA20_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

using u8 = unsigned char;
using u32 = unsigned int;
using s32 = signed int;

class iremga20_intf
{
public:
	virtual u8 read_byte(u32 address) { return 0; };
};

// ======================> iremga20_device

class iremga20_device
{
public:
	iremga20_device(iremga20_intf &intf);

	void write(u32 offset, u8 data);
	u8 read(u32 offset);

	// device-level overrides
	void device_reset();

	// sound stream update overrides
	void sound_stream_update(short** outputs, int len);

private:
	struct channel_def
	{
		channel_def() :
			rate(0),
			pos(0),
			counter(0),
			end(0),
			volume(0),
			play(0)
		{
		}

		u32 rate;
		u32 pos;
		u32 counter;
		u32 end;
		u32 volume;
		bool play;
	};

	u8 m_regs[0x20];
	channel_def m_channel[4];

	iremga20_intf &m_intf;
};

#endif // MAME_SOUND_IREMGA20_H
