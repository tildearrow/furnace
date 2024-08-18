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

#include "sid.h"
#include <stdio.h>
#include <math.h>

// ----------------------------------------------------------------------------
// Constructor.
// ----------------------------------------------------------------------------
SID2::SID2()
{
  // Initialize pointers.
  sample = 0;
  fir = 0;

  voice[0].set_sync_source(&voice[2]);
  voice[1].set_sync_source(&voice[0]);
  voice[2].set_sync_source(&voice[1]);

  bus_value = 0;
  bus_value_ttl = 0;

  ext_in = 0;

  isMuted[0]=false;
  isMuted[1]=false;
  isMuted[2]=false;

  last_chan_out[0]=0;
  last_chan_out[1]=0;
  last_chan_out[2]=0;
}


// ----------------------------------------------------------------------------
// Destructor.
// ----------------------------------------------------------------------------
SID2::~SID2()
{
  delete[] sample;
  delete[] fir;
}

// ----------------------------------------------------------------------------
// Mute/unmute channel.
// ----------------------------------------------------------------------------
void SID2::set_is_muted(int ch, bool val) {
  if (ch<0 || ch>2) return;
  isMuted[ch]=val;
}


// ----------------------------------------------------------------------------
// Set chip model.
// ----------------------------------------------------------------------------
void SID2::set_chip_model(chip_model2 model)
{
  for (int i = 0; i < 3; i++) {
    voice[i].set_chip_model(model);
    filter[i].set_chip_model(model);
  }
}


// ----------------------------------------------------------------------------
// SID2 reset.
// ----------------------------------------------------------------------------
void SID2::reset()
{
  for (int i = 0; i < 3; i++)
  {
    voice[i].reset();
    filter[i].reset();
  }

  bus_value = 0;
  bus_value_ttl = 0;
}

// ----------------------------------------------------------------------------
// Read sample from audio output.
// Both 16-bit and n-bit output is provided.
// ----------------------------------------------------------------------------
int SID2::output()
{
  const int range = 1 << 16;
  const int half = range >> 1;
  int sample = (filter[0].output() + filter[1].output() + filter[2].output())/((4095*255 >> 7)*3*15*2/range);
  if (sample >= half) {
    return half - 1;
  }
  if (sample < -half) {
    return -half;
  }
  return sample;
}


// ----------------------------------------------------------------------------
// Read registers.
//
// Reading a write only register returns the last byte written to any SID2
// register. The individual bits in this value start to fade down towards
// zero after a few cycles. All bits reach zero within approximately
// $2000 - $4000 cycles.
// It has been claimed that this fading happens in an orderly fashion, however
// sampling of write only registers reveals that this is not the case.
// NB! This is not correctly modeled.
// The actual use of write only registers has largely been made in the belief
// that all SID2 registers are readable. To support this belief the read
// would have to be done immediately after a write to the same register
// (remember that an intermediate write to another register would yield that
// value instead). With this in mind we return the last value written to
// any SID2 register for $2000 cycles without modeling the bit fading.
// ----------------------------------------------------------------------------
reg8 SID2::read(reg8 offset)
{
  switch (offset) {
  case 0x1b:
    return voice[2].wave.readOSC();
  case 0x1c:
    return voice[2].envelope.readENV();
  default:
    return bus_value;
  }
}


// ----------------------------------------------------------------------------
// Write registers.
// ----------------------------------------------------------------------------
void SID2::write(reg8 offset, reg8 value)
{
  bus_value = value;
  bus_value_ttl = 0x2000;

  switch (offset) {
  case 0x00:
    voice[0].wave.writeFREQ_LO(value);
    break;
  case 0x01:
    voice[0].wave.writeFREQ_HI(value);
    break;
  case 0x02:
    voice[0].wave.writePW_LO(value);
    break;
  case 0x03:
    voice[0].wave.writePW_HI(value);
    voice[0].envelope.writeVOL(value);
    break;
  case 0x04:
    voice[0].writeCONTROL_REG(value);
    break;
  case 0x05:
    voice[0].envelope.writeATTACK_DECAY(value);
    break;
  case 0x06:
    voice[0].envelope.writeSUSTAIN_RELEASE(value);
    break;
  case 0x07:
    voice[1].wave.writeFREQ_LO(value);
    break;
  case 0x08:
    voice[1].wave.writeFREQ_HI(value);
    break;
  case 0x09:
    voice[1].wave.writePW_LO(value);
    break;
  case 0x0a:
    voice[1].wave.writePW_HI(value);
    voice[1].envelope.writeVOL(value);
    break;
  case 0x0b:
    voice[1].writeCONTROL_REG(value);
    break;
  case 0x0c:
    voice[1].envelope.writeATTACK_DECAY(value);
    break;
  case 0x0d:
    voice[1].envelope.writeSUSTAIN_RELEASE(value);
    break;
  case 0x0e:
    voice[2].wave.writeFREQ_LO(value);
    break;
  case 0x0f:
    voice[2].wave.writeFREQ_HI(value);
    break;
  case 0x10:
    voice[2].wave.writePW_LO(value);
    break;
  case 0x11:
    voice[2].wave.writePW_HI(value);
    voice[2].envelope.writeVOL(value);
    break;
  case 0x12:
    voice[2].writeCONTROL_REG(value);
    break;
  case 0x13:
    voice[2].envelope.writeATTACK_DECAY(value);
    break;
  case 0x14:
    voice[2].envelope.writeSUSTAIN_RELEASE(value);
    break;
  case 0x15:
    filter[0].writeFC_LO(value);
    filter[0].writeMODE_VOL(value);
    break;
  case 0x16:
    filter[0].writeFC_HI(value);
    break;
  case 0x17:
    filter[0].writeRES(value);
    break;
  case 0x18:
    filter[1].writeFC_LO(value);
    filter[1].writeMODE_VOL(value);
    break;
  case 0x19:
    filter[1].writeFC_HI(value);
    break;
  case 0x1a:
    filter[1].writeRES(value);
    break;
  case 0x1b:
    filter[2].writeFC_LO(value);
    filter[2].writeMODE_VOL(value);
    break;
  case 0x1c:
    filter[2].writeFC_HI(value);
    break;
  case 0x1d:
    filter[2].writeRES(value);
    break;
  case 0x1e:
    voice[0].wave.writeNOISE_MODE(value & 3);
    voice[1].wave.writeNOISE_MODE((value >> 2) & 3);
    voice[2].wave.writeNOISE_MODE((value >> 4) & 3);

    voice[0].wave.writeFREQ_HIGHEST((value >> 6) & 1);
    voice[1].wave.writeFREQ_HIGHEST((value >> 7) & 1);
    break;
  case 0x1f:
    voice[0].wave.writeMIX_MODE(value & 3);
    voice[1].wave.writeMIX_MODE((value >> 2) & 3);
    voice[2].wave.writeMIX_MODE((value >> 4) & 3);

    voice[2].wave.writeFREQ_HIGHEST((value >> 6) & 1);
    break;
  default:
    break;
  }
}


