// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

// additional modifications by tildearrow, cam900 for furnace
#ifndef MAME_SOUND_SN76496_H
#define MAME_SOUND_SN76496_H

#include <stdlib.h>
#include <stdint.h>

typedef unsigned char u8;

class sn76496_base_device {
public:
	void stereo_w(u8 data);
	void write(u8 data);
	void device_start();
	void sound_stream_update(short* outputs, int outLen);
	inline int32_t get_channel_output(int ch) {
		return ((m_output[ch]!=0)?m_volume[ch]:0);
	}
	//DECLARE_READ_LINE_MEMBER( ready_r ) { return m_ready_state ? 1 : 0; }

	sn76496_base_device(
			int feedbackmask,
			int noise_start,
			int noisetap1,
			int noisetap2,
			bool negate,
			bool stereo,
			bool ncr,
			bool sega);

	sn76496_base_device(
			int feedbackmask,
			int noisetap1,
			int noisetap2,
			bool negate,
			bool stereo,
			bool ncr,
			bool sega)
		: sn76496_base_device(
			feedbackmask,
			feedbackmask,
			noisetap1,
			noisetap2,
			negate,
			stereo,
			ncr,
			sega)
		{}

public: // SORRY....
	inline bool     in_noise_mode();

	bool            m_ready_state;

	const int32_t   m_feedback_mask;    // mask for feedback
	const int32_t   m_noise_start;      // noise start value
	const int32_t   m_whitenoise_tap1;  // mask for white noise tap 1 (higher one, usually bit 14)
	const int32_t   m_whitenoise_tap2;  // mask for white noise tap 2 (lower one, usually bit 13)
	bool            m_negate;           // output negate flag
	const bool      m_stereo;           // whether we're dealing with stereo or not
	const bool      m_ncr_style_psg;    // flag to ignore writes to regs 1,3,5,6,7 with bit 7 low
	const bool      m_sega_style_psg;   // flag to make frequency zero acts as if it is one more than max (0x3ff+1) or if it acts like 0; the initial register is pointing to 0x3 instead of 0x0; the volume reg is preloaded with 0xF instead of 0x0

	int32_t           m_vol_table[16];    // volume table (for 4-bit to db conversion)
	int32_t           m_register[8];      // registers
	int32_t           m_last_register;    // last register written
	int32_t           m_volume[4];        // db volume of voice 0-2 and noise
	uint32_t          m_RNG;              // noise generator LFSR
	int32_t           m_stereo_mask;      // the stereo output mask
	int32_t           m_period[4];        // Length of 1/2 of waveform
	int32_t           m_count[4];         // Position within the waveform
	int32_t           m_output[4];        // 1-bit output of each channel, pre-volume
};

// SN76496: Whitenoise verified, phase verified, periodic verified (by Michael Zapf)
class sn76496_device : public sn76496_base_device
{
public:
	sn76496_device();
};

// Y2404 not verified yet. todo: verify; (don't be fooled by the Y, it's a TI chip, not Yamaha)
class y2404_device : public sn76496_base_device
{
public:
	y2404_device();
};

// SN76489 not verified yet. todo: verify;
class sn76489_device : public sn76496_base_device
{
public:
	sn76489_device();
};

// SN76489A: whitenoise verified, phase verified, periodic verified (by plgdavid)
class sn76489a_device : public sn76496_base_device
{
public:
	sn76489a_device();
};

// SN76494 not verified, (according to datasheet: same as sn76489a but without the /8 divider)
class sn76494_device : public sn76496_base_device
{
public:
	sn76494_device();
};

// SN94624 whitenoise verified, phase verified, period verified; verified by PlgDavid
class sn94624_device : public sn76496_base_device
{
public:
	sn94624_device();
};

// NCR8496 whitenoise verified, phase verified; verified by ValleyBell & NewRisingSun
class ncr8496_device : public sn76496_base_device
{
public:
	ncr8496_device();
};

// PSSJ-3 whitenoise verified, phase verified; verified by ValleyBell & NewRisingSun
class pssj3_device : public sn76496_base_device
{
public:
	pssj3_device();
};

// Verified by Justin Kerk
class gamegear_device : public sn76496_base_device
{
public:
	gamegear_device();
};

// todo: verify; from smspower wiki, assumed to have same invert as gamegear
class segapsg_device : public sn76496_base_device
{
public:
	segapsg_device();
};

#endif // MAME_SOUND_SN76496_H
