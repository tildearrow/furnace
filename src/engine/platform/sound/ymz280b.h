// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/**********************************************************************************************
 *
 *   Yamaha YMZ280B driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#include <memory>

#ifndef MAME_SOUND_YMZ280B_H
#define MAME_SOUND_YMZ280B_H

#pragma once

namespace ymz280b
{
	typedef unsigned char       u8;
	typedef signed char         s8;
	typedef unsigned short     u16;
	typedef signed short       s16;
	typedef unsigned int       u32;
	typedef signed int         s32;
	typedef signed int      offs_t;
}

using namespace ymz280b;
class ymz280b_device
{
public:
	ymz280b_device();

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	void device_start(u8 *ext_mem);
	void device_reset();

	void sound_stream_update(s16 **outputs, int samples);

private:
	/* struct describing a single playing ADPCM voice */
	struct YMZ280BVoice
	{
		u8 playing;          /* 1 if we are actively playing */
		bool ended;          /* indicate voice has ended in case samples_left is 0 */

		u8 keyon;            /* 1 if the key is on */
		u8 looping;          /* 1 if looping is enabled */
		u8 mode;             /* current playback mode */
		u16 fnum;            /* frequency */
		u8 level;            /* output level */
		u8 pan;              /* panning */

		u32 start;           /* start address, in nibbles */
		u32 stop;            /* stop address, in nibbles */
		u32 loop_start;      /* loop start address, in nibbles */
		u32 loop_end;        /* loop end address, in nibbles */
		u32 position;        /* current position, in nibbles */

		s32 signal;          /* current ADPCM signal */
		s32 step;            /* current ADPCM step */

		s32 loop_signal;     /* signal at loop start */
		s32 loop_step;       /* step at loop start */
		u32 loop_count;      /* number of loops so far */

		s32 output_left;     /* output volume (left) */
		s32 output_right;    /* output volume (right) */
		s32 output_step;     /* step value for frequency conversion */
		s32 output_pos;      /* current fractional position */
		s16 last_sample;     /* last sample output */
		s16 curr_sample;     /* current sample target */
		u8 irq_schedule;     /* 1 if the IRQ state is updated by timer */
	};

	void update_step(struct YMZ280BVoice *voice);
	void update_volumes(struct YMZ280BVoice *voice);
	int generate_adpcm(struct YMZ280BVoice *voice, s16 *buffer, int samples);
	int generate_pcm8(struct YMZ280BVoice *voice, s16 *buffer, int samples);
	int generate_pcm16(struct YMZ280BVoice *voice, s16 *buffer, int samples);
	void write_to_register(int data);
	int compute_status();

	// internal state
	struct YMZ280BVoice m_voice[8];   /* the 8 voices */
	u8 m_current_register;            /* currently accessible register */
	u8 m_status_register;             /* current status register */
	u8 m_irq_mask;                    /* current IRQ mask */
	u8 m_irq_enable;                  /* current IRQ enable */
	u8 m_keyon_enable;                /* key on enable */
	u8 m_ext_mem_enable;              /* external memory enable */
	u8 m_ext_readlatch;               /* external memory prefetched data */
	u32 m_ext_mem_address_hi;
	u32 m_ext_mem_address_mid;
	u32 m_ext_mem_address;            /* where the CPU can read the ROM */

	u8 *m_ext_mem;

	std::unique_ptr<s16[]> m_scratch;
};

#endif // MAME_SOUND_YMZ280B_H