// ----------------------------------------------------------------------------
// Constructor.
// ----------------------------------------------------------------------------
SID2::State::State()
{
  int i;

  for (i = 0; i < 0x20; i++) {
    sid_register[i] = 0;
  }

  bus_value = 0;
  bus_value_ttl = 0;

  for (i = 0; i < 3; i++) {
    accumulator[i] = 0;
    shift_register[i] = 0x7ffff8;
    rate_counter[i] = 0;
    rate_counter_period[i] = 9;
    exponential_counter[i] = 0;
    exponential_counter_period[i] = 1;
    envelope_counter[i] = 0;
    envelope_state[i] = EnvelopeGenerator2::RELEASE;
    hold_zero[i] = true;
  }
}

// ----------------------------------------------------------------------------
// Enable filter.
// ----------------------------------------------------------------------------
void SID2::enable_filter(bool enable)
{
  for(int i = 0; i < 3; i++)
  {
    filter[i].enable_filter(enable);
  }
}

// ----------------------------------------------------------------------------
// Return array of default spline interpolation points to map FC to
// filter cutoff frequency.
// ----------------------------------------------------------------------------
void SID2::fc_default(const fc_point*& points, int& count)
{
  for(int i = 0; i < 3; i++)
  {
    filter[i].fc_default(points, count);
  }
}


// ----------------------------------------------------------------------------
// Return FC spline plotter object.
// ----------------------------------------------------------------------------
PointPlotter2<sound_sample> SID2::fc_plotter()
{
  return filter[0].fc_plotter();
}


// ----------------------------------------------------------------------------
// SID2 clocking - 1 cycle.
// ----------------------------------------------------------------------------
void SID2::clock()
{
  int i;

  // Age bus value.
  if (--bus_value_ttl <= 0) {
    bus_value = 0;
    bus_value_ttl = 0;
  }

  // Clock amplitude modulators.
  for (i = 0; i < 3; i++) {
    voice[i].envelope.clock();
  }

  // Clock oscillators.
  for (i = 0; i < 3; i++) {
    voice[i].wave.clock();
  }

  // Synchronize oscillators.
  for (i = 0; i < 3; i++) {
    voice[i].wave.synchronize();
  }

  // write voice output
  last_chan_out[0]=isMuted[0]?0:voice[0].output();
  last_chan_out[1]=isMuted[1]?0:voice[1].output();
  last_chan_out[2]=isMuted[2]?0:voice[2].output();

  // Clock filter.
  for(int i = 0; i < 3; i++)
  {
    if (voice[i].envelope.envelope_counter != 0 && voice[i].envelope.volume != 0 && !isMuted[i]) //clock filter only when there is signal (when signal is constant (no sound) filter makes CPU load skyrocket!)
    {
      filter[i].clock(last_chan_out[i], ext_in);
    }

    else if(filter[i].filt) //(sort of smoothly) remove DC offset that emerges because filter is frozen (and enabled)
    {
      if(filter[i].Vbp > 0)
      {
        filter[i].Vbp--;
      }

      if(filter[i].Vhp > 0)
      {
        filter[i].Vhp--;
      }

      if(filter[i].Vlp > 0)
      {
        filter[i].Vlp--;
      }

      if(filter[i].Vbp < 0)
      {
        filter[i].Vbp++;
      }

      if(filter[i].Vhp < 0)
      {
        filter[i].Vhp++;
      }

      if(filter[i].Vlp < 0)
      {
        filter[i].Vlp++;
      }
    }
  }

  for(int i = 0; i < 3; i++)
  {
    chan_out[i] = filter[i].output();
  }
}
