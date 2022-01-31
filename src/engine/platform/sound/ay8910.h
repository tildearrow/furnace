// license:BSD-3-Clause
// copyright-holders:Couriersud
#ifndef MAME_SOUND_AY8910_H
#define MAME_SOUND_AY8910_H

#define ALL_8910_CHANNELS -1

/* Internal resistance at Volume level 7. */

#define AY8910_INTERNAL_RESISTANCE  (356)
#define YM2149_INTERNAL_RESISTANCE  (353)

/*
 * The following is used by all drivers not reviewed yet.
 * This will like the old behavior, output between
 * 0 and 7FFF
 */
#define AY8910_LEGACY_OUTPUT        (0x01)

/*
 * Specifying the next define will simulate the special
 * cross channel mixing if outputs are tied together.
 * The driver will only provide one stream in this case.
 */
#define AY8910_SINGLE_OUTPUT        (0x02)

/*
 * The following define is the default behavior.
 * Output level 0 is 0V and 7ffff corresponds to 5V.
 * Use this to specify that a discrete mixing stage
 * follows.
 */
#define AY8910_DISCRETE_OUTPUT      (0x04)

/*
 * The following define causes the driver to output
 * resistor values. Intended to be used for
 * netlist interfacing.
 */

#define AY8910_RESISTOR_OUTPUT      (0x08)

/*
 * This define specifies the initial state of
 * YM2149, YM3439, AY8930 pin 26 (SEL pin).
 * By default it is set to high,
 * compatible with AY8910.
 */
/* TODO: make it controllable while it's running (used by any hw???) */
#define YM2149_PIN26_HIGH           (0x00) /* or N/C */
#define YM2149_PIN26_LOW            (0x10)

#define BIT(x,n) (((x)>>(n))&1)

enum device_type {
  AY8910,
  AY8912,
  AY8913,
  AY8914,
  AY8930,
  YM2149,
  YM3439,
  YMZ284,
  YMZ294,
  SUNSOFT_5B_SOUND
};


class ay8910_device
{
public:
	enum psg_type_t
	{
		PSG_TYPE_AY,
		PSG_TYPE_YM
	};

	enum config_t
	{
		PSG_DEFAULT = 0x0,
		PSG_PIN26_IS_CLKSEL = 0x1,
		PSG_HAS_INTERNAL_DIVIDER = 0x2,
		PSG_EXTENDED_ENVELOPE = 0x4,
		PSG_HAS_EXPANDED_MODE = 0x8
	};

	// construction/destruction
	ay8910_device(unsigned int clock);

	// configuration helpers
	void set_flags(int flags) { m_flags = flags; }
	void set_psg_type(psg_type_t psg_type) { set_type(psg_type); }
	void set_resistors_load(int res_load0, int res_load1, int res_load2) { m_res_load[0] = res_load0; m_res_load[1] = res_load1; m_res_load[2] = res_load2; }

	unsigned char data_r() { return ay8910_read_ym(); }
	void address_w(unsigned char data);
	void data_w(unsigned char data);

	// /RES
	void reset_w(unsigned char data = 0) { ay8910_reset_ym(chip_type == AY8930); }

	// use this when BC1 == A0; here, BC1=0 selects 'data' and BC1=1 selects 'latch address'
	void data_address_w(int offset, unsigned char data) { ay8910_write_ym(~offset & 1, data); } // note that directly connecting BC1 to A0 puts data on 0 and address on 1

	// use this when BC1 == !A0; here, BC1=0 selects 'latch address' and BC1=1 selects 'data'
	void address_data_w(int offset, unsigned char data) { ay8910_write_ym(offset & 1, data); }

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

	// internal interface for PSG component of YM device
	// FIXME: these should be private, but vector06 accesses them directly

	ay8910_device(device_type type, unsigned int clock, psg_type_t psg_type, int streams, int ioports, int feature = PSG_DEFAULT);

	// device-level overrides
	void device_start();
	void device_reset();

	// sound stream update overrides
	void sound_stream_update(short** outputs, int outLen);

