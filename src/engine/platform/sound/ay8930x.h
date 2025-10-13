// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

  AY8930X emulation core by cam900

  Heavily based on ay8910.cpp MAME core

***************************************************************************/
#ifndef MAME_SOUND_AY8930X_H
#define MAME_SOUND_AY8930X_H

#pragma once

#include <algorithm>

/*
 * This define specifies the initial state of
 * YM2149, YM3439, AY8930 pin 26 (SEL pin).
 * By default it is set to high,
 * compatible with AY8910.
 */
/* TODO: make it controllable while it's running (used by any hw???) */
#define AY8930X_PIN26_HIGH           (0x00) /* or N/C */
#define AY8930X_PIN26_LOW            (0x10)

#define BIT(x,n) (((x)>>(n))&1)


class ay8930x_device
{
public:
	// construction/destruction
	ay8930x_device(unsigned int clock, bool clk_sel);

	// configuration helpers
	void set_flags(int flags) { m_flags = flags; }
	void set_psg_type(bool clk_sel) { set_type(clk_sel); }
	void set_resistors_load(int res_load0, int res_load1, int res_load2) { m_res_load[0] = res_load0; m_res_load[1] = res_load1; m_res_load[2] = res_load2; }

	unsigned char data_r() { return ay8930x_read_ym(); }
	void address_w(unsigned char data);
	void data_w(unsigned char data);

	// /RES
	void reset_w(unsigned char data = 0) { ay8930x_reset_ym(); }

	// Clock select pin
	void set_clock_sel(bool clk_sel)
	{
		if (clk_sel)
			m_flags |= AY8930X_PIN26_LOW;
		else
			m_flags &= ~AY8930X_PIN26_LOW;

		m_step_mul = is_clock_divided() ? 2 : 1;
		m_env_step_mul = m_step_mul;
		m_env_step_mul <<= 1;
	}

	// use this when BC1 == A0; here, BC1=0 selects 'data' and BC1=1 selects 'latch address'
	void data_address_w(int offset, unsigned char data) { ay8930x_write_ym(~offset & 1, data); } // note that directly connecting BC1 to A0 puts data on 0 and address on 1

	// use this when BC1 == !A0; here, BC1=0 selects 'latch address' and BC1=1 selects 'data'
	void address_data_w(int offset, unsigned char data) { ay8930x_write_ym(offset & 1, data); }

	// bc1=a0, bc2=a1
	void write_bc1_bc2(int offset, unsigned char data);

	struct ay_ym_param
	{
		double r_up;
		double r_down;
		int    res_count;
		double res[32];
	};

	struct mosfet_param
	{
		double m_Vth;
		double m_Vg;
		int    m_count;
		double m_Kn[32];
	};

	// device-level overrides
	void device_start();
	void device_reset();

	// sound stream update overrides
	void sound_stream_update(short* outputs, int outLen);

	void ay8930x_write_ym(int addr, unsigned char data);
	unsigned char ay8930x_read_ym();
	void ay8930x_reset_ym();

public:
	static constexpr int NUM_CHANNELS = 3;

	/* register id's */
	enum
	{
		AY_AFINE    = 0x00,
		AY_ACOARSE  = 0x01,
		AY_BFINE    = 0x02,
		AY_BCOARSE  = 0x03,
		AY_CFINE    = 0x04,
		AY_CCOARSE  = 0x05,
		AY_NOISEPER = 0x06,
		AY_ENABLE   = 0x07,
		AY_AVOL     = 0x08,
		AY_BVOL     = 0x09,
		AY_CVOL     = 0x0a,
		AY_EAFINE   = 0x0b,
		AY_EACOARSE = 0x0c,
		AY_EASHAPE  = 0x0d,
		AY_PORTA    = 0x0e,
		AY_PORTB    = 0x0f,
		AY_EBFINE   = 0x10,
		AY_EBCOARSE = 0x11,
		AY_ECFINE   = 0x12,
		AY_ECCOARSE = 0x13,
		AY_EBSHAPE  = 0x14,
		AY_ECSHAPE  = 0x15,
		AY_ADUTY    = 0x16,
		AY_BDUTY    = 0x17,
		AY_CDUTY    = 0x18,
		AY_NOISEAND = 0x19,
		AY_NOISEOR  = 0x1a,
		AY_NB_PER   = 0x1b,
		AY_NC_PER   = 0x1c,
		AY_EASHIFT  = 0x0e,
		AY_EBSHIFT  = 0x1d,
		AY_ECSHIFT  = 0x1e
	};

	// structs
	struct tone_t
	{
		unsigned int period;
		unsigned char volume;
		unsigned char duty;
		int count;
		unsigned char duty_cycle;
		unsigned char output;

		void reset()
		{
			period = 1;
			volume = 0;
			duty = 0;
			count = 0;
			duty_cycle = 0;
			output = 0;
		}

		void set_period(unsigned char fine, unsigned char coarse)
		{
			period = std::max<unsigned int>(1, fine | (coarse << 8));
		}

		void set_volume(unsigned char val)
		{
			volume = val;
		}

