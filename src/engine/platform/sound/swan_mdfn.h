/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* sound.h - WonderSwan Sound Emulation
**  Copyright (C) 2007-2016 Mednafen Team
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef __WSWAN_SOUND_H
#define __WSWAN_SOUND_H

#include <stdint.h>
#include "blip_buf.h"
#include "../../dispatch.h"

class WSwan
{
public:
 int32_t SoundFlush(int16_t *SoundBuf, const int32_t MaxSoundFrames);

 void SoundWrite(uint32_t, uint8_t);
 uint8_t SoundRead(uint32_t);
 void SoundReset(void);
 void SoundCheckRAMWrite(uint32_t A);

 void SoundUpdate();
 void RAMWrite(uint32_t, uint8_t);
 
 int32_t sample_cache[4][2];

 // Blip_Synth<blip_good_quality, 4096> WaveSynth;

 // Blip_Buffer *sbuf[2] = { NULL };
 blip_buffer_t* sbuf[2];
 DivDispatchOscBuffer* oscBuf[4];

 uint16_t period[4];
 uint8_t volume[4]; // left volume in upper 4 bits, right in lower 4 bits
 uint8_t voice_volume;

 uint8_t sweep_step, sweep_value;
 uint8_t noise_control;
 uint8_t control;
 uint8_t output_control;

 int32_t sweep_8192_divider;
 uint8_t sweep_counter;
 uint8_t SampleRAMPos;

 int32_t last_v_val;

 uint8_t HyperVoice;
 int32_t last_hv_val[2];
 uint8_t HVoiceCtrl, HVoiceChanCtrl;

 int32_t period_counter[4];
 int32_t last_val[4][2]; // Last outputted value, l&r
 uint8_t sample_pos[4];
 uint16_t nreg;
 uint32_t last_ts;
 uint32_t v30mz_timestamp;

 uint8_t wsRAM[64];
};

#endif
