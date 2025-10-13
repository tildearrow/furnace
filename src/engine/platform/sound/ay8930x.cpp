// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

  AY8930X emulation core by cam900

  Heavily based on ay8910.cpp MAME core

***************************************************************************/

#include "ay8930x.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <vector>

/*************************************
 *
 *  constants
 *
 *************************************/

static constexpr float MAX_OUTPUT = 1.0;

/*************************************
 *
 *  Type definitions
 *
 *************************************/


/*************************************
 *
 *  Static
 *
 *************************************/

static const ay8930x_device::ay_ym_param ay8930x_param =
{
	630, 801,
	16,
	{ 73770, 37586, 27458, 21451, 15864, 12371, 8922,  6796,
		4763,  3521,  2403,  1737,  1123,   762,  438,   251 },
};

static const ay8930x_device::ay_ym_param ay8930x_param_env =
{
	630, 801,
	32,
	{ 103350, 73770, 52657, 37586, 32125, 27458, 24269, 21451,
		18447, 15864, 14009, 12371, 10506,  8922,  7787,  6796,
		5689,  4763,  4095,  3521,  2909,  2403,  2043,  1737,
		1397,  1123,   925,   762,   578,   438,   332,   251 },
};


/*************************************
 *
 *  Inline
 *
 *************************************/

static inline void build_single_table(double rl, const ay8930x_device::ay_ym_param *par, int normalize, short *tab, int zero_is_off)
{
	double rt;
	double rw;
	double temp[32], min = 10.0, max = 0.0;

	for (int j = 0; j < par->res_count; j++)
	{
		rt = 1.0 / par->r_down + 1.0 / rl;

		rw = 1.0 / par->res[j];
		rt += 1.0 / par->res[j];

		if (!(zero_is_off && j == 0))
		{
			rw += 1.0 / par->r_up;
			rt += 1.0 / par->r_up;
		}

		temp[j] = rw / rt;
		if (temp[j] < min)
			min = temp[j];
		if (temp[j] > max)
			max = temp[j];
	}
	if (normalize)
	{
		for (int j = 0; j < par->res_count; j++)
			tab[j] = 16384 * MAX_OUTPUT * (((temp[j] - min)/(max-min)) - 0.25) * 0.5;
	}
	else
	{
		for (int j = 0; j < par->res_count; j++)
			tab[j] = 16384 * MAX_OUTPUT * temp[j];
	}
}

/*************************************
 *
 * Static functions
 *
 *************************************/

