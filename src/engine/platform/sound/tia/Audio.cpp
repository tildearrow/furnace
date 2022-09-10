//============================================================================
//
//   SSSS    tt          lll  lll
//  SS  SS   tt           ll   ll
//  SS     tttttt  eeee   ll   ll   aaaa
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2021 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#define _USE_MATH_DEFINES
#include "Audio.h"

#include <math.h>

namespace {
  constexpr double R_MAX = 30.;
  constexpr double R = 1.;

  short mixingTableEntry(unsigned char v, unsigned char vMax)
  {
    return static_cast<short>(
      floor(0x7fff * double(v) / double(vMax) * (R_MAX + R * double(vMax)) / (R_MAX + R * double(v)))
    );
  }
}

namespace TIA {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Audio::Audio()
{
  for (unsigned char i = 0; i <= 0x1e; ++i) myMixingTableSum[i] = mixingTableEntry(i, 0x1e);
  for (unsigned char i = 0; i <= 0x0f; ++i) myMixingTableIndividual[i] = mixingTableEntry(i, 0x0f);

  reset(false);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Audio::reset(bool st)
{
  myCounter = 0;
  mySampleIndex = 0;
  stereo = st;

  myCurrentSample[0]=0;
  myCurrentSample[1]=0;

  myChannel0.reset();
  myChannel1.reset();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Audio::tick()
{
  switch (myCounter) {
    case 9:
    case 81:
      myChannel0.phase0();
      myChannel1.phase0();

      break;

    case 37:
    case 149:
      phase1();
      break;
  }

  if (++myCounter == 228) myCounter = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Audio::write(unsigned char addr, unsigned char val) {
  switch (addr&0x3f) {
    case 0x15:
      myChannel0.audc(val);
      break;
    case 0x16:
      myChannel1.audc(val);
      break;
    case 0x17:
      myChannel0.audf(val);
      break;
    case 0x18:
      myChannel1.audf(val);
      break;
    case 0x19:
      myChannel0.audv(val);
      break;
    case 0x1a:
      myChannel1.audv(val);
      break;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Audio::phase1()
{
  unsigned char sample0 = myChannel0.phase1();
  unsigned char sample1 = myChannel1.phase1();

  addSample(sample0, sample1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Audio::addSample(unsigned char sample0, unsigned char sample1)
{
  if(stereo) {
    myCurrentSample[0] = myMixingTableIndividual[sample0];
    myCurrentSample[1] = myMixingTableIndividual[sample1];
  }
  else {
    myCurrentSample[0] = myMixingTableSum[sample0 + sample1];
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
AudioChannel& Audio::channel0()
{
  return myChannel0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
AudioChannel& Audio::channel1()
{
  return myChannel1;
}

}