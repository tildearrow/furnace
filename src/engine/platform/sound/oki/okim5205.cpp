// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/**********************************************************************************************
 *
 *   OKI MSM5205 ADPCM
 *
 *   TODO:
 *   3-bit ADPCM support
 *   4-bit ADPCM support
 *   maybe?
 *
 **********************************************************************************************/


#include "okim5205.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define COMMAND_STOP        (1 << 0)
#define COMMAND_PLAY        (1 << 1)
#define COMMAND_RECORD      (1 << 2)

#define STATUS_PLAYING      (1 << 1)
#define STATUS_RECORDING    (1 << 2)

static const int dividers[4] = { 128 };

/* step size index shift table */
static const int index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

/* lookup table for the precomputed difference */
static int diff_lookup[49*16];

/* tables computed? */
static int tables_computed = 0;




//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  okim5205_device - constructor
//-------------------------------------------------

okim5205_device::okim5205_device(uint32_t clock)
	:
		m_status(0),
		m_start_divider(0),
		m_divider(128),
		m_adpcm_type(0),
		m_data_in(0),
		m_nibble_shift(0),
		m_output_bits(0),
		m_signal(0),
		m_step(0),
                m_clock(clock)
{
}



/**********************************************************************************************

     compute_tables -- compute the difference tables

***********************************************************************************************/

static void compute_tables()
{
	/* nibble to bit map */
	static const int nbl2bit[16][4] =
	{
		{ 1, 0, 0, 0}, { 1, 0, 0, 1}, { 1, 0, 1, 0}, { 1, 0, 1, 1},
		{ 1, 1, 0, 0}, { 1, 1, 0, 1}, { 1, 1, 1, 0}, { 1, 1, 1, 1},
		{-1, 0, 0, 0}, {-1, 0, 0, 1}, {-1, 0, 1, 0}, {-1, 0, 1, 1},
		{-1, 1, 0, 0}, {-1, 1, 0, 1}, {-1, 1, 1, 0}, {-1, 1, 1, 1}
	};

	int step, nib;

	/* loop over all possible steps */
	for (step = 0; step <= 48; step++)
	{
		/* compute the step value */
		int stepval = floor(16.0 * pow(11.0 / 10.0, (double)step));

		/* loop over all nibbles and compute the difference */
		for (nib = 0; nib < 16; nib++)
		{
			diff_lookup[step*16 + nib] = nbl2bit[nib][0] *
				(stepval   * nbl2bit[nib][1] +
					stepval/2 * nbl2bit[nib][2] +
					stepval/4 * nbl2bit[nib][3] +
					stepval/8);
		}
	}

	tables_computed = 1;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void okim5205_device::device_start()
{
	compute_tables();

	m_divider = dividers[m_start_divider];

	m_signal = -2;
	m_step = 0;
  m_has_data = false;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void okim5205_device::device_reset()
{
	m_signal = -2;
	m_step = 0;
	m_status = 0;
  m_has_data = false;
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void okim5205_device::sound_stream_update(short* output, int len)
{
	short* buffer = output;

	if (m_status & STATUS_PLAYING)
	{
		int nibble_shift = m_nibble_shift;

		for (int sampindex = 0; sampindex < len; sampindex++)
		{
			/* Compute the new amplitude and update the current step */
			int nibble = (m_data_in >> nibble_shift) & 0xf;

			/* Output to the buffer */
			int16_t sample = clock_adpcm(nibble);

			nibble_shift ^= 4;

      if (nibble_shift==0) m_has_data=false;

			buffer[sampindex]=sample;
		}

		/* Update the parameters */
		m_nibble_shift = nibble_shift;
	}
	else
	{
		memset(buffer,0,len*sizeof(short));
	}
}



int16_t okim5205_device::clock_adpcm(uint8_t nibble)
{
	int32_t max = (1 << (m_output_bits - 1)) - 1;
	int32_t min = -(1 << (m_output_bits - 1));

	m_signal += diff_lookup[m_step * 16 + (nibble & 15)];

	/* clamp to the maximum */
	if (m_signal > max)
		m_signal = max;
	else if (m_signal < min)
		m_signal = min;

	/* adjust the step size and clamp */
	m_step += index_shift[nibble & 7];
	if (m_step > 48)
		m_step = 48;
	else if (m_step < 0)
		m_step = 0;

	/* return the signal scaled up to 32767 */
	return m_signal << 4;
}


/**********************************************************************************************

     okim5205::set_divider -- set the master clock divider

***********************************************************************************************/

void okim5205_device::set_divider(int val)
{
	m_divider = dividers[val];
}


/**********************************************************************************************

     okim5205::set_clock -- set the master clock

***********************************************************************************************/

void okim5205_device::device_clock_changed()
{
}


/**********************************************************************************************

     okim5205::get_vclk -- get the VCLK/sampling frequency

***********************************************************************************************/

int okim5205_device::get_vclk()
{
	return (m_clock / m_divider);
}


/**********************************************************************************************

     okim5205_status_r -- read the status port of an OKIM5205-compatible chip

***********************************************************************************************/

uint8_t okim5205_device::status_r()
{
	return (m_status & STATUS_PLAYING) ? 0x00 : 0x80;
}


/**********************************************************************************************

     okim5205_data_w -- write to the control port of an OKIM5205-compatible chip

***********************************************************************************************/
bool okim5205_device::data_w(uint8_t data)
{
  if (m_has_data) return false;
	m_data_in = data;
	m_nibble_shift = 0;
  m_has_data = true;
  return true;
}


/**********************************************************************************************

     okim5205_ctrl_w -- write to the control port of an OKIM5205-compatible chip

***********************************************************************************************/

void okim5205_device::ctrl_w(uint8_t data)
{
	if (data & COMMAND_STOP)
	{
		m_status &= ~(STATUS_PLAYING | STATUS_RECORDING);
    m_has_data = false;
		return;
	}

	if (data & COMMAND_PLAY)
	{
		if (!(m_status & STATUS_PLAYING))
		{
			m_status |= STATUS_PLAYING;

			/* Also reset the ADPCM parameters */
			m_signal = -2;
			m_step = 0;
			m_nibble_shift = 0;
		}
	}
	else
	{
		m_status &= ~STATUS_PLAYING;
	}

	if (data & COMMAND_RECORD)
	{
		//logerror("M5205: Record enabled\n");
		m_status |= STATUS_RECORDING;
	}
	else
	{
		m_status &= ~STATUS_RECORDING;
	}
}
