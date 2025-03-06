/* Mednafen - Multi-system Emulator
 *
 *  Original skeleton write handler and PSG structure definition:
 *   Copyright (C) 2001 Charles MacDonald
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// additional modifications by tildearrow for furnace

#ifndef __MDFN_HW_SOUND_PCE_PSG_PCE_PSG_H
#define __MDFN_HW_SOUND_PCE_PSG_PCE_PSG_H

#include <stdint.h>
#include "../../blip_buf.h"
#include "../../dispatch.h"

class PCE_PSG;

struct psg_channel
{
  uint8_t waveform[32];     /* Waveform data */
  uint8_t waveform_index;   /* Waveform data index */
  uint8_t dda;
  uint8_t control;          /* Channel enable, DDA, volume */
  uint8_t noisectrl;        /* Noise enable/ctrl (channels 4,5 only) */

  int32_t vl[2];    //vll, vlr;

  int32_t counter;

  void (PCE_PSG::*UpdateOutput)(const int32_t timestamp, psg_channel *ch);
  DivDispatchOscBuffer* oscBuf;

  uint32_t freq_cache;
  uint32_t noise_freq_cache;        // Channel 4,5 only
  int32_t noisecount;
  uint32_t lfsr;

  int32_t samp_accum;         // The result of adding up all the samples in the waveform buffer(part of an optimization for high-frequency playback).
  int32_t blip_prev_samp[2];
  int32_t lastts;

  uint16_t frequency;       /* Channel frequency */
  uint8_t balance;          /* Channel balance */
};

// Only CH4 and CH5 have NCTRL and LFSR, but it's here for the other channels for "consistency".
enum
{
 PSG_GSREG_CH0_FREQ = 0x000,
// PSG_GSREG_CH0_COUNTER,
 PSG_GSREG_CH0_CTRL,
 PSG_GSREG_CH0_BALANCE,
 PSG_GSREG_CH0_WINDEX,
 PSG_GSREG_CH0_SCACHE,
 PSG_GSREG_CH0_NCTRL,
 PSG_GSREG_CH0_LFSR,

 PSG_GSREG_CH1_FREQ = 0x100,
// PSG_GSREG_CH1_COUNTER,
 PSG_GSREG_CH1_CTRL,
 PSG_GSREG_CH1_BALANCE,
 PSG_GSREG_CH1_WINDEX,
 PSG_GSREG_CH1_SCACHE,
 PSG_GSREG_CH1_NCTRL,
 PSG_GSREG_CH1_LFSR,

 PSG_GSREG_CH2_FREQ = 0x200,
// PSG_GSREG_CH2_COUNTER,
 PSG_GSREG_CH2_CTRL,
 PSG_GSREG_CH2_BALANCE,
 PSG_GSREG_CH2_WINDEX,
 PSG_GSREG_CH2_SCACHE,
 PSG_GSREG_CH2_NCTRL,
 PSG_GSREG_CH2_LFSR,

 PSG_GSREG_CH3_FREQ = 0x300,
// PSG_GSREG_CH3_COUNTER,
 PSG_GSREG_CH3_CTRL,
 PSG_GSREG_CH3_BALANCE,
 PSG_GSREG_CH3_WINDEX,
 PSG_GSREG_CH3_SCACHE,
 PSG_GSREG_CH3_NCTRL,
 PSG_GSREG_CH3_LFSR,

 PSG_GSREG_CH4_FREQ = 0x400,
// PSG_GSREG_CH4_COUNTER,
 PSG_GSREG_CH4_CTRL,
 PSG_GSREG_CH4_BALANCE,
 PSG_GSREG_CH4_WINDEX,
 PSG_GSREG_CH4_SCACHE,
 PSG_GSREG_CH4_NCTRL,
 PSG_GSREG_CH4_LFSR,

 PSG_GSREG_CH5_FREQ = 0x500,
// PSG_GSREG_CH5_COUNTER,
 PSG_GSREG_CH5_CTRL,
 PSG_GSREG_CH5_BALANCE,
 PSG_GSREG_CH5_WINDEX,
 PSG_GSREG_CH5_SCACHE,
 PSG_GSREG_CH5_NCTRL,
 PSG_GSREG_CH5_LFSR,

 PSG_GSREG_SELECT = 0x1000,
 PSG_GSREG_GBALANCE,
 PSG_GSREG_LFOFREQ,
 PSG_GSREG_LFOCTRL,
 _PSG_GSREG_COUNT
};

class PCE_PSG
{
  public:

  enum
  {
    REVISION_HUC6280 = 0,
    REVISION_HUC6280A,
    _REVISION_COUNT
  };


  PCE_PSG(int want_revision);
  ~PCE_PSG();

  void Power(const int32_t timestamp);
  void Write(int32_t timestamp, uint8_t A, uint8_t V);

  void SetVolume(double new_volume);

  void Update(int32_t timestamp);
  void ResetTS(int32_t ts_base = 0);

  // TODO: timestamp
  uint32_t GetRegister(const unsigned int id, char *special, const uint32_t special_len);
  void SetRegister(const unsigned int id, const uint32_t value);

  void PeekWave(const unsigned int ch, uint32_t Address, uint32_t Length, uint8_t *Buffer);
  void PokeWave(const unsigned int ch, uint32_t Address, uint32_t Length, const uint8_t *Buffer);
  
  psg_channel channel[6];

  blip_buffer_t* bb[2];

  private:

  void UpdateSubLFO(int32_t timestamp);
  void UpdateSubNonLFO(int32_t timestamp);

  void RecalcUOFunc(int chnum);
  void UpdateOutputSub(const int32_t timestamp, psg_channel *ch, const int32_t samp0, const int32_t samp1);
  void UpdateOutput_Off(const int32_t timestamp, psg_channel *ch);
  void UpdateOutput_Accum_HuC6280(const int32_t timestamp, psg_channel *ch);
  void UpdateOutput_Accum_HuC6280A(const int32_t timestamp, psg_channel *ch);
  void UpdateOutput_Norm(const int32_t timestamp, psg_channel *ch);
  void UpdateOutput_Noise(const int32_t timestamp, psg_channel *ch);
  void (PCE_PSG::*UpdateOutput_Accum)(const int32_t timestamp, psg_channel *ch);

  int32_t GetVL(const int chnum, const int lr);

  void RecalcFreqCache(int chnum);
  void RecalcNoiseFreqCache(int chnum);
  void RunChannel(int chc, int32_t timestamp, bool LFO_On);

  uint8_t select;               /* Selected channel (0-5) */
  uint8_t globalbalance;        /* Global sound balance */
  uint8_t lfofreq;              /* LFO frequency */
  uint8_t lfoctrl;              /* LFO control */

  int32_t vol_update_counter;
  int32_t vol_update_which;
  int32_t vol_update_vllatch;
  bool vol_pending;

  int32_t lastts;
  int revision;

  int32_t dbtable_volonly[32];

  int32_t dbtable[32][32];
};

#endif
