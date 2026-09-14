// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    NEC uPD1771

**********************************************************************/

#ifndef MAME_SOUND_UPD1771_H
#define MAME_SOUND_UPD1771_H

#include <stdint.h>


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class upd1771c_device
{
public:
	uint8_t read();
	void write(uint8_t data);
	void pcm_write(int state);

	// device-level overrides
	void device_reset();

	// sound stream update overrides
	void sound_stream_update(short* output, int len);

  short chout[4];

private:
	static constexpr unsigned MAX_PACKET_SIZE = 0x8000;

	// internal state

	uint8_t   m_packet[MAX_PACKET_SIZE];
	uint32_t  m_index;
	uint8_t   m_expected_bytes;

	uint8_t   m_state;//0:silence, 1 noise, 2 tone
	uint8_t   m_pc3;

	//tone
	uint8_t    m_t_timbre; //[0;  7]
	uint8_t    m_t_offset; //[0; 32]
	uint16_t   m_t_period; //[0;255]
	uint8_t    m_t_volume; //[0; 31]
	uint8_t    m_t_tpos;//timbre pos
	uint16_t   m_t_ppos;//period pos

	//noise wavetable LFSR
	uint8_t    m_nw_timbre; //[0;  7]
	uint8_t    m_nw_volume; //[0; 31]
	uint32_t   m_nw_period;
	uint32_t   m_nw_tpos;   //timbre pos
	uint32_t   m_nw_ppos;   //period pos

	//noise pulse components
	uint8_t    m_n_value[3];  //[0;1]
	uint16_t   m_n_volume[3]; //[0; 31]
	uint32_t   m_n_period[3];
	uint32_t   m_n_ppos[3];   //period pos
};

#endif // MAME_SOUND_UPD1771_H
