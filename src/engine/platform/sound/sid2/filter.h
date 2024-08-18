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
#include "spline.h"

// ----------------------------------------------------------------------------
// The SID2 filter is modeled with a two-integrator-loop biquadratic filter,
// which has been confirmed by Bob Yannes to be the actual circuit used in
// the SID2 chip.
//
// Measurements show that excellent emulation of the SID2 filter is achieved,
// except when high resonance is combined with high sustain levels.
// In this case the SID2 op-amps are performing less than ideally and are
// causing some peculiar behavior of the SID2 filter. This however seems to
// have more effect on the overall amplitude than on the color of the sound.
//
// The theory for the filter circuit can be found in "Microelectric Circuits"
// by Adel S. Sedra and Kenneth C. Smith.
// The circuit is modeled based on the explanation found there except that
// an additional inverter is used in the feedback from the bandpass output,
// allowing the summer op-amp to operate in single-ended mode. This yields
// inverted filter outputs with levels independent of Q, which corresponds with
// the results obtained from a real SID2.
//
// We have been able to model the summer and the two integrators of the circuit
// to form components of an IIR filter.
// Vhp is the output of the summer, Vbp is the output of the first integrator,
// and Vlp is the output of the second integrator in the filter circuit.
//
// According to Bob Yannes, the active stages of the SID2 filter are not really
// op-amps. Rather, simple NMOS inverters are used. By biasing an inverter
// into its region of quasi-linear operation using a feedback resistor from
// input to output, a MOS inverter can be made to act like an op-amp for
// small signals centered around the switching threshold.
//
// Qualified guesses at SID2 filter schematics are depicted below.
//
// SID2 filter
// ----------
// 
//     -----------------------------------------------
//    |                                               |
//    |            ---Rq--                            |
//    |           |       |                           |
//    |  ------------<A]-----R1---------              |
//    | |                               |             |
//    | |                        ---C---|      ---C---|
//    | |                       |       |     |       |
//    |  --R1--    ---R1--      |---Rs--|     |---Rs--| 
//    |        |  |       |     |       |     |       |
//     ----R1--|-----[A>--|--R-----[A>--|--R-----[A>--|
//             |          |             |             |
// vi -----R1--           |             |             |
// 
//                       vhp           vbp           vlp
// 
// 
// vi  - input voltage
// vhp - highpass output
// vbp - bandpass output
// vlp - lowpass output
// [A> - op-amp
// R1  - summer resistor
// Rq  - resistor array controlling resonance (4 resistors)
// R   - NMOS FET voltage controlled resistor controlling cutoff frequency
// Rs  - shunt resitor
// C   - capacitor
// 
// 
// 
// SID2 integrator
// --------------
// 
//                                   V+
// 
//                                   |
//                                   |
//                              -----|
//                             |     |
//                             | ||--
//                              -||
//                   ---C---     ||->
//                  |       |        |
//                  |---Rs-----------|---- vo
//                  |                |
//                  |            ||--
// vi ----     -----|------------||
//        |   ^     |            ||->
//        |___|     |                |
//        -----     |                |
//          |       |                |
//          |---R2--                 |
//          |
//          R1                       V-
//          |
//          |
// 
//          Vw
//
// ----------------------------------------------------------------------------
class Filter2
{
public:
  Filter2();

  void enable_filter(bool enable);
  void set_chip_model(chip_model2 model);

  RESID_INLINE
  void clock(sound_sample voice,
	     sound_sample ext_in);
  void reset();

  // Write registers.
  void writeFC_LO(reg8);
  void writeFC_HI(reg8);
  void writeRES(reg8);
  void writeMODE_VOL(reg8);

  // SID2 audio output (16 bits).
  sound_sample output();

  // Spline functions.
  void fc_default(const fc_point*& points, int& count);
  PointPlotter2<sound_sample> fc_plotter();

protected:
  void set_w0();
  void set_Q();

  // Filter2 enabled.
  bool enabled;

  // Filter2 cutoff frequency.
  reg12 fc;

