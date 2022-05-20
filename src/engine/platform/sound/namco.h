// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Aaron Giles
#ifndef MAME_SOUND_NAMCO_H
#define MAME_SOUND_NAMCO_H

#include <stdio.h>
#include <stdint.h>
#include <memory>

class namco_audio_device
{
public:
	// configuration
	void set_voices(int voices) { m_voices = voices; }
	void set_stereo(bool stereo) { m_stereo = stereo; }

	void sound_enable_w(int state);

	static constexpr unsigned MAX_VOICES = 8;
	static constexpr unsigned MAX_VOLUME = 16;

	/* this structure defines the parameters for a channel */
	struct sound_channel
	{
		uint32_t frequency;
		uint32_t counter;
		int32_t volume[2];
		int32_t noise_sw;
		int32_t noise_state;
		int32_t noise_seed;
		uint32_t noise_counter;
		int32_t noise_hold;
		int32_t waveform_select;
	};

	namco_audio_device(uint32_t clock);

	// device-level overrides
	void device_start(unsigned char* wavePtr);
	void device_clock_changed(int clk);

	// internal state

	void build_decoded_waveform( uint8_t *rgnbase );
	void update_namco_waveform(int offset, uint8_t data);
	uint32_t namco_update_one(short* buffer, int size, const int16_t *wave, uint32_t counter, uint32_t freq);

	/* waveform region */
	uint8_t* m_wave_ptr;

	/* data about the sound system */
	sound_channel m_channel_list[MAX_VOICES];
	sound_channel *m_last_channel;
	uint8_t *m_wavedata;

	/* global sound parameters */
	int m_wave_size;
	bool m_sound_enable;
	int m_namco_clock;
	int m_sample_rate;
	int m_f_fracbits;

	int m_voices;     /* number of voices */
	bool m_stereo;    /* set to indicate stereo (e.g., System 1) */

	uint8_t m_waveram_alloc[0x400];

	/* decoded waveform table */
	int16_t m_waveform[MAX_VOLUME][512];

	virtual void sound_stream_update(short** outputs, int len);
        virtual ~namco_audio_device() {}
};

class namco_device : public namco_audio_device
{
public:
	namco_device(uint32_t clock);

	void pacman_sound_w(int offset, uint8_t data);

	uint8_t polepos_sound_r(int offset);
	void polepos_sound_w(int offset, uint8_t data);

        ~namco_device() {}

private:
	uint8_t m_soundregs[0x400];
};


class namco_15xx_device : public namco_audio_device
{
public:
	namco_15xx_device(uint32_t clock);

	void namco_15xx_w(int offset, uint8_t data);
	uint8_t sharedram_r(int offset);
	void sharedram_w(int offset, uint8_t data);

       ~namco_15xx_device() {}

private:
	uint8_t m_soundregs[0x400];
};


class namco_cus30_device : public namco_audio_device
{
public:
	namco_cus30_device(uint32_t clock);

	void namcos1_cus30_w(int offset, uint8_t data);   /* wavedata + sound registers + RAM */
	uint8_t namcos1_cus30_r(int offset);
	void namcos1_sound_w(int offset, uint8_t data);

	void pacman_sound_w(int offset, uint8_t data);

        ~namco_cus30_device() {}
};

#endif // MAME_SOUND_NAMCO_H