void ay8930x_device::ay8930x_write_reg(int r, int v)
{
	if ((r & 0xf) == AY_PORTB) // shared register
		r &= 0xf;

	//if (r >= 11 && r <= 13) printf("%d %x %02x\n", PSG->index, r, v);
	m_regs[r] = v;
	unsigned char coarse;

	switch (r)
	{
		case AY_AFINE:
		case AY_ACOARSE:
			coarse = m_regs[AY_ACOARSE] & (is_expanded_mode() ? 0xff : 0xf);
			m_tone[0].set_period(m_regs[AY_AFINE], coarse);
			break;
		case AY_BFINE:
		case AY_BCOARSE:
			coarse = m_regs[AY_BCOARSE] & (is_expanded_mode() ? 0xff : 0xf);
			m_tone[1].set_period(m_regs[AY_BFINE], coarse);
			break;
		case AY_CFINE:
		case AY_CCOARSE:
			coarse = m_regs[AY_CCOARSE] & (is_expanded_mode() ? 0xff : 0xf);
			m_tone[2].set_period(m_regs[AY_CFINE], coarse);
			break;
		case AY_NOISEPER:
			m_noise[0].set_period(m_regs[AY_NOISEPER] & (is_expanded_mode() ? 0xff : 0x1f));
			break;
		case AY_AVOL:
			m_tone[0].set_volume(m_regs[AY_AVOL]);
			break;
		case AY_BVOL:
			m_tone[1].set_volume(m_regs[AY_BVOL]);
			break;
		case AY_CVOL:
			m_tone[2].set_volume(m_regs[AY_CVOL]);
			break;
		case AY_EACOARSE:
		case AY_EAFINE:
			m_envelope[0].set_period(m_regs[AY_EAFINE], m_regs[AY_EACOARSE]);
			break;
		case AY_ENABLE:
			m_last_enable = m_regs[AY_ENABLE];
			break;
		case AY_EASHAPE:
			m_envelope[0].set_shape(m_regs[AY_EASHAPE], m_env_step_mask);
			break;
		case AY_EBFINE:
		case AY_EBCOARSE:
			m_envelope[1].set_period(m_regs[AY_EBFINE], m_regs[AY_EBCOARSE]);
			break;
		case AY_ECFINE:
		case AY_ECCOARSE:
			m_envelope[2].set_period(m_regs[AY_ECFINE], m_regs[AY_ECCOARSE]);
			break;
		case AY_EBSHAPE:
			m_envelope[1].set_shape(m_regs[AY_EBSHAPE], m_env_step_mask);
			break;
		case AY_ECSHAPE:
			m_envelope[2].set_shape(m_regs[AY_ECSHAPE], m_env_step_mask);
			break;
		case AY_ADUTY:
			m_tone[0].set_duty(m_regs[AY_ADUTY]);
			break;
		case AY_BDUTY:
			m_tone[1].set_duty(m_regs[AY_BDUTY]);
			break;
		case AY_CDUTY:
			m_tone[2].set_duty(m_regs[AY_CDUTY]);
			break;
		case AY_NOISEAND:
		case AY_NOISEOR:
			// No action required
			break;
		case AY_NB_PER:
			m_noise[1].set_period(m_regs[AY_NB_PER]);
			break;
		case AY_NC_PER:
			m_noise[2].set_period(m_regs[AY_NC_PER]);
			break;
		case AY_EASHIFT:
			m_envelope[0].set_shift(m_regs[AY_EASHIFT]);
			break;
		case AY_EBSHIFT:
			m_envelope[1].set_shift(m_regs[AY_EBSHIFT]);
			break;
		case AY_ECSHIFT:
			m_envelope[2].set_shift(m_regs[AY_ECSHIFT]);
			break;
		case AY_PORTB:
		{
			const unsigned char old_mode = m_mode;
			m_mode = v & 0x3;
			if (old_mode != m_mode)
			{
				if (((old_mode & 0x1) == 0x1) ^ ((m_mode & 0x1) == 0x1)) // AY8930 expanded mode
				{
					for (int i = 0; i < AY_PORTB; i++)
					{
						ay8930x_write_reg(i, 0);
						ay8930x_write_reg(i + 0x10, 0);
					}
				}
			}
			break;
		}
		default:
			m_regs[r] = 0; // reserved, set as 0
			break;
	}
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void ay8930x_device::sound_stream_update(short* outputs, int advance)
{
	tone_t *tone;
	envelope_t *envelope;
	noise_t *noise;

	/* hack to prevent us from hanging when starting filtered outputs */
	if (!m_ready)
	{
		for (int chan = 0; chan < 6; chan++)
			outputs[chan] = 0;
	}

	/* The 8910 has three outputs, each output is the mix of one of the three */
	/* tone generators and of the (single) noise generator. The two are mixed */
	/* BEFORE going into the DAC. The formula to mix each channel is: */
	/* (ToneOn | ToneDisable) & (NoiseOn | NoiseDisable). */
	/* Note that this means that if both tone and noise are disabled, the output */
	/* is 1, not 0, and can be modulated changing the volume. */

	/* loop? kill the loop and optimize! */
	for (int chan = 0; chan < NUM_CHANNELS; chan++)
	{
		tone = &m_tone[chan];
		const int period = std::max<int>(1, tone->period) * (m_step_mul << 1);
		tone->count += advance << (is_expanded_mode() ? 5 : 0);
		if (tone->count >= period)
		{
			tone->duty_cycle = (tone->duty_cycle - (tone->count / period)) & 0x1f;
			tone->output = is_expanded_mode() ? ((tone->duty_cycle >= tone_duty(tone)) ? 0 : 1) : BIT(tone->duty_cycle, 0);
			tone->count = tone->count % period;
		}
	}
	for (int chan = 0; chan < (is_expanded_mode() ? NUM_CHANNELS : 1); chan++)
	{
		noise = &m_noise[chan];
		const int period_noise = (int)(noise_period(noise)) * m_step_mul;
		noise->count_noise += advance;
		if (noise->count_noise >= period_noise)
		{
			/* toggle the prescaler output. Noise is no different to
			* channels.
			*/
			noise->count_noise = 0;
			noise->prescale_noise = (noise->prescale_noise + 1) & 3;

			if (is_expanded_mode()) // AY8930 noise generator rate is twice? compares as compatibility mode
			{
				// This is called "Noise value" on the docs, but is a counter whose period is determined by the LFSR.
				// Using AND/OR gates, specific periods can be "filtered" out.
				// A square wave can be generated through this behavior, which can be used for crude AM pulse width modulation.

				// The period of the noise is determined by this value.
				// The least significant byte of the LFSR is bitwise ANDed with the AND mask, and then bitwise ORed with the OR mask.
				if ((++noise->noise_value) >= (((unsigned char)(noise->rng) & noise_and()) | noise_or())) // Clock the noise value.
				{
					noise->noise_value = 0;

					// When everything is finally said and done, a 1bit latch is flipped.
					// This is the final output of the noise, to be multiplied by the tone and envelope generators of the channel.
					noise->noise_out ^= 1;

					noise->noise_rng_tick();
				}
			}
			else if (!noise->prescale_noise)
				noise->noise_rng_tick();
		}
	}

	for (int chan = 0; chan < NUM_CHANNELS; chan++)
	{
		tone = &m_tone[chan];
		noise = &m_noise[get_noise_chan(chan)];
		m_vol_enabled[chan] = (tone->output | (unsigned char)tone_enable(chan)) & (noise_output(noise) | (unsigned char)noise_enable(chan));
	}

	/* update envelope */
	// who cares about env 1/2 on 8910
	for (int chan = 0; chan < (is_expanded_mode() ? NUM_CHANNELS : 1); chan++)
	{
		envelope = &m_envelope[chan];
		if (envelope->holding == 0)
		{
			const int period = std::max<int>(1, envelope->period) * m_env_step_mul;
			envelope->count += advance;
			if (envelope->count >= period)
			{
				envelope->count %= period;
				envelope->step--;

				/* check envelope current position */
				if (envelope->step < 0)
				{
					if (envelope->hold)
					{
						if (envelope->alternate)
							envelope->attack ^= m_env_step_mask;
						envelope->holding = 1;
						envelope->step = 0;
					}
					else
					{
						/* if CountEnv has looped an odd number of times (usually 1), */
						/* invert the output. */
						if (envelope->alternate && (envelope->step & (m_env_step_mask + 1)))
							envelope->attack ^= m_env_step_mask;

						envelope->step &= m_env_step_mask;
					}
				}

			}
		}
		envelope->volume = (envelope->step ^ envelope->attack);
	}

	for (int chan = 0; chan < NUM_CHANNELS; chan++)
	{
		short out = 0;
		tone = &m_tone[chan];
		if (tone_envelope(tone) != 0)
		{
			envelope = &m_envelope[get_envelope_chan(chan)];
			out = m_env_table[chan][m_vol_enabled[chan] ? (envelope->volume >> envelope->shift) : 0];
		}
		else
		{
			if (is_expanded_mode())
				out = m_env_table[chan][m_vol_enabled[chan] ? tone_volume(tone) : 0];
			else
				out = m_vol_table[chan][m_vol_enabled[chan] ? tone_volume(tone) : 0];
		}
		outputs[chan] = (!tone_lout(tone)) ? out : 0;
		outputs[chan + NUM_CHANNELS] = (!tone_rout(tone)) ? out : 0;
	}
}

void ay8930x_device::build_mixer_table()
{
	for (int chan = 0; chan < NUM_CHANNELS; chan++)
	{
		build_single_table(m_res_load[chan], m_par, 0, m_vol_table[chan], m_zero_is_off);
		build_single_table(m_res_load[chan], m_par_env, 0, m_env_table[chan], 0);
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ay8930x_device::device_start()
{
	build_mixer_table();
}


void ay8930x_device::ay8930x_reset_ym()
{
	m_active = false;
	m_register_latch = 0;
	m_mode = 0; // ay-3-8910 compatible mode
	for (int chan = 0; chan < NUM_CHANNELS; chan++)
	{
		m_tone[chan].reset();
		m_envelope[chan].reset();
		m_noise[chan].reset();
	}
	m_last_enable = -1;  /* force a write */
	for (int i = 0; i < AY_PORTB; i++)
		ay8930x_write_reg(i,0);
	m_ready = 1;
}

void ay8930x_device::ay8930x_write_ym(int addr, unsigned char data)
{
	if (addr & 1)
	{
		if (m_active)
		{
			const unsigned char register_latch = m_register_latch + get_register_bank();
			ay8930x_write_reg(register_latch, data);
		}
	}
	else
	{
		m_active = (data >> 4) == 0; // mask programmed 4-bit code
		if (m_active)
		{
			/* Register port */
			m_register_latch = data & 0x0f;
		}
	}
}

unsigned char ay8930x_device::ay8930x_read_ym()
{
	unsigned char r = m_register_latch + get_register_bank();

	if (!m_active) return 0xff; // high impedance

	if ((r & 0xf) == AY_PORTB) // shared register
		r &= 0xf;

	return m_regs[r];
}

/*************************************
 *
 * Sound Interface
 *
 *************************************/


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ay8930x_device::device_reset()
{
	ay8930x_reset_ym();
}

/*************************************
 *
 * Read/Write Handlers
 *
 *************************************/

void ay8930x_device::address_w(unsigned char data)
{
	ay8930x_write_ym(0, data);
}

void ay8930x_device::data_w(unsigned char data)
{
	ay8930x_write_ym(1, data);
}

// here, BC1 is hooked up to A0 on the host and BC2 is hooked up to A1
void ay8930x_device::write_bc1_bc2(int offset, unsigned char data)
{
	switch (offset & 3)
	{
	case 0: // latch address
		address_w(data);
		break;
	case 1: // inactive
		break;
	case 2: // write to psg
		data_w(data);
		break;
	case 3: // latch address
		address_w(data);
		break;
	}
}



ay8930x_device::ay8930x_device(unsigned int clock, bool clk_sel)
	: m_ready(0),
		m_active(false),
		m_register_latch(0),
		m_last_enable(0),
		m_mode(0),
		m_env_step_mask(0x1f),
		m_step_mul(1),
		m_env_step_mul(m_step_mul << 1),
		m_zero_is_off(1),
		m_par(&ay8930x_param),
		m_par_env(&ay8930x_param_env),
		m_flags((clk_sel ? AY8930X_PIN26_LOW : 0))
{
	memset(&m_regs,0,sizeof(m_regs));
	memset(&m_tone,0,sizeof(m_tone));
	memset(&m_envelope,0,sizeof(m_envelope));
	memset(&m_noise,0,sizeof(m_noise));
	memset(&m_vol_enabled,0,sizeof(m_vol_enabled));
	memset(&m_vol_table,0,sizeof(m_vol_table));
	memset(&m_env_table,0,sizeof(m_env_table));
	m_res_load[0] = m_res_load[1] = m_res_load[2] = 1000; //Default values for resistor loads

	// TODO : measure ay8930 volume parameters (PSG_TYPE_YM for temporary 5 bit handling)
	set_type(clk_sel);
}

void ay8930x_device::set_type(bool clk_sel)
{
	m_env_step_mask = 0x1f;
	m_env_step_mul = is_clock_divided() ? 2 : 1;
	m_zero_is_off = 0;
	m_par = &ay8930x_param;
	m_par_env = &ay8930x_param_env;
	m_env_step_mul <<= 1;

	set_clock_sel(clk_sel);
}