  // Filter2 resonance.
  reg8 res;

  // Selects which inputs to route through filter.
  reg8 filt;

  // Highpass, bandpass, and lowpass filter modes.
  reg8 hp_bp_lp;

  // Mixer DC offset.
  sound_sample mixer_DC;

  // State of filter.
  float Vhp; // highpass
  float Vbp; // bandpass
  float Vlp; // lowpass
  sound_sample Vnf; // not filtered

  // Cutoff frequency, resonance.
  float w0, w0_ceil_1;
  float _1024_div_Q;

  // Cutoff frequency tables.
  // FC is an 11 bit register.
  sound_sample f0_8580[4096];
  sound_sample* f0;
  static fc_point f0_points_6581[];
  static fc_point f0_points_8580[];
  fc_point* f0_points;
  int f0_count;

friend class SID2;
};


// ----------------------------------------------------------------------------
// Inline functions.
// The following functions are defined inline because they are called every
// time a sample is calculated.
// ----------------------------------------------------------------------------

#if RESID_INLINING || defined(__FILTER_CC__)

// ----------------------------------------------------------------------------
// SID2 clocking - 1 cycle.
// ----------------------------------------------------------------------------
RESID_INLINE
void Filter2::clock(sound_sample voice, sound_sample ext_in)
{
  // Scale each voice down from 20 to 13 bits.
  voice >>= 3;
  (void)(ext_in);

  // This is handy for testing.
  /*if (!enabled) {
    Vnf = voice + ext_in;
    Vhp = Vbp = Vlp = 0;
    return;
  }*/

  // Route voices into or around filter.
  // The code below is expanded to a switch for faster execution.
  // (filt1 ? Vi : Vnf) += voice1;
  // (filt2 ? Vi : Vnf) += voice2;
  // (filt3 ? Vi : Vnf) += voice3;

  float Vi;

  if(filt)
  {
    Vnf = 0;
    Vi = voice;
  }
  else
  {
    Vnf = voice;
    Vi = 0;
    return;
  }
    
  // delta_t = 1 is converted to seconds given a 1MHz clock by dividing
  // with 1 000 000.

  // Calculate filter outputs.
  // Vhp = Vbp/Q - Vlp - Vi;
  // dVbp = -w0*Vhp*dt;
  // dVlp = -w0*Vbp*dt;

  float dVbp = (w0_ceil_1 * Vhp);
  float dVlp = (w0_ceil_1 * Vbp);
  Vbp -= dVbp;
  Vlp -= dVlp;
  Vhp = (Vbp * _1024_div_Q) - Vlp - Vi;
}

// ----------------------------------------------------------------------------
// SID2 audio output (20 bits).
// ----------------------------------------------------------------------------
RESID_INLINE
sound_sample Filter2::output()
{

  // Mix highpass, bandpass, and lowpass outputs. The sum is not
  // weighted, this can be confirmed by sampling sound output for
  // e.g. bandpass, lowpass, and bandpass+lowpass from a SID2 chip.

  // The code below is expanded to a switch for faster execution.
  // if (hp) Vf += Vhp;
  // if (bp) Vf += Vbp;
  // if (lp) Vf += Vlp;

  sound_sample Vf;

  switch (hp_bp_lp) {
  default:
  case 0x0:
    Vf = 0;
    break;
  case 0x1:
    Vf = Vlp;
    break;
  case 0x2:
    Vf = Vbp;
    break;
  case 0x3:
    Vf = Vlp + Vbp;
    break;
  case 0x4:
    Vf = Vhp;
    break;
  case 0x5:
    Vf = Vlp + Vhp;
    break;
  case 0x6:
    Vf = Vbp + Vhp;
    break;
  case 0x7:
    Vf = Vlp + Vbp + Vhp;
    break;
  }

  // Sum non-filtered and filtered output.
  // Multiply the sum with volume.
  return (Vnf + Vf);
}

#endif // RESID_INLINING || defined(__FILTER_CC__)

