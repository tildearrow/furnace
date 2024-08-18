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

#define __WAVE_CC__
#include "wave.h"

// ----------------------------------------------------------------------------
// Constructor.
// ----------------------------------------------------------------------------
WaveformGenerator2::WaveformGenerator2()
{
  sync_source = this;

  set_chip_model(MOS6581_2);

  reset();
}


// ----------------------------------------------------------------------------
// Set sync source.
// ----------------------------------------------------------------------------
void WaveformGenerator2::set_sync_source(WaveformGenerator2* source)
{
  sync_source = source;
  source->sync_dest = this;
}


// ----------------------------------------------------------------------------
// Set chip model.
// ----------------------------------------------------------------------------
void WaveformGenerator2::set_chip_model(chip_model2 model)
{
  wave__ST = wave8580__ST;
  wave_P_T = wave8580_P_T;
  wave_PS_ = wave8580_PS_;
  wave_PST = wave8580_PST;
}


// ----------------------------------------------------------------------------
// Register functions.
// ----------------------------------------------------------------------------
void WaveformGenerator2::writeFREQ_LO(reg8 freq_lo)
{
  freq = (freq & 0x1ff00) | (freq_lo & 0x00ff);
}

void WaveformGenerator2::writeFREQ_HI(reg8 freq_hi)
{
  freq = ((freq_hi << 8) & 0xff00) | (freq & 0x100ff);
}

void WaveformGenerator2::writeFREQ_HIGHEST(reg8 freq_highest)
{
  freq = ((freq_highest << 16) & 0x10000) | (freq & 0xffff);
}

void WaveformGenerator2::writePW_LO(reg8 pw_lo)
{
  pw = (pw & 0xf00) | (pw_lo & 0x0ff);
}

void WaveformGenerator2::writePW_HI(reg8 pw_hi)
{
  pw = ((pw_hi << 8) & 0xf00) | (pw & 0x0ff);
}

void WaveformGenerator2::writeCONTROL_REG(reg8 control)
{
  waveform = (control >> 4) & 0x0f;
  ring_mod = control & 0x04;
  sync = control & 0x02;

  reg8 test_next = control & 0x08;

  // Test bit set.
  // The accumulator and the shift register are both cleared.
  // NB! The shift register is not really cleared immediately. It seems like
  // the individual bits in the shift register start to fade down towards
  // zero when test is set. All bits reach zero within approximately
  // $2000 - $4000 cycles.
  // This is not modeled. There should fortunately be little audible output
  // from this peculiar behavior.
  if (test_next) {
    accumulator = 0;
    shift_register = 0;
  }
  // Test bit cleared.
  // The accumulator starts counting, and the shift register is reset to
  // the value 0x7ffff8.
  // NB! The shift register will not actually be set to this exact value if the
  // shift register bits have not had time to fade to zero.
  // This is not modeled.
  else if (test) {
    shift_register = 0x7ffff8;
  }

  test = test_next;

  // The gate bit is handled by the EnvelopeGenerator2.
}

void WaveformGenerator2::writeNOISE_MODE(reg8 value)
{
  if(noise_mode != value)
  {
    shift_register = 0x7ffff8;
  }

  noise_mode = value;
}

void WaveformGenerator2::writeMIX_MODE(reg8 value)
{
  mix_mode = value;
}

reg8 WaveformGenerator2::readOSC()
{
  return output() >> 4;
}

// ----------------------------------------------------------------------------
// SID2 reset.
// ----------------------------------------------------------------------------
void WaveformGenerator2::reset()
{
  accumulator = 0;
  shift_register = 0x7ffff8;
  freq = 0;
  pw = 0;

  test = 0;
  ring_mod = 0;
  sync = 0;

  msb_rising = false;

  mix_mode = 0;
  noise_mode = 3;
}
