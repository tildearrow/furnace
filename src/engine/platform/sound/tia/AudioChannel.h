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

#ifndef TIA_AUDIO_CHANNEL_HXX
#define TIA_AUDIO_CHANNEL_HXX

namespace TIA {
  class AudioChannel
  {
    public:
      AudioChannel() = default;

      void reset();

      void phase0();

      unsigned char phase1();

      void audc(unsigned char value);

      void audf(unsigned char value);

      void audv(unsigned char value);

    private:
      unsigned char myAudc{0};
      unsigned char myAudv{0};
      unsigned char myAudf{0};

      bool myClockEnable{false};
      bool myNoiseFeedback{false};
      bool myNoiseCounterBit4{false};
      bool myPulseCounterHold{false};

      unsigned char myDivCounter{0};
      unsigned char myPulseCounter{0};
      unsigned char myNoiseCounter{0};

    private:
      AudioChannel(const AudioChannel&) = delete;
      AudioChannel(AudioChannel&&) = delete;
      AudioChannel& operator=(const AudioChannel&) = delete;
      AudioChannel& operator=(AudioChannel&&) = delete;
  };
}

#endif // TIA_AUDIO_CHANNEL_HXX