		void set_duty(unsigned char val)
		{
			duty = val;
		}
	};

	struct envelope_t
	{
		unsigned int period;
		int count;
		signed char step;
		unsigned int volume;
		unsigned char hold, alternate, attack, holding;
		unsigned char shift;

		void reset()
		{
			period = 1;
			count = 0;
			step = 0;
			volume = 0;
			hold = 0;
			alternate = 0;
			attack = 0;
			holding = 0;
			shift = 0;
		}

		void set_period(unsigned char fine, unsigned char coarse)
		{
			period = std::max<unsigned int>(1, fine | (coarse << 8));
		}

		void set_shift(unsigned char amount)
		{
			shift = amount & 7;
		}

		void set_shape(unsigned char shape, unsigned char mask)
		{
			attack = (shape & 0x04) ? mask : 0x00;
			if ((shape & 0x08) == 0)
			{
				/* if Continue = 0, map the shape to the equivalent one which has Continue = 1 */
				hold = 1;
				alternate = attack;
			}
			else
			{
				hold = shape & 0x01;
				alternate = shape & 0x02;
			}
			step = mask;
			holding = 0;
			volume = (step ^ attack);
		}
	};

	struct noise_t
	{
		unsigned char prescale_noise;
		unsigned char period;
		signed short noise_value;
		signed short count_noise;
		unsigned int rng;
		unsigned char noise_out;

		void reset()
		{
			prescale_noise = 0;
			period = 0;
			noise_value = 0;
			count_noise = 0;
			rng = 0x1ffff;
			noise_out = 0;
		}

		void set_period(unsigned char per)
		{
			period = std::max<unsigned int>(1, per);
		}

		inline void noise_rng_tick()
		{
			// The Random Number Generator of the 8910 is a 17-bit shift
			// register. The input to the shift register is bit0 XOR bit2
			// (bit0 is the output). This was verified on AY-3-8910 and YM2149 chips.

			rng = (rng >> 1) | ((BIT(rng, 0) ^ BIT(rng, 2)) << 16);
		}
	};

	// inlines
	inline bool tone_enable(int chan) { return BIT(m_regs[AY_ENABLE], chan); }
	inline unsigned char tone_lout(tone_t *tone) { return is_expanded_mode() ? BIT(tone->volume, 6) : 0; }
	inline unsigned char tone_rout(tone_t *tone) { return is_expanded_mode() ? BIT(tone->volume, 7) : 0; }
	inline unsigned char tone_volume(tone_t *tone) { return tone->volume & (is_expanded_mode() ? 0x1f : 0x0f); }
	inline unsigned char tone_envelope(tone_t *tone) { return (tone->volume >> (is_expanded_mode() ? 5 : 4)) & 1; }
	inline unsigned char tone_duty(tone_t *tone) { return is_expanded_mode() ? (tone->duty & 0x1f) : 0x10; }
	inline unsigned char get_envelope_chan(int chan) { return is_expanded_mode() ? chan : 0; }

	inline bool noise_enable(int chan) { return BIT(m_regs[AY_ENABLE], 3 + chan); }
	inline unsigned char noise_period(noise_t *noise) { return std::max<unsigned char>(1, noise->period); }
	inline unsigned char noise_output(noise_t *noise) { return is_expanded_mode() ? noise->noise_out & 1 : noise->rng & 1; }
	inline unsigned char get_noise_chan(int chan) { return is_expanded_mode() ? chan : 0; }

	inline bool is_expanded_mode() { return (((m_mode & 0x1) == 0x1)); }
	inline unsigned char get_register_bank() { return is_expanded_mode() ? (m_mode & 0x2) << 3 : 0; }

	inline unsigned char noise_and() { return m_regs[AY_NOISEAND] & 0xff; }
	inline unsigned char noise_or() { return m_regs[AY_NOISEOR] & 0xff; }

	inline bool is_clock_divided() { return (m_flags & AY8930X_PIN26_LOW); }

	// internal helpers
	void set_type(bool clk_sel);
	void ay8930x_write_reg(int r, int v);
	void build_mixer_table();

	// internal state
	int m_ready;
	//sound_stream *m_channel;
	bool m_active;
	unsigned char m_register_latch;
	unsigned char m_regs[16 * 2];
	int m_last_enable;
	tone_t m_tone[NUM_CHANNELS];
	envelope_t m_envelope[NUM_CHANNELS];
	noise_t m_noise[NUM_CHANNELS];
	unsigned char m_mode;
	unsigned char m_env_step_mask;
	/* init parameters ... */
	int m_step_mul;
	int m_env_step_mul;
	int m_zero_is_off;
	unsigned char m_vol_enabled[NUM_CHANNELS];
	const ay_ym_param *m_par;
	const ay_ym_param *m_par_env;
	short m_vol_table[NUM_CHANNELS][16];
	short m_env_table[NUM_CHANNELS][32];
	short m_vol3d_table[32*32*32*8];
	int m_flags;          /* Flags */
	int m_res_load[3];    /* Load on channel in ohms */
};

#endif // MAME_DEVICES_SOUND_AY8910_H
