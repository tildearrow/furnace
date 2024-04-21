//  ---------------------------------------------------------------------------
//  This file is part of reSID, a MOS6581_2 SID2 emulator engine.
//  Copyright (C) 2004  Dag Lem <resid@nimrod.no>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  ---------------------------------------------------------------------------


#include "siddefs.h"
#include "wave.h"
#include "envelope.h"

class Voice2
{
public:
  Voice2();

  void set_chip_model(chip_model2 model);
  void set_sync_source(Voice2*);
  void reset();

  void writeCONTROL_REG(reg8);

  // Amplitude modulated waveform output.
  // Range [-2048*255, 2047*255].
  RESID_INLINE sound_sample output();
  RESID_INLINE sound_sample getDC();

protected:
  WaveformGenerator2 wave;
  EnvelopeGenerator2 envelope;

  // Waveform D/A zero level.
  sound_sample wave_zero;

  // Multiplying D/A DC offset.
  sound_sample voice_DC;

friend class SID2;
};


// ----------------------------------------------------------------------------
// Inline functions.
// The following function is defined inline because it is called every
// time a sample is calculated.
// ----------------------------------------------------------------------------

#if RESID_INLINING || defined(__VOICE_CC__)

// ----------------------------------------------------------------------------
// Amplitude modulated waveform output.
// Ideal range [-2048*255, 2047*255].
// ----------------------------------------------------------------------------
RESID_INLINE
sound_sample Voice2::output()
{
  // Multiply oscillator output with envelope output.
  return (wave.output() - wave_zero)*envelope.output();
}

RESID_INLINE
sound_sample Voice2::getDC()
{
  return voice_DC;
}

#endif // RESID_INLINING || defined(__VOICE_CC__)

