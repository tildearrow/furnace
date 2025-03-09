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

#ifndef TIA_AUDIO_HXX
#define TIA_AUDIO_HXX

#include "AudioChannel.h"
#include <array>

namespace TIA {
  class Audio
  {
    public:
      Audio();

      void reset(bool stereo);

      void tick(int len);

      void write(unsigned char addr, unsigned char val);

      AudioChannel& channel0();

      AudioChannel& channel1();

      short myCurrentSample[2];
      short myChannelOut[2];

    private:
      void phase1();
      void addSample(unsigned char sample0, unsigned char sample1);

    public:
      int myCounter;
      unsigned char myPhase;

    private:
      AudioChannel myChannel0;
      AudioChannel myChannel1;

      bool stereo;

      std::array<short, 0x1e + 1> myMixingTableSum;
      std::array<short, 0x0f + 1> myMixingTableIndividual;

      unsigned int mySampleIndex{0};
    #ifdef GUI_SUPPORT
      bool myRewindMode{false};
      mutable ByteArray mySamples;
    #endif

    private:
      Audio(const Audio&) = delete;
      Audio(Audio&&) = delete;
      Audio& operator=(const Audio&) = delete;
      Audio& operator=(Audio&&) = delete;
  };
}

#endif // TIA_AUDIO_HXX
