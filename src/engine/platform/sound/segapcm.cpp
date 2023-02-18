// license:BSD-3-Clause
// copyright-holders:Hiromitsu Shioya, Olivier Galibert
/*********************************************************/
/*    SEGA 16ch 8bit PCM                                 */
/*********************************************************/

#include <string.h>

#include "segapcm.h"

//-------------------------------------------------
//  segapcm_device - constructor
//-------------------------------------------------

segapcm_device::segapcm_device()
	: m_bankshift(12)
	, m_bankmask(0x70)
{
  memset(m_muted,0,16*sizeof(bool));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void segapcm_device::device_start()
{
        memset(m_ram,255,0x800);
        memset(m_low,0,16);
        memset(lastOut,0,16*2*sizeof(short));
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void segapcm_device::sound_stream_update(int* outputs)
{
	/* clear the buffers */
        outputs[0]=0;
        outputs[1]=0;

	// reg      function
	// ------------------------------------------------
	// 0x00     ?
	// 0x01     ?
	// 0x02     volume left
	// 0x03     volume right
	// 0x04     loop address (08-15)
	// 0x05     loop address (16-23)
	// 0x06     end address
	// 0x07     address delta
	// 0x80     ?
	// 0x81     ?
	// 0x82     ?
	// 0x83     ?
	// 0x84     current address (08-15), 00-07 is internal?
	// 0x85     current address (16-23)
	// 0x86     bit 0: channel disable?
	//          bit 1: loop disable
	//          other bits: bank
	// 0x87     ?

	/* loop over channels */
	for (int ch = 0; ch < 16; ch++)
	{
		uint8_t *regs = &m_ram[8*ch];

		/* only process active channels */
		if (!(regs[0x86]&1))
		{
			int offset = (regs[0x86] & m_bankmask) << m_bankshift;
			uint32_t addr = (regs[0x85] << 16) | (regs[0x84] << 8) | m_low[ch];
			uint32_t loop = (regs[0x05] << 16) | (regs[0x04] << 8);
			uint8_t end = regs[6] + 1;

			int8_t v;
                        bool fetch=true;

			/* handle looping if we've hit the end */
			if ((addr >> 16) == end)
			{
				if (regs[0x86] & 2)
				{
					regs[0x86] |= 1;
                                        fetch=false;
				}
				else addr = loop;
			}

			/* fetch the sample */
                        if (fetch) {
                                v = read_byte(offset + (addr >> 8)) - 0x80;

                                /* apply panning and advance */
                                if (m_muted[ch]) {
                                  lastOut[ch][0]=0;
                                  lastOut[ch][1]=0;
                                } else {
                                  lastOut[ch][0]=v * (regs[2] & 0x7f);
                                  lastOut[ch][1]=v * (regs[3] & 0x7f);
                                }
                                outputs[0]+=lastOut[ch][0];
                                outputs[1]+=lastOut[ch][1];
                                addr = (addr + regs[7]) & 0xffffff;
                        } else {
                          lastOut[ch][0]=0;
                          lastOut[ch][1]=0;
                        }

			/* store back the updated address */
			regs[0x84] = addr >> 8;
			regs[0x85] = addr >> 16;
			m_low[ch] = regs[0x86] & 1 ? 0 : addr;
		}
	}
}


void segapcm_device::write(unsigned int offset, uint8_t data)
{
	m_ram[offset & 0x07ff] = data;
}


uint8_t segapcm_device::read(unsigned int offset)
{
	return m_ram[offset & 0x07ff];
}

uint8_t* segapcm_device::get_ram() {
  return m_ram;
}

void segapcm_device::mute(int ch, bool doMute) {
  m_muted[ch&15]=doMute;
}