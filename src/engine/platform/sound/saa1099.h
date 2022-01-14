// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, Manuel Abadia
/**********************************************
    Philips SAA1099 Sound driver
**********************************************/

#ifndef MAME_SOUND_SAA1099_H
#define MAME_SOUND_SAA1099_H

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> saa1099_device

class saa1099_device
{
public:
	saa1099_device();

	void control_w(unsigned char data);
	void data_w(unsigned char data);

	void write(int offset, unsigned char data);

	// device-level overrides
	void device_start();
	void device_clock_changed();

	// sound stream update overrides
	void sound_stream_update(short** outputs, int len);

private:
	struct saa1099_channel
	{
		saa1099_channel() : amplitude{ 0, 0 }, envelope{ 0, 0 } { }

		unsigned char frequency      = 0;      // frequency (0x00..0xff)
		bool freq_enable  = false;  // frequency enable
		bool noise_enable = false;  // noise enable
		unsigned char octave         = 0;      // octave (0x00..0x07)
		unsigned short amplitude[2];           // amplitude
		unsigned char envelope[2];             // envelope (0x00..0x0f or 0x10 == off)

		/* vars to simulate the square wave */
		inline unsigned int freq() const { return (511 - frequency) << (8 - octave); } // clock / ((511 - frequency) * 2^(8 - octave))
		int counter = 0;
		unsigned char level = 0;
	};

	struct saa1099_noise
	{
		saa1099_noise() { }

		/* vars to simulate the noise generator output */
		int counter = 0;
		int freq = 0;
		unsigned int level = 0xffffffffU;    // noise polynomial shifter
	};

	void envelope_w(int ch);

	unsigned char m_noise_params[2];           // noise generators parameters
	bool m_env_enable[2];           // envelope generators enable
	bool m_env_reverse_right[2];    // envelope reversed for right channel
	unsigned char m_env_mode[2];               // envelope generators mode
	bool m_env_bits[2];             // true = 3 bits resolution
	bool m_env_clock[2];            // envelope clock mode (true external)
	unsigned char m_env_step[2];               // current envelope step
	bool m_all_ch_enable;           // all channels enable
	bool m_sync_state;              // sync all channels
	unsigned char m_selected_reg;              // selected register
	saa1099_channel m_channels[6];  // channels
	saa1099_noise m_noise[2];       // noise generators
};

#endif // MAME_SOUND_SAA1099_H
