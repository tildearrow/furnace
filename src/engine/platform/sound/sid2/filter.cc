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

#define __FILTER_CC__
#include "filter.h"

// Maximum cutoff frequency is specified as
// FCmax = 2.6e-5/C = 2.6e-5/2200e-12 = 11818.
//
// Measurements indicate a cutoff frequency range of approximately
// 220Hz - 18kHz on a MOS6581_2 fitted with 470pF capacitors. The function
// mapping FC to cutoff frequency has the shape of the tanh function, with
// a discontinuity at FCHI = 0x80.
// In contrast, the MOS8580_2 almost perfectly corresponds with the
// specification of a linear mapping from 30Hz to 12kHz.
// 
// The mappings have been measured by feeding the SID2 with an external
// signal since the chip itself is incapable of generating waveforms of
// higher fundamental frequency than 4kHz. It is best to use the bandpass
// output at full resonance to pick out the cutoff frequency at any given
// FC setting.
//
// The mapping function is specified with spline interpolation points and
// the function values are retrieved via table lookup.
//
// NB! Cutoff frequency characteristics may vary, we have modeled two
// particular Commodore 64s.

fc_point Filter2::f0_points_8580[] =
{
  //  FC      f         FCHI FCLO
  // ----------------------------
  {    0,     0 },   // 0x00      - repeated end point
  {    0,     0 },   // 0x00
  {  128 * 2,   800 },   // 0x10
  {  256 * 2,  1600 },   // 0x20
  {  384 * 2,  2500 },   // 0x30
  {  512 * 2,  3300 },   // 0x40
  {  640 * 2,  4100 },   // 0x50
  {  768 * 2,  4800 },   // 0x60
  {  896 * 2,  5600 },   // 0x70
  { 1024 * 2,  6500 },   // 0x80
  { 1152 * 2,  7500 },   // 0x90
  { 1280 * 2,  8400 },   // 0xa0
  { 1408 * 2,  9200 },   // 0xb0
  { 1536 * 2,  9800 },   // 0xc0
  { 1664 * 2, 10500 },   // 0xd0
  { 1792 * 2, 11000 },   // 0xe0
  { 1920 * 2, 11700 },   // 0xf0
  { 4095, 12500 },   // 0xff 0x07
  { 4095, 12500 }    // 0xff 0x07 - repeated end point
};


// ----------------------------------------------------------------------------
// Constructor.
// ----------------------------------------------------------------------------
Filter2::Filter2()
{
  fc = 0;

  res = 0;

  filt = 0;

  hp_bp_lp = 0;

  // State of filter.
  Vhp = 0;
  Vbp = 0;
  Vlp = 0;
  Vnf = 0;

  enable_filter(true);

  interpolate2(f0_points_8580, f0_points_8580
	      + sizeof(f0_points_8580)/sizeof(*f0_points_8580) - 1,
	      PointPlotter2<sound_sample>(f0_8580), 1.0);

  set_chip_model(MOS8580_2);
}


// ----------------------------------------------------------------------------
// Enable filter.
// ----------------------------------------------------------------------------
void Filter2::enable_filter(bool enable)
{
  enabled = enable;
}


// ----------------------------------------------------------------------------
// Set chip model.
// ----------------------------------------------------------------------------
void Filter2::set_chip_model(chip_model2 model)
{
  mixer_DC = 0;

  f0 = f0_8580;
  f0_points = f0_points_8580;
  f0_count = sizeof(f0_points_8580)/sizeof(*f0_points_8580);

  set_w0();
  set_Q();
}


// ----------------------------------------------------------------------------
// SID2 reset.
// ----------------------------------------------------------------------------
void Filter2::reset()
{
  fc = 0;

  res = 0;

  filt = 0;

  hp_bp_lp = 0;

  // State of filter.
  Vhp = 0;
  Vbp = 0;
  Vlp = 0;
  Vnf = 0;

  set_w0();
  set_Q();
}


// ----------------------------------------------------------------------------
// Register functions.
// ----------------------------------------------------------------------------
void Filter2::writeFC_LO(reg8 fc_lo)
{
  fc = (fc & 0xff0) | (fc_lo & 0xf);
  set_w0();
}

void Filter2::writeFC_HI(reg8 fc_hi)
{
  fc = (fc & 0xf) | (fc_hi << 4);
  set_w0();
}

void Filter2::writeRES(reg8 res_filt)
{
  res = res_filt;
  set_Q();
}

void Filter2::writeMODE_VOL(reg8 mode_vol)
{
  hp_bp_lp = (mode_vol >> 4) & 0x07;
  filt = mode_vol >> 7;

  //vol = mode_vol & 0x0f;
}

// Set filter cutoff frequency.
void Filter2::set_w0()
{
  const double pi = 3.1415926535897932385;

  // Multiply with 1.048576 to facilitate division by 1 000 000 by right-
  // shifting 20 times (2 ^ 20 = 1048576).
  w0 = (2.0*pi*(float)f0[fc]) / 1000000.0;

  // Limit f0 to 16kHz to keep 1 cycle filter stable.
  const float w0_max_1 = (2.0*pi*16000.0) / 1000000.0;
  w0_ceil_1 = w0 <= w0_max_1 ? w0 : w0_max_1;
}

// Set filter resonance.
void Filter2::set_Q()
{
  // Q is controlled linearly by res. Q has approximate range [0.707, 1.7].
  // As resonance is increased, the filter must be clocked more often to keep
  // stable.

  // The coefficient 1024 is dispensed of later by right-shifting 10 times
  // (2 ^ 10 = 1024).
  _1024_div_Q = (1.0/(0.707 + 2.5*(float)res/(float)0x0ff));
}

// ----------------------------------------------------------------------------
// Spline functions.
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Return the array of spline interpolation points used to map the FC register
// to filter cutoff frequency.
// ----------------------------------------------------------------------------
void Filter2::fc_default(const fc_point*& points, int& count)
{
  points = f0_points;
  count = f0_count;
}

// ----------------------------------------------------------------------------
// Given an array of interpolation points p with n points, the following
// statement will specify a new FC mapping:
//   interpolate2(p, p + n - 1, filter.fc_plotter(), 1.0);
// Note that the x2 range of the interpolation points *must* be [0, 2047],
// and that additional end points *must* be present since the end points
// are not interpolated.
// ----------------------------------------------------------------------------
PointPlotter2<sound_sample> Filter2::fc_plotter()
{
  return PointPlotter2<sound_sample>(f0);
}