	void ay8910_write_ym(int addr, unsigned char data);
	unsigned char ay8910_read_ym();
	void ay8910_reset_ym(bool ay8930);

private:
	static constexpr int NUM_CHANNELS = 3;
  device_type chip_type;

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
		AY_TEST     = 0x1f
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
			period = 0;
			volume = 0;
			duty = 0;
			count = 0;
			duty_cycle = 0;
			output = 0;
		}

		void set_period(unsigned char fine, unsigned char coarse)
		{
			period = fine | (coarse << 8);
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

		void reset()
		{
			period = 0;
			count = 0;
			step = 0;
			volume = 0;
			hold = 0;
			alternate = 0;
			attack = 0;
			holding = 0;
		}

		void set_period(unsigned char fine, unsigned char coarse)
		{
			period = fine | (coarse << 8);
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

	// inlines
	inline bool tone_enable(int chan) { return BIT(m_regs[AY_ENABLE], chan); }
	inline unsigned char tone_volume(tone_t *tone) { return tone->volume & (is_expanded_mode() ? 0x1f : 0x0f); }
	inline unsigned char tone_envelope(tone_t *tone) { return (tone->volume >> (is_expanded_mode() ? 5 : 4)) & ((m_feature & PSG_EXTENDED_ENVELOPE) ? 3 : 1); }
	inline unsigned char tone_duty(tone_t *tone) { return is_expanded_mode() ? (tone->duty & 0x8 ? 0x8 : (tone->duty & 0xf)) : 0x4; }
	inline unsigned char get_envelope_chan(int chan) { return is_expanded_mode() ? chan : 0; }

	inline bool noise_enable(int chan) { return BIT(m_regs[AY_ENABLE], 3 + chan); }
	inline unsigned char noise_period() { return is_expanded_mode() ? m_regs[AY_NOISEPER] & 0xff : m_regs[AY_NOISEPER] & 0x1f; }
	inline unsigned char noise_output() { return is_expanded_mode() ? m_noise_latch & 1 : m_rng & 1; }

	inline bool is_expanded_mode() { return ((m_feature & PSG_HAS_EXPANDED_MODE) && ((m_mode & 0xe) == 0xa)); }
	inline unsigned char get_register_bank() { return is_expanded_mode() ? (m_mode & 0x1) << 4 : 0; }

	// internal helpers
	void set_type(psg_type_t psg_type);
	inline float mix_3D();
	void ay8910_write_reg(int r, int v);
	void build_mixer_table();

	// internal state
	psg_type_t m_type;
	int m_streams;
	int m_ready;
	//sound_stream *m_channel;
	bool m_active;
	int m_register_latch;
	unsigned char m_regs[16 * 2];
	int m_last_enable;
	tone_t m_tone[NUM_CHANNELS];
	envelope_t m_envelope[NUM_CHANNELS];
	unsigned char m_prescale_noise;
	int m_count_noise;
	int m_rng;
  unsigned int m_noise_and;
  unsigned int m_noise_or;
  unsigned int m_noise_value;
  unsigned int m_noise_latch;
	unsigned char m_mode;
	unsigned char m_env_step_mask;
	/* init parameters ... */
	int m_step;
	int m_zero_is_off;
	unsigned char m_vol_enabled[NUM_CHANNELS];
	const ay_ym_param *m_par;
	const ay_ym_param *m_par_env;
	short m_vol_table[NUM_CHANNELS][16];
	short m_env_table[NUM_CHANNELS][32];
	short m_vol3d_table[32*32*32*8];
	int m_flags;          /* Flags */
	int m_feature;        /* Chip specific features */
	int m_res_load[3];    /* Load on channel in ohms */
};

class ay8912_device : public ay8910_device
{
public:
	ay8912_device(unsigned int clock);
};

class ay8913_device : public ay8910_device
{
public:
	ay8913_device(unsigned int clock);
};

class ay8914_device : public ay8910_device
{
public:
	ay8914_device(unsigned int clock);

	/* AY8914 handlers needed due to different register map */
	unsigned char read(int offset);
	void write(int offset, unsigned char data);
};

class ay8930_device : public ay8910_device
{
public:
	ay8930_device(unsigned int clock);
};

class ym2149_device : public ay8910_device
{
public:
	ym2149_device(unsigned int clock);
};

class ym3439_device : public ay8910_device
{
public:
	ym3439_device(unsigned int clock);
};

class ymz284_device : public ay8910_device
{
public:
	ymz284_device(unsigned int clock);
};

class ymz294_device : public ay8910_device
{
public:
	ymz294_device(unsigned int clock);
};

class sunsoft_5b_sound_device : public ay8910_device
{
public:
	sunsoft_5b_sound_device(unsigned int clock);
};


#endif // MAME_DEVICES_SOUND_AY8910_H
