// license:BSD-3-Clause
// copyright-holders:Hiromitsu Shioya, Olivier Galibert
/*********************************************************/
/*    SEGA 8bit PCM                                      */
/*********************************************************/

#ifndef MAMESOUND_SEGAPCM_H
#define MAMESOUND_SEGAPCM_H

#include <stdint.h>
#include <functional>

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class segapcm_device {
public:
	static constexpr int BANK_256    = 11;
	static constexpr int BANK_512    = 12;
	static constexpr int BANK_12M    = 13;
	static constexpr int BANK_MASK7  = 0x70 << 16;
	static constexpr int BANK_MASKF  = 0xf0 << 16;
	static constexpr int BANK_MASKF8 = 0xf8 << 16;

  short lastOut[16][2];

	segapcm_device();

	// configuration
	void set_bank(int bank) { m_bankshift = (bank & 0xf); m_bankmask = (0x70|((bank >> 16) & 0xfc)); }
        void set_read(std::function<unsigned char(unsigned int)> r) { read_byte = r; }

	void write(unsigned int offset, uint8_t data);
	uint8_t read(unsigned int offset);
  uint8_t* get_ram();
  void mute(int ch, bool doMute);

	// device-level overrides
	void device_start();

	// sound stream update overrides
	void sound_stream_update(int* outputs);

private:
	uint8_t m_ram[0x800];
	uint8_t m_low[16];
  bool m_muted[16];
	int m_bankshift;
	int m_bankmask;
        std::function<unsigned char(unsigned int)> read_byte;
};

#endif // MAMESOUND_SEGAPCM_H
